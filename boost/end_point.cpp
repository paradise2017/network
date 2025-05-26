#include "end_point.hpp"
#include <iostream>
#include <boost/asio.hpp>
using namespace boost;
int clientEndPoint()
{
    std::string raw_ip_address = "127.4.7.1";
    unsigned short port = 3333;

    boost::system::error_code ec;
    asio::ip::address ip_address = asio::ip::make_address(raw_ip_address, ec);
    if (ec.value() != 0)
    {
        std::cout << "Fail to parse the Ip address" << ec.value() << ".Message is " << ec.message() << std::endl;
        return ec.value();
    }
    asio::ip::tcp::endpoint end_point(ip_address, port);
    return 0;
}

int serverEndPoint()
{
    unsigned short port = 3333;

    // 任意地址通信
    asio::ip::address ip_address = asio::ip::address_v4::any();

    // 服务器端口
    asio::ip::tcp::endpoint end_point(ip_address, port);
    return 0;
}

int createTcpSocket()
{
    // socket 通信需要上下文， 上下文是boost asio 的核心服务。
    return 0;
}
