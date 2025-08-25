// 已存在包含保护符，确保正确
#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

#include "const.h"
#include "AsioIOServicePool.h"
#include "LogicSystem.h"

class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{
friend class LogicSystem;
public:
    HttpConnection(boost::asio::io_context& ioc);
    void Start();
    tcp::socket& get_socket(){
        return _socket;
    };
public:
    void CheckDeadline();
    void WriteResponse();
    void HandleReq();
    void PreParseGetParam();
    tcp::socket  _socket;
    beast::flat_buffer  _buffer{ 100*1024*1024 };
    http::request<http::dynamic_body> _request;
    http::response<http::dynamic_body> _response;
    net::steady_timer deadline_{
        _socket.get_executor(), std::chrono::seconds(30) };

    std::string _get_url;
    std::unordered_map<std::string, std::string> _get_params;
};

#endif // HTTP_CONNECTION_H