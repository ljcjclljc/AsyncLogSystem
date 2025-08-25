#pragma once
#include "Service.h"
#include <thread>
class test
{
    public:
    test()
    {
        //std::cout<<"test"<<std::endl;
        mylog::LoggerManager::GetInstance().GetAllLogger();
    }
};