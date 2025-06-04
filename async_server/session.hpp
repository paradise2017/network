#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <map>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
using boost::asio::ip::tcp;
using namespace std;
class Server;

// enable_shared_from_this模版类，enable_shared_from_this<Session> 模版类生成的真实类
class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(boost::asio::io_context &ioc, Server *server);
    tcp::socket &Socket();
    void Start();
    std::string GetUuid();

private:
    void handle_read(const boost::system::error_code &error, std::size_t bytes_transferred, shared_ptr<Session> self);
    void handle_write(const boost::system::error_code &error, shared_ptr<Session> self);

private:
    tcp::socket socket_;
    enum
    {
        max_length = 1024
    };
    char data_[max_length];
    Server *server_;
    std::string uuid_;
};

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
    tcp::acceptor acceptor_;
    std::map<std::string, shared_ptr<Session>> session_;
};
