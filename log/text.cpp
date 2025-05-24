// #include"Level.h"
// #include"Util.h"
// #include"LogFlush.h"

// mylog::Util::JsonData* g_json_data = mylog::Util::JsonData::GetJsonData();
// int main()
// {
    

//     // 输出读取到的配置信息
    // std::cout << "buffer_size: " << g_json_data->buffer_size << std::endl;
    // std::cout << "threshold: " << g_json_data->threshold << std::endl;
    // std::cout << "linear_growth: " << g_json_data->linear_growth << std::endl;
    // std::cout << "flush_log: " << g_json_data->flush_log << std::endl;
    // std::cout << "backup_addr: " << g_json_data->backup_addr << std::endl;
    // std::cout << "backup_port: " << g_json_data->backup_port << std::endl;
    // std::cout << "thread_count: " << g_json_data->thread_count << std::endl;

//     // std::string testData = "Test data for StdoutFlush\n";
//     // auto stdoutFlush = mylog::LogFlushFactory::CreateLog<mylog::stdoutLogFlush>();
//     // stdoutFlush->Flush(testData.c_str(), testData.length());
//     // std::cout << "StdoutFlush test passed." << std::endl;

// std::cout<<"---------------------------------------------------------"<<'\n';
//     // std::string testFileName = "./logfile/FileFlush.log";
//     // std::string testData = "Test data for FileFlush\n";
//     // auto fileFlush = mylog::LogFlushFactory::CreateLog<mylog::FileFlush>(testFileName);
//     // std::cout << "FileFlush test start." << std::endl;
//     // fileFlush->Flush(testData.c_str(), testData.length());

//     // // 检查文件是否存在并包含写入的数据
//     // std::ifstream file(testFileName);
//     // std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
//     // assert(fileContent.find(testData) != std::string::npos);
//     // std::cout << "FileFlush test passed." << std::endl;
//     std::cout<<"---------------------------------------------------------"<<'\n';
//     std::string testBaseName = "./logfile/test_roll_file";
//     size_t maxSize = 50;
//     std::string testData = "Test data for RollFileFlush\n";
//     auto rollFileFlush = mylog::LogFlushFactory::CreateLog<mylog::RollFileFlush>(testBaseName, maxSize);

//     // 多次写入数据以触发滚动
//     for (int i = 0; i < 20; ++i) {
//         rollFileFlush->Flush(testData.c_str(), testData.length());
//     }

// }
#include "Mylog.h"
#include "Threadpool.h"
#include "Util.h"
using std::cout;
using std::endl;

ThreadPool* tp=nullptr;
mylog::Util::JsonData* g_conf_data;
void test() {
    int cur_size = 0;
    int cnt = 1;
    while (cur_size++ < 2) {
        mylog::GetLogger("asynclogger")->Info("测试日志-%d", cnt++);
        mylog::GetLogger("asynclogger")->Warn("测试日志-%d", cnt++);
        mylog::GetLogger("asynclogger")->Debug("测试日志-%d", cnt++);
        mylog::GetLogger("asynclogger")->Error("测试日志-%d", cnt++);
        mylog::GetLogger("asynclogger")->Fatal("测试日志-%d", cnt++);
    }
}

void init_thread_pool() {
    std::cout << "buffer_size: " << g_conf_data->buffer_size << std::endl;
    std::cout << "threshold: " << g_conf_data->threshold << std::endl;
    std::cout << "linear_growth: " << g_conf_data->linear_growth << std::endl;
    std::cout << "flush_log: " << g_conf_data->flush_log << std::endl;
    std::cout << "backup_addr: " << g_conf_data->backup_addr << std::endl;
    std::cout << "backup_port: " << g_conf_data->backup_port << std::endl;
    std::cout << "thread_count: " << g_conf_data->thread_count << std::endl;
    tp = new ThreadPool(g_conf_data->thread_count);
}
int main() {
    g_conf_data = mylog::Util::JsonData::GetJsonData();
    init_thread_pool();
    std::shared_ptr<mylog::LoggerBuilder> Glb(new mylog::LoggerBuilder());
    Glb->BuildLoggerName("asynclogger");
    Glb->BuildLoggerFlush<mylog::FileFlush>("./logfile/FileFlush.log");
    Glb->BuildLoggerFlush<mylog::RollFileFlush>("./logfile/RollFile_log",1024 * 1024);
    std::cout << "主线程 ID: " << std::this_thread::get_id() << std::endl;
    //建造完成后，日志器已经建造，由LoggerManger类成员管理诸多日志器
    // 把日志器给管理对象，调用者通过调用单例管理对象对日志进行落地
    mylog::LoggerManager::GetInstance().AddLogger(Glb->Build());
    test();
    delete(tp);
    return 0;
}