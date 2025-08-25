#pragma once
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <mutex>              // for std::mutex
#include <condition_variable> // for std::condition_variable
#include <queue>              // for std::queue
#include <atomic>             // for std::atomic
#include <memory>
#include <iostream>
#include <unordered_map>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/reader.h>
#include "Singleton.h"
#include <assert.h>
#include <queue>
// #include <mysqlcppconn/mysql_driver.h>
// #include <mysqlcppconn/mysql_connection.h>
// #include <mysqlcppconn/cppconn/prepared_statement.h>
// #include <mysqlcppconn/cppconn/resultset.h>
// #include <mysqlcppconn/cppconn/statement.h>
// #include <mysqlcppconn/cppconn/exception.h>

//#include<mysqlx/xdevapi.h>
#include <iostream>
#include <functional>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <string>
// #include <hiredis/hiredis.h>
#include <fstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <map>
#include <iostream>
#include <boost/filesystem.hpp>
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
