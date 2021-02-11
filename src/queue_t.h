//
// Created by Admin on 02.02.2021.
//
#include <mutex>
#include <queue>

#ifndef UNTITLED4_QUEUE_T_H
#define UNTITLED4_QUEUE_T_H
template<typename T>
class queue_t{
private:
    std::queue<T> queue;
    std::mutex mutex;
public:
    void push(T &&elem){
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(std::move(elem));
    }

    bool pop(T& elem){
        if (queue.empty())
            return false;
        std::lock_guard<std::mutex> lock(mutex);
        elem = queue.front();
        queue.pop();
        return true;
    }

    bool empty(){
        return queue.empty();
    }
};
#endif //UNTITLED4_QUEUE_T_H
