//
// Created by hwhiaiuser on 20-12-7.
//

#ifndef zntk_objdec_thread_manage_H
#define zntk_objdec_thread_manage_H
#include <opencv2/core.hpp>
#include <queue>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include "utils.h"

typedef std::pair<ImageData, int> basic_item;

struct mutex_struct{
    std::mutex mtx;
};
class threadManage
{
    public:

    typedef std::queue<basic_item> basic_queue;
    int add_queue();
    int insert_queue(int id, basic_item &insert_item);
    bool get_queue(int id, basic_item &get_item);
    bool get_queue_without_pop(int id, basic_item &get_item);
    bool insert_queue_with_pop(int id, basic_item &insert_item);
    int get_queue_size(int id);
    std::mutex g_mutex;
    std::condition_variable g_cond;
    inline bool get_try_to_expire(int id)
    {
        return m_try_to_expire[id];
    }
    inline void try_to_expire()
    {
        for(int i = 0; i < m_try_to_expire.size(); i++)
        {
            m_try_to_expire[i] = true;
        }
        for(int i = 0; i < m_try_to_expire.size(); i++)
        {
            m_conds[i].notify_all();
        }
        should_exit = true;
    }
    bool should_exit;
    threadManage();

    private:
    std::vector<basic_queue> m_queues;
    std::mutex m_mutexs[10];
    std::condition_variable m_conds[10];
    std::vector<bool> m_try_to_expire;
};

extern threadManage g_thread_manager;
#endif //zntk_objdec_thread_manage_H
