//
// Created by Admin on 02.02.2021.
//

#include "queue_t.h"
template<typename T>
void queue_t<T>::push(T &&elem){
    std::lock_guard<std::mutex> lock(mutex);
    queue.push(std::move(elem));
}

template<class T>
bool queue_t<T>::pop(T& elem){
    if (queue.empty())
        return false;
    std::lock_guard<std::mutex> lock(mutex);
    elem = queue.front();
    queue.pop();
    return true;
}

template<class T>
bool queue_t<T>::empty(){
    return queue.empty();
}
