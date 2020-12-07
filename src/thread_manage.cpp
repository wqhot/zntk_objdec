//
// Created by hwhiaiuser on 20-12-7.
//

#include <thread_manage.h>
#include <iostream>

threadManage::threadManage()
{
    m_queues.clear();
    m_try_to_expire.clear();
    should_exit = false;
}

int threadManage::add_queue()
{
    int queue_id = m_queues.size();
    m_queues.push_back(basic_queue());
    m_try_to_expire.push_back(false);
    return queue_id;
}

int threadManage::insert_queue(int id, basic_item &insert_item)
{
    std::unique_lock<std::mutex> lock(m_mutexs[id]);
    while (m_queues[id].size() > 10)
    {
        m_queues[id].pop();
        std::cout << "img[" << id << "] drop!" << std::endl;
    }
    m_queues[id].push(insert_item);
    m_conds[id].notify_all();
    lock.unlock();
    return 0;
}

bool threadManage::insert_queue_with_pop(int id, basic_item &insert_item)
{
    std::unique_lock<std::mutex> lock(m_mutexs[id]);
    while (m_queues[id].size() > 0)
    {
        m_queues[id].pop();
    }
    m_queues[id].push(insert_item);
    m_conds[id].notify_all();
    lock.unlock();
    return 0;
}

bool threadManage::get_queue(int id, basic_item &get_item)
{
    // std::cout << "lock " << id << " try get" << std::endl;
    std::unique_lock<std::mutex> lock(m_mutexs[id]);
    // std::cout << "lock " << id << "get" << std::endl;
    if (m_queues[id].empty())
    {
        m_conds[id].wait(lock);
    }
    bool ret = !m_queues[id].empty();
    if (!m_queues[id].empty())
    {
        get_item = m_queues[id].front();
        m_queues[id].pop();
    }
    lock.unlock();
    // std::cout << "lock " << id << " release" << std::endl;
    return ret;
}

bool threadManage::get_queue_without_pop(int id, basic_item &get_item)
{
    std::unique_lock<std::mutex> lock(m_mutexs[id]);
    // std::cout << "lock " << id << "get" << std::endl;
    if (m_queues[id].empty())
    {
        m_conds[id].wait(lock);
    }
    bool ret = !m_queues[id].empty();
    if (!m_queues[id].empty())
    {
        get_item = m_queues[id].front();
    }
    lock.unlock();
    // std::cout << "lock " << id << " release" << std::endl;
    return ret;
}

int threadManage::get_queue_size(int id)
{
    std::unique_lock<std::mutex> lock(m_mutexs[id]);
    int length = 0;
    if (m_queues[id].empty())
    {
        m_conds[id].wait(lock);
    }
    if (!m_queues[id].empty())
    {
        length = m_queues[id].size();
    }
    lock.unlock();
    return length;
}