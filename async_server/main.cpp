#include "server.hpp"
#include "session.hpp"
using namespace std;

int main()
{
    try
    {
        boost::asio::io_context io_context;
        Server server(io_context, 10086);
        io_context.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}