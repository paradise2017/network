#pragma once
#include <iostream>
#include <boost/asio.hpp>
using boost::asio::ip::tcp;
using namespace std;

class Session
{
public:
    Session(boost::asio::io_context &ioc);
    tcp::socket &Socket();
    void Start();

private:
    void handle_read(const boost::system::error_code &error, std::size_t bytes_transferred);
    void handle_write(const boost::system::error_code &error);

private:
    tcp::socket socket_;
    enum
    {
        max_length = 1024
    };
    char data_[max_length];
};

class Server
{
public:
    Server(boost::asio::io_context &ioc, short port);

private:
    void start_accept();
    void handle_accept(Session *new_session, const boost::system::error_code &error);

private:
    // io_context 不允许构造
    boost::asio::io_context &ioc_;
    tcp::acceptor acceptor_;
};