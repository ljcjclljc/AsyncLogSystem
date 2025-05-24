#pragma once
#include <string>
#include <vector>
#include<boost/filesystem.hpp>
#include<ctime>
#include<iostream>
#include<fstream>
#include<sys/stat.h>
#include<sys/types.h>
#include<boost/property_tree/ptree.hpp>
#include<boost/property_tree/json_parser.hpp>
namespace mylog
{
    namespace Util 
    {
        class Date
        {
            public:
                static time_t Now(){ return time(nullptr); }
        };
        class File
        {
        public:
            //判断文件是否存在
            static bool Exists(const std::string& path)
            {
                return boost::filesystem::exists(path);
            }
            //获取文件路径
            static std::string Path(const std::string& path)
            {
                if(path.empty())
                {
                    return "";
                }
                boost::filesystem::path p(path);
                return p.parent_path().string();
            }
            //创建目录
            static bool createDirectory(const std::string& path)
            {
                if(path.empty())
                {
                    return false;
                }
                boost::system::error_code ec;
                boost::filesystem::path p(path);
                boost::filesystem::path dir = p;
                //std::cout<<"create directory "<<dir.c_str()<<" start"<<std::endl;
                bool ret = boost::filesystem::create_directories(dir,ec);
                if(ec){
                    //<<ec.message()<<std::endl;
                    //return false;
                }else
                {
                    //std::cout<<"create directory "<<path<<" success"<<std::endl;
                }
                return ret;
            }
            //获取文件大小
            static int64_t FileSize(const std::string& path)
            {
                boost::filesystem::path p(path);
                boost::system::error_code ec;
                auto size = boost::filesystem::file_size(p,ec);
                if(ec)
                {
                    std::cout<<ec.message()<<std::endl;
                    return -1;
                }
                return static_cast<int64_t>(size);

            }

            bool GetContent( std::string &content,std::string path)
            {
                std::ifstream file(path,std::ios::binary);
                //std::cout<<path<<std::endl;
                if(!file.is_open())
                {
                    std::cout<<"open file failed"<<std::endl;
                    return false;
                }
                file.seekg(0,std::ios::end);//移动到文件末尾
                content.resize(file.tellg());//获取文件大小
                file.seekg(0,std::ios::beg);//移动到文件开头
                file.read(&content[0],content.size());//读取文件内容

                return !file.fail();
            }
        };

        class JsonUtil
        {
            public:
            //序列化代码
            static bool Serialize(const boost::property_tree::ptree& tree,std::string& str)
            {
                try{
                    std::ostringstream oss;
                    boost::property_tree::write_json(oss,tree);
                    str = oss.str();
                    return true;
                }
                catch(const std::exception& e)
                {
                    std::cout<<e.what()<<std::endl;
                    return false;
                }
            }
            //反序列化
            static bool UnSerialize(const std::string& str,boost::property_tree::ptree& tree)
            {
                try{
                    std::istringstream iss(str);
                    boost::property_tree::read_json(iss,tree);
                    return true;
                }
                catch(const std::exception& e)
                {
                    std::cout<<e.what()<<std::endl;
                    return false;
                }
            }

        };
        struct JsonData{
            static JsonData* GetJsonData(){
                static JsonData* json_data = new JsonData();
                return json_data;
            }

            private:
                JsonData(){
                    std::string content;
                    Util::File file;
                    std::string file_path="/home/ubuntu/桌面/AsynLogSystem/log/config.conf";
                    if(file.GetContent(content,file_path)==false)
                    {
                        std::cout<<"读取配置文件失败"<<std::endl;
                        return;
                    } 
                    boost::property_tree::ptree ptree;
                    JsonUtil::UnSerialize(content,ptree);
                    buffer_size = ptree.get<size_t>("buffer_size");
                    threshold = ptree.get<size_t>("threshold");
                    linear_growth = ptree.get<size_t>("linear_growth");
                    flush_log = ptree.get<size_t>("flush_log");
                    backup_addr = ptree.get<std::string>("backup_addr");
                    backup_port = ptree.get<uint16_t>("backup_port");
                    thread_count = ptree.get<size_t>("thread_count");
                    std::cout<<buffer_size<<std::endl;
                }

            public:
                size_t buffer_size;//缓冲区基础容量
                size_t threshold;// 倍数扩容阈值
                size_t linear_growth;// 线性增长容量
                size_t flush_log;//控制日志同步到磁盘的时机，默认为0,1调用fflush，2调用fsync
                std::string backup_addr;
                uint16_t backup_port;
                size_t thread_count;
        };
    }
}


