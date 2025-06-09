#include "server.hpp"
#include "session.hpp"
Server::Server(boost::asio::io_context &ioc, short port)
    : ioc_(ioc),
      acceptor_(ioc_, tcp::endpoint(tcp::v4(), port))
{
    cout << "Server started on port " << port << endl;
    start_accept(); // Start accepting connections
}

void Server::ClearSession(std::string uuid)
{
    session_.erase(uuid);
}

void Server::start_accept()
{
    // 一个client 一个 session
    shared_ptr<Session> new_session = make_shared<Session>(ioc_, this);
    // Socket 类似于epoll中的传递消息的fd
    // 当有新的连接到来时，acceptor_分配socket收发数据
    // 底层等待客户端发送数据
    // 异步接收事件写到 io_context 事件队列
    acceptor_.async_accept(new_session->Socket(),
                           std::bind(&Server::handle_accept, this, new_session, placeholders::_1));
}

void Server::handle_accept(shared_ptr<Session> new_session, const boost::system::error_code &error)
{
    if (!error)
    {
        cout << "new session accepted" << endl;
        // socket接收
        new_session->Start();
        session_[new_session->GetUuid()] = new_session;
    }
    else
    {
        cout << "accept error: " << error.message() << endl;
    }
    // listen 接收
    start_accept();
}