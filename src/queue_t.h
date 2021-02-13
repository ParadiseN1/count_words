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
    bool is_finished = false;
public:
    void push(T &&elem){
        std::lock_guard<std::mutex> lock(mutex);
        cv.notify_one();
        queue.push(std::move(elem));
    }

//    bool pop(T& elem){
//        if (queue.empty())
//            return false;
//        std::lock_guard<std::mutex> lock(mutex);
//        elem = queue.front();
//        queue.pop();
//        return true;
//    }

    bool pop(T& elem){
        std::unique_lock<std::mutex> lk(mutex);
        while (empty() && !is_finished)
            cv.wait(lk);
        elem = queue.front();
        queue.pop();
        return true;
    }

    void set_finished() {
        std::unique_lock<std::mutex> locker(mutex);
        is_finished = true;
        cv.notify_all();
    }

    bool empty(){
        return queue.empty();
    }
};
#endif //UNTITLED4_QUEUE_T_H
