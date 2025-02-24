// #include <iostream>
// #include "threadpool.hpp"
// #include <thread>
// #include <chrono>
// class MyTask:public Task{
//     private:
//     public:
//         void run(){
//             std::cout<<"线程: "<<std::this_thread::get_id()<<" 开始执行任务"<<std::endl;
//             std::this_thread::sleep_for(std::chrono::seconds(3));
//             std::cout<<"线程: "<<std::this_thread::get_id()<<" 执行任务结束"<<std::endl;
//         }
// };

// int main(int argc,char** argv){
//     ThreadPool pool;
//     pool.start(4);
//     //std::this_thread::sleep_for(std::chrono::seconds(3));

//     pool.submitTask(std::make_shared<MyTask>());
//     pool.submitTask(std::make_shared<MyTask>());
//     pool.submitTask(std::make_shared<MyTask>());

//     for(;;){

//     }
// }