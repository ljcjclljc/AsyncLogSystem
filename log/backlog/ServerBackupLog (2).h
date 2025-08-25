#pragma once
#include <iostream>
#include <string>
#include <memory>
#include <functional>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;
using fun_c=std::function<void(const std::string&)>;
class TcpServer;
class TcpConnection:public std::enable_shared_from_this<TcpConnection>{
    public:
        
        /**
         * @brief TCP连接构造函数
         * @param socket TCP套接字对象,使用右值引用进行移动
         * @param c 回调函数对象
         * @details 创建新的TCP连接,将传入的socket移动到socket_成员,保存回调函数c到c_成员
         */
        TcpConnection(tcp::socket socket, fun_c c):socket_(std::move(socket)), c_(c){
            std::cout << "new connection" << std::endl;
        };
        void start(){
            do_read();
        };
        void do_read(){
            std::shared_ptr<TcpConnection> self(shared_from_this());
            boost::asio::async_read_until(socket_,buffer_,'\n',
            [this,self](boost::system::error_code ec, std::size_t length){
                if(!ec){
                    std::istream is(&buffer_);
                    std::string message;
                    std::getline(is, message);
                    c_(message);
                    do_read();
                }
            });
        }
    private:
        tcp::socket socket_;
        boost::asio::streambuf buffer_;
        fun_c c_;
};
class TcpServer{
    public:
        TcpServer(boost::asio::io_context& io_context, short port):acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
        socket_(io_context){
            do_accept();
        };
        void do_accept(){
            acceptor_.async_accept(socket_, [this](boost::system::error_code ec){
                if(!ec){
                    std::make_shared<TcpConnection>(std::move(socket_), c_)->start();
                }
                do_accept();
            });
        };
    private:
        tcp::acceptor acceptor_;
        tcp::socket socket_;
        fun_c c_;
};