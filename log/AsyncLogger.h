#pragma once
#include <atomic>
#include <cassert>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <vector>
#include<cstdarg>

#include "Level.h"
#include "AsyncWorker.h"
#include "Message.h"
#include "LogFlush.h"
#include "Threadpool.h"

extern ThreadPool *tp;

namespace mylog
{
    class AsyncLogger
    {
        public:
            using ptr = std::shared_ptr<AsyncLogger>;
            AsyncLogger(const std::string& logger_name,std::vector<LogFlush::ptr> flush,AsyncType type)
            :logger_name_(logger_name),
            flush_(flush),
            asyncworker_(std::make_shared<AsyncWorker>
                (std::bind(&AsyncLogger::RealFlush,this,std::placeholders::_1),type)){}
            virtual ~AsyncLogger() {};
            std::string Name() {return logger_name_;}

            void Debug(const std::string &file, size_t line, const std::string format,
                ...)
     {
         // 获取可变参数列表中的格式
         va_list va;
         va_start(va, format);
         char *ret;
         int r = vasprintf(&ret, format.c_str(), va);
         if (r == -1)
             perror("vasprintf failed!!!: ");
         va_end(va); // 将va指针置空
     
         serialize(LogLevel::value::DEBUG, file, line,
                   ret); // 生成格式化日志信息并写文件
     
         free(ret);
         ret = nullptr;
     };


     void Info(const std::string &file, size_t line, const std::string format,
                ...)
     {
         // 获取可变参数列表中的格式
         va_list va;
         va_start(va, format);
         char *ret;
         int r = vasprintf(&ret, format.c_str(), va);
         if (r == -1)
             perror("vasprintf failed!!!: ");
         va_end(va); // 将va指针置空
     
         serialize(LogLevel::value::INFO, file, line,
                   ret); // 生成格式化日志信息并写文件
     
         free(ret);
         ret = nullptr;
     };

     void Warn(const std::string &file, size_t line, const std::string format,
                ...)
     {
         // 获取可变参数列表中的格式
         va_list va;
         va_start(va, format);
         char *ret;
         int r = vasprintf(&ret, format.c_str(), va);
         if (r == -1)
             perror("vasprintf failed!!!: ");
         va_end(va); // 将va指针置空
     
         serialize(LogLevel::value::WARN, file, line,
                   ret); // 生成格式化日志信息并写文件
     
         free(ret);
         ret = nullptr;
     };

     void Error(const std::string &file, size_t line, const std::string format,
                ...)
     {
         // 获取可变参数列表中的格式
         va_list va;
         va_start(va, format);
         char *ret;
         int r = vasprintf(&ret, format.c_str(), va);
         if (r == -1)
             perror("vasprintf failed!!!: ");
         va_end(va); // 将va指针置空
     
         serialize(LogLevel::value::ERROR, file, line,
                   ret); // 生成格式化日志信息并写文件
     
         free(ret);
         ret = nullptr;
     };
     void Fatal(const std::string &file, size_t line, const std::string format,
        ...)
    {
        // 获取可变参数列表中的格式
        va_list va;
        va_start(va, format);
        char *ret;
        int r = vasprintf(&ret, format.c_str(), va);
        if (r == -1)
            perror("vasprintf failed!!!: ");
        va_end(va); // 将va指针置空

        serialize(LogLevel::value::FATAL, file, line,
                ret); // 生成格式化日志信息并写文件

        free(ret);
        ret = nullptr;
    };

    protected:
         void serialize(LogLevel::value level, const std::string &file, size_t line,
        char* ret)
        {
            mylog::LogMessage msg(level,file,line,logger_name_,ret);
            std::string msg_str = msg.format();
            if(level == LogLevel::value::FATAL||
               level == LogLevel::value::ERROR)
               {
                try
                {
                    // auto ret=tp->enqueue(start_backup,msg_str);
                    // ret.get();
                }
                catch (const std::runtime_error &e)
                {
                // 该线程池没有把stop设置为true的逻辑，所以不做处理
                std::cout << __FILE__ << __LINE__ << "thread pool closed" << std::endl;
                }
               }
               Flush(msg_str.c_str(),msg_str.size());

        }

        void Flush(const char *data, size_t len)
        {
            asyncworker_->Push(data,len);
        }

        void RealFlush(Buffer &buffer)
        {
            if(flush_.empty())
            {
                return;
            }
            for(auto& flush:flush_)
            {
                flush->Flush(buffer.Begin(),buffer.ReadableSize());
            }
        }
        private:
            std::mutex mutex_;
            std::string logger_name_;
            std::vector<LogFlush::ptr> flush_;
            mylog::AsyncWorker::ptr asyncworker_;

    };

    class LoggerBuilder
    {
    public:
        using ptr = std::shared_ptr<LoggerBuilder>;
        void BuildLoggerName(const std::string &name) { logger_name_ = name; }
        void BuildLopperType(AsyncType type) { async_type_ = type; }
        template <typename FlushType, typename... Args>
        void BuildLoggerFlush(Args &&...args)
        {
            flushs_.emplace_back(
                LogFlushFactory::CreateLog<FlushType>(std::forward<Args>(args)...));
        }
        AsyncLogger::ptr Build()
        {
            assert(logger_name_.empty() == false);// 必须有日志器名称
            //std::cout << "logger name:" << logger_name_ << std::endl;
            
            // 如果写日志方式没有指定，那么采用默认的标准输出
            if (flushs_.empty())
                flushs_.emplace_back(std::make_shared<StdoutFlush>());
            return std::make_shared<AsyncLogger>(
                logger_name_, flushs_, async_type_);
        }

    protected:
        std::string logger_name_ = "async_logger"; // 日志器名称
        std::vector<mylog::LogFlush::ptr> flushs_; // 写日志方式
        AsyncType async_type_ = AsyncType::ASYNC_SAFE;//用于控制缓冲区是否增长
    };
}