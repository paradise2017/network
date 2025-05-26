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
    asio::io_context io_context;

    // 创建ipv4协议
    asio::ip::tcp protocol(asio::ip::tcp::v4());

    // 生成socket
    asio::ip::tcp::socket socket(io_context);

    boost::system::error_code ec;
    socket.open(protocol, ec);
    if (ec.value() != 0)
    {
        std::cout << "Fail to open the socket" << ec.value() << ".Message is " << ec.message() << std::endl;
        return ec.value();
    }
    return 0;
}

int createAcceptorSocket()
{
    // 新的写法
    asio::io_context io_context;

    asio::ip::tcp::acceptor acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 3333));

    /* 旧写法
    asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor(io_context);

    asio::ip::tcp protocol = asio::ip::tcp::v4();
    boost::system::error_code ec;
    acceptor.open(protocol, ec);

    if (ec.value() != 0)
    {
        std::cout << "Fail to open the acceptor" << ec.value() << ".Message is " << ec.message() << std::endl;
        return ec.value();
    }

    return 0;
     */
}

int bindAcceptorSocket()
{
    unsigned short port = 3333;
    asio::ip::tcp::endpoint end_point(asio::ip::address_v4::any(), port);
    asio::io_context io_context;

    asio::ip::tcp::acceptor acceptor(io_context, end_point.protocol());
    boost::system::error_code ec;
    acceptor.bind(end_point, ec);

    if (ec.value() != 0)
    {
        std::cout << "Fail to open the acceptor" << ec.value() << ".Message is " << ec.message() << std::endl;
        return ec.value();
    }

    return 0;
}
// 客户端连接服务器
int connectToEnd()
{
    std::string raw_ip_address = "192.168.1.124";
    unsigned short port = 3333;
    try
    {
        asio::io_context io_context;

        asio::ip::tcp::endpoint end_point(asio::ip::make_address(raw_ip_address), port);
        // 创建socket时指定协议类型
        // 默认ipv4协议
        asio::ip::tcp::socket socket(io_context, end_point.protocol());

        boost::system::error_code ec;
        socket.connect(end_point, ec);

        if (ec.value() != 0)
        {
            std::cout << "Fail to connect to the endpoint" << ec.value() << ".Message is " << ec.message() << std::endl;
            return ec.value();
        }
    }
    catch (system::system_error &e)
    {
        std::cerr << "System error: " << e.what() << '\n';
        return e.code().value();
    }

    return 0;
}

// 这种方式比较少
int dnsConnectToEnd()
{
    std::string host = "test.cyn";
    std::string port = "3333";
    asio::io_context io_context;

    // 域名解析
    asio::ip::tcp::resolver resolver(io_context);
    boost::system::error_code ec;
    auto results = resolver.resolve(host, port, ec);
    if (ec)
    {
        std::cout << "Failed to resolve host: " << ec.message() << std::endl;
        return ec.value();
    }

    try
    {
        asio::ip::tcp::socket socket(io_context);
        asio::connect(socket, results, ec);
    }
    catch (system::system_error &e)
    {
        std::cout << "System error: " << e.what() << '\n';
    }

    return 0;
}

int acceptNewConnection()
{
    // 队列中包含待处理连接请求的大小。 就是来不及处理的连接
    const int BACKLOG_SIZE = 30;
    // 第一步：这里我们假设服务器应用程序已经获得了协议端口号。
    unsigned short port_num = 3333;
    // 第二步：创建一个服务器端点。
    asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(),
                               port_num);
    asio::io_context ios;
    try
    {
        // 第三步：实例化并打开一个接收器套接字。
        asio::ip::tcp::acceptor acceptor(ios, ep.protocol());
        // 第四步：将接收器套接字绑定到服务器端点。
        acceptor.bind(ep);
        // 第五步：开始监听传入的连接请求。
        acceptor.listen(BACKLOG_SIZE);
        // 第六步：创建一个活动套接字。
        asio::ip::tcp::socket sock(ios);
        // 第七步：处理下一个连接请求，并将活动套接字连接到客户端。
        acceptor.accept(sock);
        // 此时，'sock' 套接字已连接到客户端应用程序，
        // 并可用于向其发送数据或从其接收数据。
    }
    catch (system::system_error &e)
    {
        std::cout << "发生错误！错误代码 = " << e.code()
                  << "。消息: " << e.what();
        return e.code().value();
    }
    return 0;
}
