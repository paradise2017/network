#pragma once
#include <iostream>
#include "session.hpp"
#include <boost/asio.hpp>
#include <map>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
using boost::asio::ip::tcp;
using namespace std;
class Session;
class Server
{
public:
    Server(boost::asio::io_context &ioc, short port);
    void ClearSession(std::string uuid);

private:
    void start_accept();
    void handle_accept(shared_ptr<Session> new_session, const boost::system::error_code &error);

private:
    // io_context 不允许构造
    boost::asio::io_context &ioc_;
    // 监听端口,类似于epoll中的 listenfd
    tcp::acceptor acceptor_;
    std::map<std::string, shared_ptr<Session>> session_;
};