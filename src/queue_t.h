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
    std::condition_variable cv;
public:
    void push(T &&elem){
        std::lock_guard<std::mutex> lock(mutex);
        cv.notify_one();
        queue.push(std::move(elem));
    }

    bool pop(T& elem){
        std::unique_lock<std::mutex> lk(mutex);
        while (empty())
            cv.wait(lk);
        elem = queue.front();
        queue.pop();
        return true;
    }

    u_int size(){
        return queue.size();
    }

    bool empty(){
        return queue.empty();
    }
};
#endif //UNTITLED4_QUEUE_T_H
