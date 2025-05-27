#include <boost/asio.hpp>
#include <set>
#include <iostream>
#include <memory>
using namespace boost::asio::ip;
using namespace std;
const int MAX_LENGTH = 1024;
std::set<std::shared_ptr<std::thread>> threads_set;
void session(shared_ptr<tcp::socket> socket)
{
    try
    {
        while (true)
        {

            char data[MAX_LENGTH];
            memset(data, 0, MAX_LENGTH);
            boost::system::error_code error;
            // size_t length = boost::asio::read(socket, boost::asio::buffer(data, MAX_LENGTH), error);
            size_t length = socket->read_some(boost::asio::buffer(data, MAX_LENGTH), error);
            if (error == boost::asio::error::eof)
            {
                std::cout << "Connection closed by client." << std::endl;
                break; // 连接关闭
            }
            else if (error)
            {
                std::cerr << "Error reading from socket: " << error.value() << std::endl;
                break; // 发生错误
            }

            cout << "receive from " << socket->remote_endpoint().address().to_string() << ":"
                 << socket->remote_endpoint().port() << endl;
            cout << "Received: " << string(data, length) << endl;

            // 回传给对方
            boost::asio::write(*socket, boost::asio::buffer(data, length));
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception in thread" << e.what() << '\n';
    }
}

void server(boost::asio::io_context &io_context, unsigned short port)
{
    // 类似于监听listen 的epoll
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
    while (true)
    {
        shared_ptr<tcp::socket> socket = make_shared<tcp::socket>(io_context);
        acceptor.accept(*socket);
        // 线程用于接受任务
        auto t = std::make_shared<std::thread>(session, socket);
        // 保证t线程局部变量不被回收
        threads_set.insert(t);
    }
}

int main()
{

    std::cout << "Server is running..." << std::endl;
    try
    {
        // 创建上下文服务
        boost::asio::io_context io_context;

        server(io_context, 10086);

        for (auto &t : threads_set)
        {
            t->join(); // 等待所有线程结束
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}