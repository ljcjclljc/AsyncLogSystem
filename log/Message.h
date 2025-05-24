#pragma once

#include<memory>
#include<thread>
#include<sstream>
#include"Level.h"
#include"Util.h"

namespace mylog
{
inline std::string IdToString(std::thread::id tid)
{
    std::stringstream ss;
    ss<<tid;
    return ss.str();
}
    struct LogMessage
    {
        using ptr = std::shared_ptr<LogMessage>;
        LogMessage() = default;
        LogMessage(LogLevel::value level, const std::string file,size_t line,
        std::string name, std::string paylod):
        name_(name),
        file_name_(file),
        payload_(paylod),
        level_(level),
        line_(line),
        ctime_(Util::Date::Now()),
        tid_(std::this_thread::get_id()){}

        std::string format()
        {
            struct tm t;
            localtime_r(&ctime_,&t);
            char buf[128] = {0};
            strftime(buf,sizeof(buf),"%H:%M:%S",&t);
            // boost::format fmt("[%s][%16X][%s][%s:%zu]\t%s\n");
            // fmt % std::string(buf)%IdToString(tid_)%LogLevel::toString(level_)%name_%line_%payload_;
            std::stringstream fmt;
            fmt<<"["<<buf<<"]["<<IdToString(tid_)<<"]["<<LogLevel::toString(level_)<<"]["<< name_ << ":" << line_ << "]\t" << payload_ << "\n";
            return fmt.str();
        }

        size_t line_;//行号
        time_t ctime_; //创建时间
        std::string file_name_; //文件名
        std::string name_; //日志名
        std::string payload_; //日志内容
        LogLevel::value level_; //日志级别
        std::thread::id tid_; //线程id
    };
}
