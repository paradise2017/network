#include "server.hpp"
#include "session.hpp"
using namespace std;

int main()
{
    try
    {
        boost::asio::io_context io_context;
        // 往 io_context 注册消息，引用传递。
        Server server(io_context, 10086);
        // asio进行消息循环，事件队列循环。
        io_context.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}