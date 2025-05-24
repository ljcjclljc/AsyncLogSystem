#pragma once
#include "Util.h"
#include <boost/thread/once.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <memory>
#include "../log/Mylog.h"

namespace storage{
    //配置文件
    const std::string Config_File = "Storage.conf";

    class Config : boost::noncopyable{
        public:
        //单例模式
        static Config* GetInstance(){
            //使用call_once代替双重检查加锁
            static boost::once_flag flag;
            //std::cout << "Config::GetInstance" << std::endl;
            static std::unique_ptr<Config> instance;
            //std::cout << "Config::GetInstance" << std::endl;
            boost::call_once(flag, [](){
                std::cout << "Config::GetInstance" << std::endl;
                instance.reset(new Config());
            });
            //std::cout << "Config::GetInstance" << std::endl;
            return instance.get();
        }

        bool ReadConfig(){
            //std::cout << "ReadConfig start" << std::endl;
            mylog::GetLogger("asynclogger")->Info("ReadConfig start");
            //std::cout<<Config_File<<std::endl;
            storage::FileUtil fu(Config_File);
            std::string content;
            if (!fu.GetContent(&content))
            {
                return false;
            }

            Json::Value root;
            //std::cout << "Config::GetInstance1" << std::endl;
            storage::JsonUtil::Unserialize(content, &root); // 反序列化，把内容转成jaon value格式
            //std::cout << "Config::GetInstance2" << std::endl;
            // 要记得转换的时候用上asint，asstring这种函数，json的数据类型是Value。
            server_port_ = root["server_port"].asInt();
            server_ip = root["server_ip"].asString();
            download_prefix_ = root["download_prefix"].asString();
            storage_info_ = root["storage_info"].asString();
            deep_storage_dir_ = root["deep_storage_dir"].asString();
            low_storage_dir_ = root["low_storage_dir"].asString();
            bundle_format_ = root["bundle_format"].asInt();
            
            return true;
        }

        //获取配置值
        int GetServerPort() const { return server_port_; }
        const std::string& GetServerIp() const { return server_ip; }
        const std::string& GetDownloadPrefix() const { return download_prefix_; }
        int GetBundleFormat() const { return bundle_format_; }
        const std::string& GetDeepStorageDir() const { return deep_storage_dir_; }
        const std::string& GetLowStorageDir() const { return low_storage_dir_; }
        const std::string& GetStorageInfoFile() const { return storage_info_; }
        
        private:
        Config():server_port_(0),server_ip(""),download_prefix_(""),storage_info_(""),deep_storage_dir_(""),low_storage_dir_(""),bundle_format_(0){
            //std::cout<<"Config()"<<std::endl;
            ReadConfig();
        }
        int server_port_;
        std::string server_ip;
        std::string download_prefix_;  // URL路径前缀
        std::string deep_storage_dir_; // 深度存储文件的存储路径
        std::string low_storage_dir_;  // 浅度存储文件的存储路径
        std::string storage_info_;     // 已存储文件的信息
        int bundle_format_;            // 深度存储的文件格式
    };
}