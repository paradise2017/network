#include <boost/asio.hpp>
#include <iostream>
#include <cstring> // 添加此头文件以使用 strlen
using namespace boost::asio::ip;
using namespace std;

const int MAX_LENGTH = 1024;

int main()
{
    std::cout << "client is running" << std::endl;

    try
    {
        // 创建上下文服务
        boost::asio::io_context io_context;

        // 构造endpoint
        tcp::endpoint remote_ep(make_address("127.0.0.1"), 10086);

        // 创建socket
        tcp::socket socket(io_context);

        // 尝试连接
        boost::system::error_code error;
        socket.connect(remote_ep, error);

        if (error)
        {
            cout << "Failed to connect: " << error.message() << endl;
            return 1; // 退出程序，表示连接失败
        }

        // 输入消息
        std::cout << "Enter Message: ";
        char request[MAX_LENGTH];
        std::cin.getline(request, MAX_LENGTH);

        // 计算消息长度
        size_t request_length = std::strlen(request);

        // 发送消息
        boost::system::error_code send_error;
        socket.write_some(boost::asio::buffer(request, request_length), send_error);

        if (send_error)
        {
            cout << "Failed to send message: " << send_error.message() << endl;
            return 1;
        }

        std::cout << "Message sent successfully!" << std::endl;
        system("pause");
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}