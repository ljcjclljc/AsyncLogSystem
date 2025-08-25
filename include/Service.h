#pragma once
#include"DataManager.h"
#include<sys/queue.h>
#include"const.h"
#include<fcntl.h>
#include<sys/stat.h>
#include<regex>
#include"base64.h"
#include<memory>

class HttpConnection;
namespace storage
{
    class Service
    {
    public:
        Service();
        ~Service() = default;

    public:
        void Upload(std::shared_ptr<HttpConnection> conn);
        std::string TimetoStr(time_t t);
        std::string generateModernFileList(const std::vector<StorageInfo>& files);
        std::string formatSize(uint16_t bytes);
        void ListShow(std::shared_ptr<HttpConnection> conn);
        std::string GetETag(const StorageInfo& info);
        void Download(std::shared_ptr<HttpConnection> conn);
    };
}