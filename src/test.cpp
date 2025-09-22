/* ************************************************************************
> File Name:     test.cpp
> Created Time:  Thu 07 Sep 2023 06:37:16 PM CST
> Description:
 ************************************************************************/
#define DEBUG_LOG
#include "Service.h"
#include <thread>
#include"test.h"
#include"const.h"
#include"LogicSystem.h"
#include"CServer.h"
#include"AsioIOServicePool.h"
// #include"fastdfs_client.h"  // 注释掉系统的 fastdfs 头文件
// #include"fastdfs_client.h"  // 重复的包含，已注释
#include"FastDFSClient.h"   // 真实的 FastDFS 客户端（需要系统库）
// #include"MockFastDFSClient.h"  // 使用模拟的 FastDFS 客户端（不需要系统库）
using namespace std;

namespace storage {
    DataManager *data_ = nullptr;
}

ThreadPool* tp=nullptr;
mylog::Util::JsonData* g_conf_data;
void service_module()
{
    // storage::Service s;
    // mylog::GetLogger("asynclogger")->Info("service step in RunModule");
    // s.RunModule();
    unsigned short gate_port = 8080;
    try
    {
        unsigned short port = static_cast<unsigned short>(gate_port);
        auto& ioc = AsioIOServicePool::GetInstance()->GetIOService();
        //net::io_context ioc{1};
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const boost::system::error_code& ec, int signal_number) {
            if (ec) {
                return;
            }

            ioc.stop();
        });
        std::make_shared<CServer>(ioc, port)->start();
        std::cout<<"server start at port:"<<port<<std::endl;
        ioc.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}

void log_system_module_init()
{
    g_conf_data = mylog::Util::JsonData::GetJsonData();
    tp = new ThreadPool(g_conf_data->thread_count);
    std::shared_ptr<mylog::LoggerBuilder> Glb(new mylog::LoggerBuilder());
    Glb->BuildLoggerName("asynclogger");
    Glb->BuildLoggerFlush<mylog::RollFileFlush>("./logfile/RollFile_log",
                                              1024 * 1024);
    // The LoggerManger has been built and is managed by members of the LoggerManger class
    //The logger is assigned to the managed object, and the caller lands the log by invoking the singleton managed object
    mylog::LoggerManager::GetInstance().AddLogger(Glb->Build());
}
int main()
{
    // 创建 FastDFS 客户端
    FastDFSClient client("/etc/fdfs/client.conf");
    
    // 初始化客户端
    if (!client.initialize()) {
        std::cerr << "初始化失败: " << client.getLastError() << std::endl;
        return 1;  // 返回错误码
    }
    
    std::cout << "FastDFS 客户端初始化成功" << std::endl;

    log_system_module_init();
    // test t;
    // t=test();
    storage::data_ = new storage::DataManager();
    //std::cout << "主线程 ID: " << std::this_thread::get_id() << std::endl;
    thread t1(service_module);

    t1.join();
    delete(tp);
    return 0;
}