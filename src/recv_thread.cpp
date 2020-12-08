//
// Created by hwhiaiuser on 20-12-7.
//
#include <cstdlib>      // For atoi()
#include <string.h>
#include <iostream>

#include "recv_thread.h"
#include "clock.h"
#include "utils.h"

#define BUF_LEN 65540
#define PACK_SIZE 4096

void RecvThread::recv_server(RecvThread *handler)
{
    char buffer[BUF_LEN]; // Buffer for echo string
    int recvMsgSize; // Size of received message
    string sourceAddress; // Address of datagram source
    unsigned short sourcePort; // Port of datagram source
    while (true)
    {
        // Block until receive message from a client
        do {
            recvMsgSize = handler->sock.recvFrom(buffer, BUF_LEN, sourceAddress, handler->serv_port);
        } while (recvMsgSize > sizeof(int));
        uint32_t total_length = ((int * ) buffer)[0];
        int total_pack = 1 + (total_length - 1) / PACK_SIZE;
//        cout << "expecting length of packs:" << total_length << endl;
        u_int8_t * longbuf = new(std::nothrow) u_int8_t[total_length];
        int recv_length = 0;
        for (int i = 0; i < total_pack; i++) {
            recvMsgSize = handler->sock.recvFrom(buffer, BUF_LEN, sourceAddress, handler->serv_port);
            memcpy( & longbuf[recv_length], buffer, recvMsgSize);
            recv_length = recv_length + recvMsgSize;
        }
        if (recv_length != total_length)
        {
            std::cerr << "Received unexpected size pack:" << recv_length << ", except " << total_length << std::endl;
            continue;
        }
//        cout << "Received packet from " << sourceAddress << ":" << handler->serv_port << endl;
        ImageData image;
        Utils::get_image_from_buffer(image, longbuf, total_length);
        basic_item insert_item(image, get_current_time());
        g_thread_manager.insert_queue(handler->queue_id, insert_item);
    }
}

RecvThread::RecvThread(unsigned short serv_port) : serv_port(serv_port), sock(serv_port)
{
    RecvThread::queue_id = g_thread_manager.add_queue();
    std::thread th(recv_server, this);
    th.detach();
}