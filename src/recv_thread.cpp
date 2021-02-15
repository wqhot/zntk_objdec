//
// Created by hwhiaiuser on 20-12-7.
//
#include <cstdlib>      // For atoi()
#include <string.h>
#include <iostream>

#include "recv_thread.h"
#include "clock.h"
#include "utils.h"
#include "acl/acl.h"

#define BUF_LEN 65540
#define PACK_SIZE 4096

void RecvThread::recv_server(RecvThread *handler)
{
    char buffer[BUF_LEN]; // Buffer for echo string
    int recvMsgSize; // Size of received message
    std::string sourceAddress; // Address of datagram source
    unsigned short sourcePort; // Port of datagram source
    int id;
    while (true)
    {
        // Block until receive message from a client
        do {
            recvMsgSize = handler->sock.recvFrom(buffer, BUF_LEN, sourceAddress, handler->serv_port);
        } while (recvMsgSize > sizeof(int));
        if (!handler->detect_thread->set)
        {
            handler->detect_thread->setip(sourceAddress);
        }
        uint32_t total_length = ((int * ) buffer)[0];
        int total_pack = 1 + (total_length - 1) / PACK_SIZE;
        do {
            recvMsgSize = handler->sock.recvFrom(buffer, BUF_LEN, sourceAddress, handler->serv_port);
        } while (recvMsgSize > sizeof(int));
        id = ((int * ) buffer)[0];
//        cout << "expecting length of packs:" << total_length << endl;
        std::vector<u_int8_t> longbuf;
        longbuf.resize(total_length);
        int recv_length = 0;
        for (int i = 0; i < total_pack; i++) {
            recvMsgSize = handler->sock.recvFrom(buffer, BUF_LEN, sourceAddress, handler->serv_port);
            memcpy(longbuf.data() + recv_length, buffer, recvMsgSize);
            recv_length = recv_length + recvMsgSize;
        }
        if (recv_length != total_length)
        {
            std::cerr << "Received unexpected size pack:" << recv_length << ", except " << total_length << std::endl;
            continue;
        }

        basic_item insert_item(longbuf, id);
        global_queue.insert_queue(handler->queue_id, insert_item);
    }
}

RecvThread::RecvThread(unsigned short serv_port) : serv_port(serv_port), sock(serv_port)
{
    RecvThread::queue_id = global_queue.add_queue();
    std::thread th(recv_server, this);
    th.detach();
}