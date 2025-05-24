#pragma once
#include<atomic>
#include<thread>
#include<condition_variable>
#include<functional>
#include<iostream>
#include<mutex>

#include"AsyncBuffer.h"

namespace mylog{
enum class AsyncType{ ASYNC_SAFE,ASYNC_UNSAFE };
using functor = std::function<void(Buffer&)>;
class AsyncWorker{
    public:
        using ptr = std::shared_ptr<AsyncWorker>;
        AsyncWorker(const functor &cb,AsyncType async_type = AsyncType::ASYNC_SAFE)
        :callback_(cb),
        async_type_(async_type),
        stop_(false),
        thread_(std::thread(&AsyncWorker::ThreadEntry,this)){};
    ~AsyncWorker(){
        stop();
    }

    void Push(const char* data,size_t len)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if(AsyncType::ASYNC_SAFE == async_type_)
        {
            cond_productor_.wait(lock,[&]{
                return len<=buffer_productor_.WriteableSize();
            });
        }
        buffer_productor_.Push(data,len);
        cond_consumer_.notify_one();
    }

    void stop()
    {
        stop_ = true;
        cond_consumer_.notify_one();
        thread_.join();
    }

    private:
        void ThreadEntry()
        {
            //std::cout << "工作线程 ID: " << std::this_thread::get_id() << std::endl;
            while(1){
                //缓冲区交换完枷锁，productor继续读取数据
                std::unique_lock<std::mutex> lock(mutex_);
                if(buffer_productor_.IsEmpty()&&stop_)
                {
                    cond_consumer_.wait(lock,[&]{
                        return stop_||!buffer_productor_.IsEmpty();
                    });
                }
                buffer_productor_.Swap(buffer_consumer_);
                //固定大小缓冲区交换完解锁，consumer继续读取数据
                if(async_type_==AsyncType::ASYNC_SAFE)
                {
                    cond_consumer_.notify_one();
                }
                callback_(buffer_consumer_);
                buffer_consumer_.Reset();
                if(stop_&&buffer_productor_.IsEmpty())
                {
                    return;
                }
            }
        }
      private:
        AsyncType async_type_;
        std::atomic<bool> stop_;//控制异步工作器是否停止
        std::mutex mutex_;
        mylog::Buffer buffer_productor_;
        mylog::Buffer buffer_consumer_;
        std::condition_variable cond_productor_;
        std::condition_variable cond_consumer_;
        std::thread thread_;

        functor callback_;
};
}
