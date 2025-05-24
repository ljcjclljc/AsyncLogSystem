#pragma once

#include "Util.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/ssl.hpp>

extern mylog::Util::JsonData* g_conf_data;

using boost::asio::ip::tcp;

void start_backup(boost::asio::io_context& io_context, const std::string& message)
{
    tcp::resolver resolver(io_context);
    tcp::socket socket(io_context);
    boost::asio::async_connect(socket, resolver.resolve({g_conf_data->backup_addr, std::to_string(g_conf_data->backup_port)}), 
    [&](boost::system::error_code ec, tcp::resolver::results_type::endpoint_type)
{
    if(ec)
    {
        std::cout << "connect failed: " << ec.message() << std::endl;
        return;
    }
    boost::asio::async_write(socket, boost::asio::buffer(message),[&](boost::system::error_code ec, std::size_t /*length*/)
{
    if(ec)
    {
        std::cout << "write failed: " << ec.message() << std::endl;
        return;
    }
});
});
}