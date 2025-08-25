#include "CServer.h"
#include "HttpConnection.h"
#include "AsioIOServicePool.h"
CServer::CServer(boost::asio::io_context& ioc, unsigned short& port):
	_ioc(ioc),_acceptor(ioc,tcp::endpoint(tcp::v4(),port)),_socket(ioc)
{
}

void CServer::start()
{
	auto self = shared_from_this();
	 std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(_ioc);
		_acceptor.async_accept(new_con->get_socket(), [self,new_con](beast::error_code ec) {

		try {
			//std::cerr << "accept: " <<8080<< std::endl;
			if (ec) {
				std::cerr << "accept: " <<8080<<" error: " << ec.message() << std::endl;
				self->start();
				return;
			}

			new_con->Start();
			self->start();
		}
		catch (std::exception& exp) {

		}
		});
}
