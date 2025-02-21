#include "threadpool.hpp"

const int MAX_THRESHHOLD_SIZE = 1024;
const int TASK_MAX_THRESHHOLD = INT32_MAX;
/**************************************ThreadPool**************************************************/
// 线程池构造
ThreadPool::ThreadPool()
:initThreadSize_(4),threadSizeThreshHold_(MAX_THRESHHOLD_SIZE),taskSize_(0),taskQueMaxThreshHold_(TASK_MAX_THRESHHOLD)
{
}

// 线程池析构
ThreadPool::~ThreadPool()
{}

// 设置线程池的工作模式
void ThreadPool::setMode(PoolMode mode){
    poolMode_ = mode;
}

// 设置task任务队列上线阈值
void ThreadPool::setTaskQueMaxThreshHold(int threshhold){
    taskQueMaxThreshHold_ = threshhold;
}

// 设置线程池cached模式下线程阈值
void ThreadPool::setThreadSizeThreshHold(int threshhold){
    threadSizeThreshHold_ = threshhold;
}

// 开启线程池
void ThreadPool::start(int initThreadSize){
    initThreadSize_ = initThreadSize;
}

// 定义线程函数
void ThreadPool::threadFunc(){

}
/**************************************ThreadPool**************************************************/

/****************************************Thread**************************************************/
//构造函数
Thread::Thread()
{
}

//析构函数
Thread::~Thread(){}

/****************************************Thread**************************************************/

/*****************************************Task**************************************************/
//构造函数
Task::Task()
{
}

//析构函数
Task::~Task()
{}


/*****************************************Task**************************************************/