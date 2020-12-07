//
// Created by hwhiaiuser on 20-12-7.
//

#ifndef zntk_objdec_detect_thread_H
#define zntk_objdec_detect_thread_H


class DetectThread {
    private:
    static int detect_func(DetectThread *handler);

    public:
    int get_queue_id;
    unsigned short serv_port;
    DetectThread(int get_queue_id);
};

#endif

//zntk_objdec_detect_thread_H
