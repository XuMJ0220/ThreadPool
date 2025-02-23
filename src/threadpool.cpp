#include "threadpool.hpp"

const int MAX_THRESHHOLD_SIZE = 1024;
const int TASK_MAX_THRESHHOLD = INT32_MAX;
/**************************************ThreadPool**************************************************/

//使用例子:
//class MyTask:public Task
//{
//  private:
//  public:
//         void run(){
//             自定义
//             ....
//         }
//      
//}

//ThreadPool pool;
//pool.start(4);

//pool.submitTask(std::share_ptr<MyTask>());



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

// 定义线程函数，这里充当消费者，从任务队列里取任务执行
void ThreadPool::threadFunc(){
    for(;;){
        // 抢锁
        std::unique_lock<std::mutex> lock(taskMutex_);

        //提示一下

        std::cout<<"线程: "<<std::this_thread::get_id()<<" 准备从任务队列取任务"<<std::endl;

        // 如果任务队列空了，就要等待，非空才能往下走
        notEmpty_.wait(lock, [&]() -> bool
                       { return taskQue_.size() > 0; });
        // 来到了这里表示任务队列非空，可以取任务执行了
        std::cout<<"线程: "<<std::this_thread::get_id()<<" 取到了任务"<<std::endl;

        std::shared_ptr<Task> ptr = taskQue_.front();
        taskQue_.pop();
        taskSize_--;
        //在这里就要把锁让出去，不能等到我把任务做完了才把锁让出去，且notFull去通知那些被notFull卡住的线程，让他们变成阻塞态
        lock.unlock();
        notFull_.notify_all();
        //如果依然有剩余任务，继续通知其它线程执行任务
        if(taskQue_.size()>0){
            notEmpty_.notify_all();
        }
        //自己去执行任务了
        
        ptr->run();
    }
    
}

//用户提交任务,这里就是一个生产者了
Result ThreadPool::submitTask(std::shared_ptr<Task> task){
    //先获得互斥锁
    std::unique_lock<std::mutex> lock(taskMutex_);

    //有时间限制的版本,在规定的时间内满足了就往下走，不然就退出
    if(notFull_.wait_for(lock,std::chrono::seconds(1),[&]()->bool{
        return taskQue_.size() < (size_t)taskQueMaxThreshHold_;
    })==false){
        std::cerr<<"task queue is full,submit task failed!"<<std::endl;
        return Result(task,false);
    }

    //来到了这里表示队列没满，可以往队列添加任务
    taskQue_.emplace(task);
    taskSize_++;
    //添加完任务后，通知那些被notEmpty条件变量卡住的线程，让他们变成阻塞态
    notEmpty_.notify_all();
    return Result(task);
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

/*****************************************MyAny**************************************************/




/*****************************************MyAny**************************************************/
/**************************************MySemaphore**************************************************/
void MySemaphore::wait(){
    //先获得锁
    std::unique_lock<std::mutex> lock(mtx_);
    //等待信号量有资源，没有资源的话，会阻塞当前线程
    cond_.wait(lock,[&]()->bool{
        return resLimit_ > 0;
    });
    resLimit_--;
}

void MySemaphore::post(){
    //先获得锁
    std::unique_lock<std::mutex> lock(mtx_);
    resLimit_++;
    cond_.notify_all();
}

/**************************************MySemaphore**************************************************/

/**************************************Result**************************************************/
Result::Result(std::shared_ptr<Task> task,bool isValid)
:task_(task),isValid_(isValid)
{
}
/**************************************Result**************************************************/