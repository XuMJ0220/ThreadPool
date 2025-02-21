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
    initThreadSize_ = initThreadSize;//初始化线程数量

    for(int i=0;i<initThreadSize_;i++){
        //往线程池里添加线程
        for(int i = 0; i < initThreadSize_ ; i++){
            //尖括号<>里的填原本new的那个类型，()括号里的填传入的参数
            std::unique_ptr<Thread> ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc,this)); 
            //threads_.emplace_back(new Thread(std::bind(&ThreadPool::threadFunc,this)));
            //不能直接把ptr传进去，ptr这里是一个左值，
            //ptr在这里其实是一个unique_ptr，unique_ptr的左值引用拷贝构造函数被删掉了，
            //而emplace_back是会默认进行赋值的，这就导致了传入一个左值的话会报错，所以得用move来把左值改成右值
            //简单来说，就是emplace_back如果参数是一个左值，
            //那么就会进行左值引用拷贝构造，如果是一个右值，就会进行右值引用拷贝构造
            threads_.emplace_back(std::move(ptr));
        }
    }

    //开始执行
    for(int i = 0;i<initThreadSize_;i++){
        threads_[i]->start();
    }
}

// 定义线程函数
void ThreadPool::threadFunc(){
    std::cout<<"begin thread tid: "<<std::this_thread::get_id()<<std::endl;
    std::cout<<"end thread tid: "<<std::this_thread::get_id()<<std::endl;
}
/**************************************ThreadPool**************************************************/

/****************************************Thread**************************************************/
//构造函数
Thread::Thread(std::function<void(void)> threadFun)
:threadFun_(threadFun)
{
}

//析构函数
Thread::~Thread(){}

//开始线程
void Thread::start(){
    // 创建一个线程来执行一个线程函数 pthread_create
    std::thread t(threadFun_);
    t.detach(); // 设置分离线程  
}
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