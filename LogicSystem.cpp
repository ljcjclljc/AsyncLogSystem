#include "LogicSystem.h"
#include "HttpConnection.h"
#include "Service.h"
bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> con)
{
	if (_get_handlers.find(path) == _get_handlers.end())
	{
		return false;
	}

	_get_handlers[path](con);
	return true;
}

bool LogicSystem::HandlePost(std::string path, std::shared_ptr<HttpConnection> con)
{
	if (_post_handlers.find(path) == _post_handlers.end())
	{
		return false;
	}

	_post_handlers[path](con);
	return true;
}


void LogicSystem::RegGet(std::string url, HttpHandler handler)
{
	_get_handlers.insert(make_pair(url,handler));
}

void LogicSystem::RegPost(std::string url, HttpHandler handler)
{
	_post_handlers.insert(make_pair(url, handler));
}

LogicSystem::LogicSystem(){
    RegGet("/get_test", [this](std::shared_ptr<HttpConnection> connection) {
	beast::ostream(connection->_response.body()) << "receive get_test req ";
	int i = 0;
	//beast::ostream(connection->_response.body()) << i ;
	for (auto elem : connection->_get_params) {
		i++;
		beast::ostream(connection->_response.body()) << "pram " << i << "key is " << elem.first;
		beast::ostream(connection->_response.body()) << "pram " << i << "value is " << elem.second << std::endl;
	}
	});

    RegGet("/download/", [this](std::shared_ptr<HttpConnection> connection) {
        _service->Download(connection);
	});

    RegPost("/upload", [this](std::shared_ptr<HttpConnection> connection) {
        std::string path = std::string(connection->_request.target());
        path = storage::UrlDecode(path);
        _service->Upload(connection);
    });

    RegGet("/", [this](std::shared_ptr<HttpConnection> connection) {
        _service->ListShow(connection);
    });
}