//
// Created by hwhiaiuser on 20-12-7.
//

#ifndef zntk_objdec_detect_thread_H
#define zntk_objdec_detect_thread_H

#include <vector>
#include "utils.h"
#include "PracticalSocket.h"

class DetectThread {
    private:
    static int detect_func(DetectThread *handler);
    Result result_to_uchars(const std::vector<BBox> &detect_result, std::vector<u_char> &send_buffer);
    u_char uint2chars(uint32_t num, std::vector<unsigned char> &cs);

    public:
    int get_queue_id;
    UDPSocket sock;
    unsigned short serv_port;
    std::string client_address;
    unsigned short client_port;
    DetectThread(int get_queue_id, std::string client_address, std::string client_port);
};

#endif

//zntk_objdec_detect_thread_H
