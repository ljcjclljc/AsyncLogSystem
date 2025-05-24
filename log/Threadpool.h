#pragma once
#include<vector>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<functional>
#include<future>
#include<queue>
#include<atomic>
#include<deque>
#include<map>
#include<boost/asio/thread_pool.hpp>
#include<boost/asio/post.hpp>
#include<memory>
#include<stdexcept>
#include<functional>

class ThreadPool
{
public:
    ThreadPool(size_t threads):pool(threads){}

    template<class F,class... Args>
    auto enqueue(F&& f,Args&&... args)->
    std::future<typename std::result_of<F(Args...)>>
    {
        using return_type = typename std::result_of<F(Args...)>::type;
        auto task = std::make_shared<std::packaged_task<return_type()>>
        (std::bind(std::forward<F>(f),std::forward<Args>(args)...));
        std::future<return_type> res = task->get_future();
        boost::asio::post(pool,[task](){(*task)();});

        return res;

    }

    ~ThreadPool()
    {
        pool.join();
    }
private:
    boost::asio::thread_pool pool;
};
