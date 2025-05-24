#include<unordered_map>
#include"AsyncLogger.h"

namespace mylog
{
    class LoggerManager
    {
        public:
            static LoggerManager &GetInstance()
            {
                static LoggerManager instance;
                return instance;
            }
            
            bool LoggerExist(const std::string &name)
            {
                std::lock_guard<std::mutex> lock(mtx_);
                auto it = loggers_.find(name);
                if(it!=loggers_.end())
                return true;
                else
                return false;
            }

            void AddLogger(const AsyncLogger::ptr &&AsyncLogger)
            {
                if(LoggerExist(AsyncLogger->Name()))
                return;
                std::lock_guard<std::mutex> lock(mtx_);
                //std::cout<<loggers_.size()<<std::endl;
                loggers_.insert(std::make_pair(AsyncLogger->Name(),AsyncLogger));
                //std::cout<<loggers_.size()<<std::endl;
            }

            AsyncLogger::ptr GetLogger(const std::string &name)
            {
                std::lock_guard<std::mutex> lock(mtx_);
                auto it = loggers_.find(name);
                if(it!=loggers_.end())
                return it->second;
                else
                return AsyncLogger::ptr();
            }
            void GetAllLogger()
            {
                for(auto &logger_ : loggers_)
                {
                    std::cout<<logger_.first<<std::endl;
                }
            }
            AsyncLogger::ptr DefaultLogger() {return default_logger_;}

        private:
            LoggerManager()
            {
                std::unique_ptr<LoggerBuilder> builder(new LoggerBuilder());
                //std::unique_ptr<LoggerBuilder> builder(new LoggerBuilder());
                builder->BuildLoggerName("default");
                default_logger_ = builder->Build();
                loggers_.insert(std::make_pair("default", default_logger_));
            }
        public:
            std::mutex mtx_;
            std::unordered_map<std::string, AsyncLogger::ptr> loggers_;
            AsyncLogger::ptr default_logger_;//默认日志器
    };
};