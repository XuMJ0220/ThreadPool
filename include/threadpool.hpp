#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <vector>
#include <queue>
#include <atomic>
#include <memory>
#include <thread>
#include <functional>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
class Thread{
    private:
        std::function<void(int)> threadFun_;

		static int generateId_;//静态线程ID，可以保证不管创建了多少个Thread，这个generateId都是唯一的
		int threadId_;//保存线程id
    public:
        //构造函数
        Thread(std::function<void(int)>);

        //析构函数
        ~Thread();

        //开始线程
        void start();

		//获取线程ID
		int getId() const;
};
//实现自己的Any类
class MyAny
{
public:
	// 1.要接收任意类型，所以得用模板 5.初始化部分,base_指针赋值为Derive(data)的指针
	template <typename T>
	MyAny(T data)
	:base_(std::make_unique<Derive<T>>(data))
	{
	}

	// 2.一个Base基类
	class Base
	{
	private:
	public:
		virtual ~Base() = default;
	};
	// 3.派生类,把data_放在派生类中
	template <typename T>
	class Derive : public Base
	{
	public:
		T data_;

	public:
		Derive(T data) : data_(data) {};
	};

	//6.
	MyAny() = default;
	~MyAny() = default;
	MyAny(const MyAny&) = delete;
	MyAny& operator=(const MyAny&) = delete;
	MyAny(MyAny&&) = default;
	MyAny& operator=(MyAny&&) = default;

	//7.把MyAny类型里面存储的data数据提取出来
	template<typename T>
	T cast_();
private:
	// 4.一个基类的指针指针
	std::unique_ptr<Base> base_;
};

//7.
template<typename T>
T MyAny::cast_(){
    //在这里我们明确的知道了base_就是一个基类的智能指针，但是指向Derive<T>这个派生类
    //那么就直接接收了这个指针
    //这里dynamic_cast<T>(base_.get())是因为base_.get()就是一个指向Derive<T>派生类的指针
    //智能指针的get()可以得到普通指针
    Derive<T>* pt = dynamic_cast<Derive<T>*>(base_.get());
    if(pt==nullptr){
        throw "type is unmatch!";
    }
    return pt->data_;
}

//MySemaphore
class MySemaphore{
	private:
		//条件变量
		std::mutex mtx_;
		std::condition_variable cond_;
		//资源
		int resLimit_;
	public:
		MySemaphore(int resLimit = 0)
		:resLimit_(resLimit)
		{}
		MySemaphore(const MySemaphore&) = delete;
		MySemaphore& operator=(const MySemaphore&) = delete;
		~MySemaphore() = default;
		//P
		void wait();
		//V
		void post();
};
//Task类的前置声明
class Task;
//Result类
class Result{
	private:
		MyAny any_;//存储任务的返回值
		//MySemaphore sem_;//线程通信信号量
		std::unique_ptr<MySemaphore> sem_; // 改为unique_ptr
		std::shared_ptr<Task> task_;//指向对应获取返回值的任务对象
		std::atomic_bool isValid_;//返回值是否有效
	public:
		Result(std::shared_ptr<Task> task,bool isValid=true);
		Result(Result&& other) noexcept; // 移动构造函数
		~Result() = default;
		Result(const Result&) = delete;
    	Result& operator=(const Result&) = delete;

		//问题一：如何把Task中任务执行完，也就是run()执行完的MyAny返回值存到Result的any_呢？
		void setVal(MyAny any);
		//问题二：用户在调用返回的Result的get方法，用户调用这个方法获取run()执行后返回的MyAny，
		//同时，如果submitTask提交的任务没有被执行完，get是要被阻塞住的
		MyAny get();
};

// 任务抽象基类
class Task
{
    public:
        //构造函数
        Task();
        
        //析构函数
        ~Task();

		//执行任务
		//用户可以自定义任意任务类型，从Task继承，重写run方法，实现自定义任务处理
		//run返回类型改为MyAny
		virtual MyAny run() = 0;

		//把run包装起来，因为我们要在run的基础上得到run的返回值
		void exec();

		//指向Result类的指针
		Result* result_;
	private:
		
};

// 线程池支持的模式
enum class PoolMode
{
	MODE_FIXED,  // 固定数量的线程
	MODE_CACHED, // 线程数量可动态增长
};

class ThreadPool
{
public:
	// 线程池构造
	ThreadPool();

	// 线程池析构
	~ThreadPool();

	// 设置线程池的工作模式
	void setMode(PoolMode mode);

	// 设置task任务队列上线阈值
	void setTaskQueMaxThreshHold(int threshhold);

	// 设置线程池cached模式下线程阈值
	void setThreadSizeThreshHold(int threshhold);

	// 开启线程池
	void start(int initThreadSize = 4);

	//用户提交任务
	Result submitTask(std::shared_ptr<Task> task);


	//禁止使用赋值和赋值构造函数  
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

private:
	// 定义线程函数
	void threadFunc(int threadid);
	//检查线程池的运行状态
	bool checkRunningState() const;

private:
	//之前用裸指针传进去threads_的都是new出来的，既然有new，就得有delete，
	//但是在容器里放着也不好去delete，直接改成智能指针让它帮我们delete就行了
	//std::vector<std::unique_ptr<Thread>> threads_; // 线程列表,改为智能指针
	//用map存
	std::unordered_map<int,std::unique_ptr<Thread>> threads_;

	int initThreadSize_;  // 初始的线程数量
	int threadSizeThreshHold_; // 线程数量上限阈值
	std::atomic_bool isRunning_; // 线程池是否正在运行
	std::atomic_int idleThreadSize_; // 记录空闲线程的数量
	std::atomic_int curThreadSize_; // 当前线程数量

    //任务用队列来存储，用基类的指针或者引用才能实现多态但是需要考虑一个问题：
    //用户可能到时候pool.submitTask(concreteTask)传进来的函数只是个临时对象，
    //如果用裸指针，那么queue里面都是析构了的对象，到时候我们什么都访问不了
    //我们有必要保证这个传进来的任务的声明周期，另外也希望资源能够自动释放掉，所以用智能指针
	std::queue<std::shared_ptr<Task>> taskQue_; // 任务队列
	std::atomic_int taskSize_; // 任务的数量
	int taskQueMaxThreshHold_;  // 任务队列数量上限阈值

	std::mutex taskMutex_;//保证任务队列的线程安全
	std::condition_variable notFull_;//任务队列非满的条件变量
	std::condition_variable notEmpty_;//任务队列非空的条件变量

	PoolMode poolMode_; // 当前线程池的工作模式
	
};

#endif