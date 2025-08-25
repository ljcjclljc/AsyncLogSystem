#pragma once
#include "Singleton.h"
#include <functional>
#include <map>
#include "const.h"
class HttpConnection;
namespace storage { class Service; }
typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;
class LogicSystem :public Singleton<LogicSystem>
{
    friend class Singleton<LogicSystem>;
public:
    //~LogicSystem();
    bool HandleGet(std::string, std::shared_ptr<HttpConnection>);
    void RegGet(std::string, HttpHandler handler);
    void RegPost(std::string url, HttpHandler handler);
    bool HandlePost(std::string path, std::shared_ptr<HttpConnection> con);
private:
    LogicSystem();
    std::map<std::string, HttpHandler> _post_handlers;
    std::map<std::string, HttpHandler> _get_handlers;
    storage::Service* _service;
};