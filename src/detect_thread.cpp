//
// Created by hwhiaiuser on 20-12-7.
//
#include <iostream>
#include <stdlib.h>
#include <dirent.h>
#include <iomanip>
#include <fstream>
#include <map>
#include <cstdint>
#include <string>
#include <memory>
#include "detect_thread.h"
#include <tictoc.h>

namespace
{
    uint32_t kModelWidth = 416;
    uint32_t kModelHeight = 416;
    const char *kModelPath = "/opt/zntk/model/yolov4.om";
} // namespace

#define CHECK_FUNCTION_RESULT(name, result) \
    {                                       \
        if (result != SUCCESS)              \
        {                                   \
            ERROR_LOG(#name " failed");     \
            return -1;                      \
        }                                   \
    }


#define CHECK_FUNCTION_RESULT_NOTQUIT(name, result) \
    {                                       \
        if (result != SUCCESS)              \
        {                                   \
            ERROR_LOG(#name " failed");     \
            continue;                      \
        }                                   \
    }

void BufferDeleter_Thread(void *p) {
    if (!RunStatus::GetDeviceStatus()) {
        if (p != nullptr) {
            (void) aclrtFreeHost(p);
        }
    }
}


/**
* @brief post get memory buffer after image decode and resize
* @param [in] imageFile: image file path
* @param [in] resizeWidth: resize width
* @param [in] resizeHeight: resize height
* @param [out] buffer: buffer
* @param [out] bufferLen: length of buffer
* @param [out] xScale: x scale of the origin image and resized image
* @param [out] yScale: y scale of the origin image and resized image
* @return result failed or success
*/
Result DetectThread::GetImageResizeBuffer(std::vector<u_int8_t> &image_buffer, int resizeWidth, int resizeHeight,
                                          void *&buffer, size_t &bufferLen,
                                          float &xScale, float &yScale)
{
    aclrtStream stream = nullptr;
    aclError ret = aclrtCreateStream(&stream);
    if (ret != ACL_ERROR_NONE)
    {
        ERROR_LOG("create stream failed, error code is %d", static_cast<int32_t>(ret));
        return FAILED;
    }

    uint32_t devBufferSize = 0;
    void *devBuffer = nullptr;
    PicDesc testPic = {"", 0, 0, 0};
    if (SUCCESS != Utils::GetDeviceBufferOfPictureMemery(image_buffer, testPic, devBuffer, devBufferSize))
    {
        ERROR_LOG("get image device buffer failed.");
        (void)aclrtDestroyStream(stream);
        return FAILED;
    }
    INFO_LOG("get device buffer for picture success.");

    xScale = static_cast<float>(testPic.width) / static_cast<float>(resizeWidth);
    yScale = static_cast<float>(testPic.height) / static_cast<float>(resizeHeight);

    DvppProcess dvppProcess(stream);
    if (SUCCESS != dvppProcess.InitResource())
    {
        ERROR_LOG("dvpp process init failed.");
        (void)aclrtDestroyStream(stream);
        return FAILED;
    }

    dvppProcess.SetInput(devBuffer, devBufferSize, testPic);
    if (SUCCESS != dvppProcess.InitDvppOutputPara(resizeWidth, resizeHeight))
    {
        ERROR_LOG("dvpp init output parameters failed.");
        (void)acldvppFree(devBuffer);
        (void)aclrtDestroyStream(stream);
        return FAILED;
    }

    if (SUCCESS != dvppProcess.Process())
    {
        ERROR_LOG("dvpp process failed.");
        (void)acldvppFree(devBuffer);
        (void)aclrtDestroyStream(stream);
        return FAILED;
    }

    INFO_LOG("dvpp decode and resize success.");

    int len = 0;
    dvppProcess.GetDvppOutput(&buffer, len);
    bufferLen = static_cast<size_t>(len);

    (void)acldvppFree(devBuffer);
    devBuffer = nullptr;
    (void)aclrtDestroyStream(stream);

    return SUCCESS;
}

/**
* @brief post processing for output
* @param [in] output: output data for model inference
* @param [in] originImage: origin image file path
* @param [in] xScale: x scale of the origin image and the model input image
* @param [in] yScale: y scale of the origin image and the model input image
*/
void DetectThread::ProcessOutput(const aclmdlDataset *output, float xScale, float yScale, std::vector<BBox> &result)
{
    std::shared_ptr<void> dataBuff[2] = {nullptr, nullptr};
    size_t index = 0;
    for (size_t i = 0; (i < aclmdlGetDatasetNumBuffers(output)) && (i < 2); ++i)
    {
        //get model output data
        aclDataBuffer *dataBuffer = aclmdlGetDatasetBuffer(output, i);
        void *data = aclGetDataBufferAddr(dataBuffer);
        size_t len = aclGetDataBufferSize(dataBuffer);

        if (RunStatus::GetDeviceStatus())
        {
            std::shared_ptr<void> ptr(data, BufferDeleter_Thread);
            dataBuff[i] = ptr;
        }
        else
        {
            void *outHostData = nullptr;
            aclError ret = aclrtMallocHost(&outHostData, len);
            if (ret != ACL_ERROR_NONE)
            {
                std::cout << "aclrtMallocHost failed, result code is " << ret << std::endl;
                break;
            }
            ret = aclrtMemcpy(outHostData, len, data, len, ACL_MEMCPY_DEVICE_TO_HOST);
            if (ret != ACL_ERROR_NONE)
            {
                std::cout << "aclrtMemcpy failed, result code is " << ret << std::endl;
                (void)aclrtFree(outHostData);
                break;
            }
            // fwrite(outHostData, len, sizeof(char), outputFile);
            std::shared_ptr<void> ptr(outHostData, BufferDeleter_Thread);
            dataBuff[i] = ptr;
        }
    }

    PostProcess postProcess(dataBuff[0].get(), dataBuff[1].get(), xScale, yScale);
    Result rlt = postProcess.Process(result);
    if (rlt != SUCCESS)
    {
        ERROR_LOG("post process failed.");
    }
    INFO_LOG("post process success.");
}

int DetectThread::detect_func(DetectThread *handler)
{
    // model process init
    ModelProcess modelProcess;
    Result rlt = modelProcess.Init(0);
    CHECK_FUNCTION_RESULT(modelProcess.Init, rlt);

    rlt = modelProcess.LoadModel(kModelPath);
    CHECK_FUNCTION_RESULT(modelProcess.LoadModel, rlt);

    int width = 0;
    int height = 0;
    rlt = modelProcess.GetModelInputWH(width, height);
    CHECK_FUNCTION_RESULT(modelProcess.GetModelInputWH, rlt);
    while (true)
    {
        basic_item item;
        if (!global_queue.get_queue(handler->get_queue_id, item))
        {
            continue;
        }
        TicToc tic;
        std::vector<BBox> detect_result;
        void *inputBuffer = nullptr;
        int id = item.second;
        size_t inputLen = 0;
        float xScale = 1;
        float yScale = 1;
        rlt = handler->GetImageResizeBuffer(item.first, width, height,
                                            inputBuffer, inputLen, xScale, yScale);
        CHECK_FUNCTION_RESULT_NOTQUIT(handler->GetImageResizeBuffer, rlt);

        // Input memory for creating models
        rlt = modelProcess.CreateInput(inputBuffer, inputLen);
        CHECK_FUNCTION_RESULT_NOTQUIT(modelProcess.LoadModel, rlt);

        // Create the output memory of the model
        rlt = modelProcess.CreateOutput();
        CHECK_FUNCTION_RESULT_NOTQUIT(modelProcess.CreateOutput, rlt);

        // The input and output memory is transferred to the model instance for synchronous execution
        rlt = modelProcess.Execute();
        CHECK_FUNCTION_RESULT_NOTQUIT(modelProcess.Execute, rlt);

        // After execution, the post-processing is carried out for the output memory
        const aclmdlDataset *output = modelProcess.GetModelOutputData();
        handler->ProcessOutput(output, xScale, yScale, detect_result);

        std::vector<u_char> send_buffer;
        handler->result_to_uchars(detect_result, id, send_buffer);
        handler->sock.sendTo(&send_buffer[0], send_buffer.size(),
                             handler->client_address, handler->client_port);
        std::cout << "send result " << send_buffer.size() << std::endl;
        tic.toc_print("detect");
    }
}

u_char DetectThread::uint2chars(uint32_t num, std::vector<unsigned char> &cs)
{
    u_char check = 0;
    for (int i = 0; i < 4; i++)
    {
        uint32_t num_c = (num & (0xFF << (i * 8))) >> (i * 8);
        cs.push_back(num_c);
        check += num_c;
    }
    return check;
}

static void print_BBox(BBox &bbox)
{
    // std::cout << "[" << bbox.rect.ltX
    // << ", "<< bbox.rect.ltY
    // << "  "<< bbox.rect.rbX
    // << ", "<< bbox.rect.rbY
    // << "]: " << bbox.label_id
    // << ": " << bbox.score <<std::endl;
}

Result DetectThread::result_to_uchars(std::vector<BBox> &detect_result, int id, std::vector<u_char> &send_buffer)
{
    // head: 0x7f 0x7f 0x7f 0x7f
    // num
    // ltx lty rbx rby score label
    // checknum

    send_buffer = std::vector<u_char>{0x7f, 0x7f, 0x7f, 0x7f};
    // num
    u_char check;
    //        std::cout << "detecet result: " << detect_result.size() << std::endl;
    check += uint2chars(id, send_buffer);
    check += uint2chars(detect_result.size(), send_buffer);
    //        std::cout << (int)send_buffer[4] << " " << (int)send_buffer[5] << " " << (int)send_buffer[6] << " " << (int)send_buffer[7] << " " << std::endl;
    for (int i = 0; i != detect_result.size(); ++i)
    {
        check += uint2chars(detect_result[i].x, send_buffer);
        check += uint2chars(detect_result[i].y, send_buffer);
        check += uint2chars(detect_result[i].w, send_buffer);
        check += uint2chars(detect_result[i].h, send_buffer);
        check += uint2chars(detect_result[i].score, send_buffer);
        check += uint2chars(detect_result[i].classIndex, send_buffer);
        //            print_BBox(detect_result[i]);
    }
    send_buffer.push_back(check);

    return SUCCESS;
}

DetectThread::DetectThread(int get_queue_id, std::string client_address, std::string client_port) : get_queue_id(get_queue_id), client_address(client_address)
{
    DetectThread::client_port = Socket::resolveService(client_port, "udp");
    set = false;
    std::thread th(detect_func, this);
    th.detach();
}

//DetectThread::DetectThread(): runing(false)
//{
//
//}
//
//
//void DetectThread::init(int get_queue_id, std::string client_port)
//{
//    DetectThread::client_port = Socket::resolveService(client_port, "udp");
//    DetectThread::get_queue_id = get_queue_id;
//}
//
void DetectThread::setip(std::string client_address)
{
    DetectThread::client_address = client_address;
    set = true;
}