#include <iostream>
#include "threadpool.hpp"
#include <thread>
#include <chrono>
int main(){
    ThreadPool pool;
    pool.start(4);

    std::this_thread::sleep_for(std::chrono::seconds(5));
    return 0;
}