//
// Created by Admin on 09.02.2021.
//

#ifndef UNTITLED4_TIME_H
#define UNTITLED4_TIME_H

inline std::chrono::steady_clock::time_point get_current_time_fenced() {
    assert(std::chrono::steady_clock::is_steady &&
                   "Timer should be steady (monotonic).");
    std::atomic_thread_fence(std::memory_order_seq_cst);
    auto res_time = std::chrono::steady_clock::now();
    std::atomic_thread_fence(std::memory_order_seq_cst);
    return res_time;
}
template<class D>
inline long long to_s(const D& d)
{
    return std::chrono::duration_cast<std::chrono::microseconds>(d).count() / 1000000;
}

#endif //UNTITLED4_TIME_H
