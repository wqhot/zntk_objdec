//
// Created by hwhiaiuser on 20-12-7.
//

#ifndef zntk_objdec_recv_thread_H
#define zntk_objdec_recv_thread_H

#include <thread>
#include <condition_variable>
#include <mutex>
#include "thread_manage.h"
#include "PracticalSocket.h"

class RecvThread
{
private:
    static void recv_server(RecvThread *recvThread);

public:
    int queue_id;
    unsigned short serv_port;
    UDPSocket sock;
    RecvThread(unsigned short serv_port);

};

#endif //zntk_objdec_recv_thread_H
