#include "threadpool.hpp"

const int MAX_THRESHHOLD_SIZE = 1024;
const int TASK_MAX_THRESHHOLD = INT32_MAX;
const int THREAD_MAX_THRESHHOLD = 10;
const int THREAD_MAX_IDLE_TIME = 60;//空闲线程最多存在的时间
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
:initThreadSize_(4),
taskSize_(0),
taskQueMaxThreshHold_(TASK_MAX_THRESHHOLD),
isRunning_(false),
idleThreadSize_(0),
threadSizeThreshHold_(THREAD_MAX_THRESHHOLD),
curThreadSize_(0)
{
}

// 线程池析构
ThreadPool::~ThreadPool()
{}

// 设置线程池的工作模式
void ThreadPool::setMode(PoolMode mode){
    //如果是线程池已经开始runing了，那么不允许更改
    if(checkRunningState()){
        return ;
    }
    poolMode_ = mode;
}

// 设置task任务队列上线阈值
void ThreadPool::setTaskQueMaxThreshHold(int threshhold){
    //如果是线程池已经开始runing了，那么不允许更改
    if(checkRunningState()){
        return ;
    }
    taskQueMaxThreshHold_ = threshhold;
}

// 设置线程池cached模式下线程阈值
void ThreadPool::setThreadSizeThreshHold(int threshhold){
    //如果是线程池已经开始runing了，那么不允许更改
    if(checkRunningState()){
        return ;
    }
    threadSizeThreshHold_ = threshhold;
}


// 开启线程池
void ThreadPool::start(int initThreadSize){
    initThreadSize_ = initThreadSize;//初始化线程数量
    isRunning_ = true;//线程池开始运行
    curThreadSize_ = initThreadSize; //初始化当前线程数量

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
        //初始时，开始了一个线程，那肯定是一个空闲线程
        idleThreadSize_++;
    }
}

// 定义线程函数，这里充当消费者，从任务队列里取任务执行
void ThreadPool::threadFunc(){
    auto lastTime = std::chrono::high_resolution_clock::now();//记录上一次时间

    for(;;){
        // 抢锁
        std::unique_lock<std::mutex> lock(taskMutex_);

        //提示一下

        std::cout<<"线程: "<<std::this_thread::get_id()<<" 准备从任务队列取任务"<<std::endl;

        //Cached模式下，有可能已经创建了很多线程，但是空闲时间超过了60S，应该把多余的线程结束回收掉
        //回收那些超过initThreadSize_数量的线程
        //超过时间 = 上一次时间 - 当前时间
        if(poolMode_ == PoolMode::MODE_CACHED){
        //在cached模式下，即使是任务队列里面有任务，但可能这个线程就一直没有被notify到，所以就一直空闲这
        while(taskQue_.size() > 0){
            if(notEmpty_.wait_for(lock,std::chrono::seconds(1))==std::cv_status::timeout){//超时了1S还没被notify到
                auto nowTime = std::chrono::high_resolution_clock::now();
                auto dur = std::chrono::duration_cast<std::chrono::seconds>(nowTime-lastTime);//持续了多少时间没被notify到了
                if(dur.count()>=THREAD_MAX_IDLE_TIME&&curThreadSize_>initThreadSize_){//持续时间超过了60秒，且空闲数量大于初始的数量，那么就要把多余的空闲的给回收掉
                    //开始回收当前线程
                    //记录线程数量相关变量的值修改
                    //把线程对象从线程列表容器中删除，在这之前的痛点是不知道这个threadFUnc和Thread对象的对应关系，所以设置一个线程id
                }
            }
        }


        }else{
            // 如果任务队列空了，就要等待，非空才能往下走
            notEmpty_.wait(lock, [&]() -> bool{ return taskQue_.size() > 0; });
        }

        // 来到了这里表示任务队列非空，可以取任务执行了
        std::cout<<"线程: "<<std::this_thread::get_id()<<" 取到了任务"<<std::endl;
        // 都已经取到任务了，那么空闲线程数量应该减一
        idleThreadSize_--;
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
        //在这里还是让用户自己去定义任务，但是我们要把run的返回值MyAny类防盗Result了，
        //所以得重新包装
        if(ptr!=nullptr){
            //ptr->run();
            ptr->exec();
        }
        //执行完线程了，那么空闲线程数量应该加一
        idleThreadSize_++;
        //任务执行完了，更新一下时间
        lastTime = std::chrono::high_resolution_clock::now();
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
        return Result(std::move(task),false);
    }

    //来到了这里表示队列没满，可以往队列添加任务
    taskQue_.emplace(task);
    taskSize_++;
    //添加完任务后，通知那些被notEmpty条件变量卡住的线程，让他们变成阻塞态
    notEmpty_.notify_all();

    //如果是在cached模式下，并且任务的数量大于空闲线程的数量，并且线程数量小于线程数量阈值，那么就创建新的线程
    if(poolMode_ == PoolMode::MODE_CACHED&&taskSize_>idleThreadSize_&&curThreadSize_<threadSizeThreshHold_){
        std::unique_ptr<Thread> ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc,this)); 
        threads_.emplace_back(std::move(ptr));
        curThreadSize_++;
    }

    return Result(std::move(task));
}

//检查线程池的运行状态
bool ThreadPool::checkRunningState() const {
    return isRunning_;
}

/**************************************ThreadPool**************************************************/

/****************************************Thread**************************************************/
 int Thread::generateId_ = 0;
//构造函数
Thread::Thread(std::function<void(void)> threadFun)
:threadFun_(threadFun),
threadId_(generateId_++)
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

//获取线程ID
int Thread::getId() const {
    return threadId_;
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

//把run包装起来，因为我们要在run的基础上得到run的返回值
void Task::exec(){
    result_->setVal(run());
}


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
    sem_ = std::make_unique<MySemaphore>();

    //submitTask返回的Result类型的对象包含submitTask提交的具体的一个task_
    //这个Task类包含Result指针，在这里就是要把task_的result_指针指向当前这个Result对象
    task->result_ = this;
}
// 添加移动构造函数实现
Result::Result(Result&& other) noexcept
    : any_(std::move(other.any_)),
      sem_(std::move(other.sem_)),
      task_(std::move(other.task_)),
      isValid_(other.isValid_.load()) {
}
MyAny Result::get(){
    if(isValid_==false){
        return "";
    }
    sem_->wait();//task任务还没有执执行完，就会阻塞在这里
    return std::move(any_);
}

void Result::setVal(MyAny any){
    any_ = std::move(any);
    //到了这里已经是任务执行完了，就可以把信号量加1了
    sem_->post();
}
/**************************************Result**************************************************/