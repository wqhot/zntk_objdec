//
// Created by hwhiaiuser on 20-12-7.
//

#ifndef zntk_objdec_clock_H
#define zntk_objdec_clock_H

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>

inline double get_current_time()
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz); //该函数在sys/time.h头文件中
    return double(tv.tv_sec) + double(tv.tv_usec) / 1000000.0;
}

#endif //zntk_objdec_clock_H
