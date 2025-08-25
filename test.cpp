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