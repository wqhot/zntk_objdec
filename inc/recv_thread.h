//
// Created by hwhiaiuser on 20-12-7.
//

#ifndef zntk_objdec_recv_thread_H
#define zntk_objdec_recv_thread_H

#include <thread>
#include <condition_variable>
#include <mutex>
#include "thread_queue.h"
#include "PracticalSocket.h"
#include "detect_thread.h"

class RecvThread
{
private:
    static void recv_server(RecvThread *recvThread);

public:
    int queue_id;
    unsigned short serv_port;
    UDPSocket sock;
    DetectThread *detect_thread;
    RecvThread(unsigned short serv_port);

};

#endif //zntk_objdec_recv_thread_H
