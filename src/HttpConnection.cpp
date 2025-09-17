#include "HttpConnection.h"

HttpConnection::HttpConnection(boost::asio::io_context& ioc):_socket(ioc)
{
}

void HttpConnection::Start()
{
	//std::cout << "Start" << std::endl;
	auto self = shared_from_this();
	http::async_read(_socket, _buffer, _request, [self](beast::error_code ec, std::size_t bytes_transfered) {
		try {
			
			//error_code重载了==可以用于判断返回bool类型数
			boost::ignore_unused(bytes_transfered);
			self->HandleReq();
			//self->CheckDeadline();
		}
		catch (std::exception& exp)
		{
			std::cout << "exception is " << exp.what() << std::endl;
		}
	});
}

void HttpConnection::CheckDeadline()
{
	auto self = shared_from_this();
	// 设置超时时间（例如 5 秒无活动则关闭连接）
	deadline_.async_wait([self](beast::error_code ec) {
		if (!ec){
			// 超时关闭连接
            beast::error_code ec_close;
            self->_socket.close(ec_close);
		}
		});
}

void HttpConnection::WriteResponse()
{
	auto self = shared_from_this();
	_response.content_length(_response.body().size());
	bool keep_alive = _response.keep_alive();
    if(!_socket.is_open())
	{
		return;
	}
    http::async_write(_socket, _response, [self, keep_alive](beast::error_code ec, std::size_t bytes_transfered) {
        if (ec) {
            return;
        }

        // 如果是长连接，重置状态并等待新请求
      
            // 重置请求和缓冲区
            self->_request = http::request<http::dynamic_body>();
            self->_buffer.consume(self->_buffer.size());
            
            // 取消之前可能存在的超时
        //    self->deadline_.cancel();
            
            // 重新启动超时
            // self->deadline_.expires_after(std::chrono::seconds(5));
            // self->CheckDeadline();
            
            // 等待新请求
			
            self->Start();
        
    });
}

unsigned char ToHex(unsigned char x)
{
	return  x > 9 ? x + 55 : x + 48;
}

unsigned char FromHex(unsigned char x)
{
	unsigned char y;
	if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
	else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
	else if (x >= '0' && x <= '9') y = x - '0';
	else assert(0);
	return y;
}

std::string UrlEncode(const std::string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		//判断是否仅有数字和字母构成
		if (isalnum((unsigned char)str[i]) ||
			(str[i] == '-') ||
			(str[i] == '_') ||
			(str[i] == '.') ||
			(str[i] == '~'))
			strTemp += str[i];
		else if (str[i] == ' ') //为空字符
			strTemp += "+";
		else
		{
			//其他字符需要提前加%并且高四位和低四位分别转为16进制
			strTemp += '%';
			strTemp += ToHex((unsigned char)str[i] >> 4);
			strTemp += ToHex((unsigned char)str[i] & 0x0F);
		}
	}
	return strTemp;
}

std::string UrlDecode(const std::string& str)
{
	std::string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		//还原+为空
		if (str[i] == '+') strTemp += ' ';
		//遇到%将后面的两个字符从16进制转为char再拼接
		else if (str[i] == '%')
		{
			assert(i + 2 < length);
			unsigned char high = FromHex((unsigned char)str[++i]);
			unsigned char low = FromHex((unsigned char)str[++i]);
			strTemp += high * 16 + low;
		}
		else strTemp += str[i];
	}
	return strTemp;
}

void HttpConnection::PreParseGetParam() {
	// 提取 URI  
	auto uri = _request.target();
	// 查找查询字符串的开始位置（即 '?' 的位置）  
	auto query_pos = uri.find('?');
	if (query_pos == std::string::npos) {
		 _get_url = std::string(uri);
		return;
	}

	
	_get_url = std::string(uri);
	
	// 第122行修复
	_get_url = std::string(uri.substr(0, query_pos));
	
	// 第123行修复
	std::string query_string = std::string(uri.substr(query_pos + 1));
	std::string key;
	std::string value;
	size_t pos = 0;
	while ((pos = query_string.find('&')) != std::string::npos) {
		auto pair = query_string.substr(0, pos);
		size_t eq_pos = pair.find('=');
		if (eq_pos != std::string::npos) {
			key = UrlDecode(pair.substr(0, eq_pos)); // 假设有 url_decode 函数来处理URL解码  
			value = UrlDecode(pair.substr(eq_pos + 1));
			_get_params[key] = value;
		}
		query_string.erase(0, pos + 1);
	}
	// 处理最后一个参数对（如果没有 & 分隔符）  
	if (!query_string.empty()) {
		size_t eq_pos = query_string.find('=');
		if (eq_pos != std::string::npos) {
			key = UrlDecode(query_string.substr(0, eq_pos));
			value = UrlDecode(query_string.substr(eq_pos + 1));
			_get_params[key] = value;
		}
	}
}

void HttpConnection::HandleReq()
{
	//设置版本
	_response.version(_request.version());
	//根据请求设置长连接
	_response.keep_alive(true);
	// 添加 OPTIONS 请求处理
	if (_request.method() == http::verb::options) {
		_response.result(http::status::ok);
		_response.set(http::field::access_control_allow_origin, "*");
		_response.set(http::field::access_control_allow_methods, "GET, POST, OPTIONS");
		_response.set(http::field::access_control_allow_headers, "StorageType, FileName, Cache-Control");
		_response.set(http::field::content_length, 0);
		WriteResponse();
		return;
	}

	if (_request.method() == http::verb::get) {
		PreParseGetParam();
		//std::cout << "get url is " << std::string(_request.target()) << std::endl;
		if(std::string(_request.target()).find("/download/")!=std::string::npos){
		bool success = LogicSystem::GetInstance()->HandleGet("/download/", shared_from_this());
		if (!success) {
			_response.result(http::status::not_found);
			_response.set(http::field::content_type, "text/plain");
			beast::ostream(_response.body()) << "url not found\r\n";
			WriteResponse();
			return;
		}
		}else
		{
			//std::cout<<"get url is "<<std::string(_request.target())<<std::endl;
		bool success = LogicSystem::GetInstance()->HandleGet(std::string(_request.target()), shared_from_this());
		if (!success) {
			_response.result(http::status::not_found);
			_response.set(http::field::content_type, "text/plain");
			beast::ostream(_response.body()) << "url not found\r\n";
			WriteResponse();
			return;
		}
		}
		

		_response.result(http::status::ok);
		_response.set(http::field::server, "GateServer");
		WriteResponse();
		return;
	}

	if (_request.method() == http::verb::post) {
		PreParseGetParam();
		//std::cout << "post url is " << std::string(_request.target()) << std::endl;
		bool success = LogicSystem::GetInstance()->HandlePost(std::string(_request.target()), shared_from_this());
		if (!success) {
			_response.result(http::status::not_found);
			_response.set(http::field::content_type, "text/plain");
			beast::ostream(_response.body()) << "url not found\r\n";
			WriteResponse();
			return;
		}
		// 处理POST请求
		_response.result(http::status::ok);
        _response.set(http::field::content_type, "text/plain");
		WriteResponse();
		return;
	}

	// 添加默认请求处理 - 处理除OPTIONS、GET、POST外的所有请求
	_response.result(http::status::method_not_allowed);
	_response.set(http::field::content_type, "text/plain");
	_response.set(http::field::allow, "GET, POST, OPTIONS"); // 指明允许的请求方法
	beast::ostream(_response.body()) << "Method not allowed\r\n";
	WriteResponse();
	return;

}
