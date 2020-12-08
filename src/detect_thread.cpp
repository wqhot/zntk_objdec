//
// Created by hwhiaiuser on 20-12-7.
//
#include <iostream>
#include <stdlib.h>
#include <dirent.h>
#include <iomanip>

#include "object_detect.h"
#include "utils.h"
#include "tic_toc.h"
#include "detect_thread.h"
#include "thread_manage.h"

namespace {
    uint32_t kModelWidth = 416;
    uint32_t kModelHeight = 416;
    const char* kModelPath = "../model/yolov3.om";
}

int DetectThread::detect_func(DetectThread *handler)
{
    ObjectDetect detect(kModelPath, kModelWidth, kModelHeight);
    Result ret = detect.Init();
    if (ret != SUCCESS) {
        ERROR_LOG("Classification Init resource failed");
        return FAILED;
    }
    std::vector<BBox> detect_result;
    while (true)
    {
        basic_item item;
        if (!g_thread_manager.get_queue(handler->get_queue_id, item))
        {
            continue;
        }
        ImageData image = item.first;
        if (image.data == nullptr) {
            ERROR_LOG("Read image failed");
            continue;
        }
        //Preprocess: copy image to device, convert to yuv, and resize
        ImageData resizedImage;
        Result ret = detect.Preprocess(resizedImage, image);
        if (ret != SUCCESS) {
            ERROR_LOG("Read file failed, continue to read next");
            continue;
        }
        //Send the resized picture to the model for inference
        //and get the inference results
        aclmdlDataset* inferenceOutput = nullptr;
        ret = detect.Inference(inferenceOutput, resizedImage);
        if ((ret != SUCCESS) || (inferenceOutput == nullptr)) {
            ERROR_LOG("Inference model inference output data failed");
            continue;
        }
        //Analyze the inference output, mark the object category and
        //location by the inference result
        ret = detect.Postprocess(image, inferenceOutput, detect_result);
        if (ret != SUCCESS) {
            ERROR_LOG("Process model inference output data failed");
            continue;
        }

        std::vector<u_char> send_buffer;
        handler->result_to_uchars(detect_result, send_buffer);
        handler->sock.sendTo(&send_buffer[0], send_buffer.size(),
                             handler->client_address, handler->client_port);
//        std::cout << "send result " << send_buffer.size() << std::endl;

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
        std::cout << "[" << bbox.rect.ltX
        << ", "<< bbox.rect.ltY
        << "  "<< bbox.rect.rbX
        << ", "<< bbox.rect.rbY
        << "]: " << bbox.label_id
        << ": " << bbox.score <<std::endl;
}

Result DetectThread::result_to_uchars(std::vector<BBox> &detect_result, std::vector<u_char> &send_buffer)
{
    // head: 0x7f 0x7f 0x7f 0x7f
    // num
    // ltx lty rbx rby score label
    // checknum

        send_buffer = std::vector<u_char> {0x7f, 0x7f, 0x7f, 0x7f};
        // num
        u_char check;
//        std::cout << "detecet result: " << detect_result.size() << std::endl;
        check += uint2chars(detect_result.size(), send_buffer);
//        std::cout << (int)send_buffer[4] << " " << (int)send_buffer[5] << " " << (int)send_buffer[6] << " " << (int)send_buffer[7] << " " << std::endl;
        for (int i = 0; i != detect_result.size(); ++i)
        {
            check += uint2chars(detect_result[i].rect.ltX, send_buffer);
            check += uint2chars(detect_result[i].rect.ltY, send_buffer);
            check += uint2chars(detect_result[i].rect.rbX, send_buffer);
            check += uint2chars(detect_result[i].rect.rbY, send_buffer);
            check += uint2chars(detect_result[i].score, send_buffer);
            check += uint2chars(detect_result[i].label_id, send_buffer);
//            print_BBox(detect_result[i]);
        }
        send_buffer.push_back(check);

    return SUCCESS;
}

DetectThread::DetectThread(int get_queue_id, std::string client_address, std::string client_port): get_queue_id(get_queue_id), client_address(client_address)
{
    DetectThread::client_port = Socket::resolveService(client_port, "udp");
    std::thread th(detect_func, this);
    th.detach();
}
