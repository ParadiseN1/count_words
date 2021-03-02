//
// Created by Admin on 02.02.2021.
//
#include <mutex>
#include <queue>
#include <condition_variable>

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
        queue.push(std::move(elem));
        cv.notify_one(); //! Push then notify!
    }

    bool pop(T& elem){
        std::unique_lock<std::mutex> lk(mutex);
        while (queue.empty()) //! Відповідни метод queue_t має захоплювати м'ютекс, тому тут не можна використати.
            cv.wait(lk);
        elem = std::move(queue.front());
        queue.pop();
        return true;
    }

    size_t size(){
        std::lock_guard<std::mutex> lk(mutex);
        return queue.size();
    }

    bool empty(){
        std::lock_guard<std::mutex> lk(mutex);
        return queue.empty();
    }
};
#endif //UNTITLED4_QUEUE_T_H
