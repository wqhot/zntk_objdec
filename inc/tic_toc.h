//
// Created by wq on 20-12-6.
//

#ifndef objectdetection_tic_toc.h_H
#define objectdetection_tic_toc.h_H

#include <ctime>
#include <cstdlib>
#include <chrono>
#include <iostream>
#include <string>

class TicToc
{
    public:
    TicToc()
    {
        tic();
    }

    void tic()
    {
        start = std::chrono::system_clock::now();
    }

    double toc()
    {
        end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        return elapsed_seconds.count() * 1000;
    }

    double restart_toc()
    {
        end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        start = std::chrono::system_clock::now();
        return elapsed_seconds.count() * 1000;
    }

    void toc_print(std::string str)
    {
        end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << str << " spend: " << elapsed_seconds.count() * 1000 << " ms" << std::endl;
    }

    private:
    std::chrono::time_point<std::chrono::system_clock> start, end;
};

#endif //objectdetection_tic_toc.h_H
