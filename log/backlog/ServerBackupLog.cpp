#pragma once
#include "ServerBackupLog.h"

const std::string filename="./logfili.log";

void usage(std::string procgress)
{
    std::cout<<"Usage: "<<procgress<<" [options]"<<std::endl;
    std::cout<<"Options:"<<std::endl;
}
bool file_exist(const std::string& name)
{
    struct stat exist;
    return stat(name.c_str(), &exist) == 0;
}
void backuo_log(std::string& message)
{
    FILE* fp = fopen(filename.c_str(), "a+");
    if(fp == NULL)
    {
        std::cout<<"open file failed"<<std::endl;
        return;
    }
    int write_len = fwrite(message.c_str(), 1, message.size(), fp);
    if(write_len != message.size())
    {
        std::cout<<"write file failed"<<std::endl;
    }
    fclose(fp);
}

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        usage(argv[0]);
        return 0;
    }

    uint16_t port = atoi(argv[1]);
    boost::asio::io_context io_context;
    TcpServer server(io_context, port);
    
    io_context.run();
}