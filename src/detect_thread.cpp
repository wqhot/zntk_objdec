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
    while (true)
    {
        basic_item item;
        if (!g_thread_manager.get_queue(handler->get_queue_id, item))
        {
            continue;
        }
        // TicToc tic;
        ImageData image = item.first;
        if (image.data == nullptr) {
            ERROR_LOG("Read image failed");
            continue;
        }
//        for (int i = 0; i < 100; i++)
//        {
//            std::cout << std::hex << (int)*(image.data.get() + i) << " ";
//        }
//        std::cout << "\n image length = " << image.size << std::endl;

        //Preprocess: copy image to device, convert to yuv, and resize
        ImageData resizedImage;
        Result ret = detect.Preprocess(resizedImage, image);
        if (ret != SUCCESS) {
            ERROR_LOG("Read file failed, continue to read next");
            continue;
        }
//        tic.toc_print("Load JPEG");
        //Send the resized picture to the model for inference
        //and get the inference results
//        tic.restart_toc();
        aclmdlDataset* inferenceOutput = nullptr;
        ret = detect.Inference(inferenceOutput, resizedImage);
        if ((ret != SUCCESS) || (inferenceOutput == nullptr)) {
            ERROR_LOG("Inference model inference output data failed");
            continue;
        }
//        tic.toc_print("Inference model");
//        tic.restart_toc();
        //Analyze the inference output, mark the object category and
        //location by the inference result
        ret = detect.Postprocess(image, inferenceOutput, "output.jpg");
        if (ret != SUCCESS) {
            ERROR_LOG("Process model inference output data failed");
            continue;
        }
//        tic.toc_print("Process model");
    }
}

DetectThread::DetectThread(int get_queue_id): get_queue_id(get_queue_id)
{
    std::thread th(detect_func, this);
    th.detach();
}
