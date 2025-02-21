#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <vector>
#include <queue>
#include <atomic>
#include <memory>
#include <thread>
#include <functional>
#include <iostream>
class Thread{
    private:
        std::function<void(void)> threadFun_;
    public:
        //构造函数
        Thread(std::function<void(void)>);

        //析构函数
        ~Thread();

        //开始线程
        void start();
};

// 任务抽象基类
class Task
{
    public:
        //构造函数
        Task();
        
        //析构函数
        ~Task();
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

	//禁止使用赋值和赋值构造函数  
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

private:
	// 定义线程函数
	void threadFunc();

private:
	std::vector<Thread*> threads_; // 线程列表,先用裸指针,后面再用智能指针

	int initThreadSize_;  // 初始的线程数量
	int threadSizeThreshHold_; // 线程数量上限阈值

    //任务用队列来存储，用基类的指针或者引用才能实现多态但是需要考虑一个问题：
    //用户可能到时候pool.submitTask(concreteTask)传进来的函数只是个临时对象，
    //如果用裸指针，那么queue里面都是析构了的对象，到时候我们什么都访问不了
    //我们有必要保证这个传进来的任务的声明周期，另外也希望资源能够自动释放掉，所以用智能指针
	std::queue<std::shared_ptr<Task>> taskQue_; // 任务队列
	std::atomic_int taskSize_; // 任务的数量
	int taskQueMaxThreshHold_;  // 任务队列数量上限阈值

	PoolMode poolMode_; // 当前线程池的工作模式
};

#endif