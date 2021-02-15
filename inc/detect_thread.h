//
// Created by hwhiaiuser on 20-12-7.
//

#ifndef zntk_objdec_detect_thread_H
#define zntk_objdec_detect_thread_H

#include <vector>
#include "utils.h"
#include "PracticalSocket.h"
#include "acl/acl.h"
#include "model_process.h"
#include "dvpp_process.h"
#include "post_process.h"
#include "thread_queue.h"

class DetectThread {
    private:
    static int detect_func(DetectThread *handler);
    void ProcessOutput(const aclmdlDataset *output, float xScale, float yScale, std::vector <BBox> &result);
    Result GetImageResizeBuffer(std::vector<u_int8_t> &image_buffer, int resizeWidth, int resizeHeight,
                            void *&buffer, size_t &bufferLen,
                            float &xScale, float &yScale);
    Result result_to_uchars(std::vector<BBox> &detect_result, int id, std::vector<u_char> &send_buffer);
    u_char uint2chars(uint32_t num, std::vector<unsigned char> &cs);

    public:
    int get_queue_id;
    UDPSocket sock;
    unsigned short serv_port;
    std::string client_address;
    unsigned short client_port;
    bool set;
    void setip(std::string client_address);
    DetectThread(int get_queue_id, std::string client_address, std::string client_port);
};

#endif

//zntk_objdec_detect_thread_H
