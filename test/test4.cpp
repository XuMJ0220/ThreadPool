#include "threadpool.hpp"

using ULong = unsigned long long;
//并发计算，实现1+2+3+4+...+300000000
class MyTask:public Task{
    private:
        ULong begin_;
        ULong end_;
    public:
        MyTask(ULong begin,ULong end)
        :begin_(begin),end_(end)
        {
        }

        MyAny run(){
            std::cout<<"线程: "<<std::this_thread::get_id()<<"开始"<<std::endl;
            ULong sum = 0;
            for(ULong i = begin_;i<=end_;i++){
                sum += i;
            }
            std::cout<<"线程: "<<std::this_thread::get_id()<<"结束"<<std::endl;
            return sum;
        }
};

int main(){
    ThreadPool pool;
    pool.setMode(PoolMode::MODE_CACHED);
    pool.start(4);

    Result res1 = pool.submitTask(std::make_shared<MyTask>(1,100000000));
    Result res2 = pool.submitTask(std::make_shared<MyTask>(100000001,200000000));
    Result res3 = pool.submitTask(std::make_shared<MyTask>(200000001,300000000));
    pool.submitTask(std::make_shared<MyTask>(200000001,300000000));
    pool.submitTask(std::make_shared<MyTask>(200000001,300000000));
    pool.submitTask(std::make_shared<MyTask>(200000001,300000000));
    //ULong sum1 = res1.get().cast_<ULong>();
    //ULong sum2 = res2.get().cast_<ULong>();
   // ULong sum3 = res3.get().cast_<ULong>();
    //std::cout<<"sum = "<<sum1+sum2+sum3<<std::endl;
    //ULong sum = 0;
    // for(ULong i = 1;i<=300000000;i++){
    //     sum += i;
    // }
    // std::cout<<"sum = "<<sum<<std::endl; 
    for(;;){}
}