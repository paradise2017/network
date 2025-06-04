#include "session.hpp"
Session::Session(boost::asio::io_context &ioc, Server *server)
    : socket_(ioc)
{
    // 借助 Boost 库生成随机 UUID，并将其转换为字符串形式，存储在 Session 对象里，
    // 方便后续对不同的会话进行唯一标识和管理。
    boost::uuids::uuid temp_uuid = boost::uuids::random_generator()();
    uuid_ = boost::uuids::to_string(temp_uuid);
}

tcp::socket &Session::Socket()
{
    return socket_;
}

void Session::Start()
{
    memset(data_, 0, max_length);

    // shared_ptr<session>(this) 普通的智能指针，是一个局部对象 传递过去就是 shared_ptr<Session> self(this)
    // 如何通过this 再生成一个智能指针，实现引用计数同步？
    // shared_from_this() 返回指向自己的一个智能指针，实现引用计数同步

    socket_.async_read_some(boost::asio::buffer(data_, max_length),
                            std::bind(&Session::handle_read, this, placeholders::_1,
                                      placeholders::_2, shared_from_this()));
}

std::string Session::GetUuid()
{
    return uuid_;
}

void Session::handle_read(const boost::system::error_code &error, std::size_t bytes_transferred, shared_ptr<Session> self)
{
    if (!error)
    {
        cout << "server receive data is " << data_ << endl;
        boost::asio::async_write(socket_, boost::asio::buffer(data_, bytes_transferred),
                                 std::bind(&Session::handle_write, this, placeholders::_1, self));
    }
    else
    {
        std::cout << "read error" << std::endl;
        // If an error occurs, we delete the session object.
        server_->ClearSession(uuid_);
    }
}

void Session::handle_write(const boost::system::error_code &error, shared_ptr<Session> self)
{
    if (!error)
    {
        memset(data_, 0, max_length);
        socket_.async_read_some(boost::asio::buffer(data_, max_length), std::bind(&Session::handle_read, this, placeholders::_1, placeholders::_2, self));
    }
    else
    {
        std::cout << "write error" << std::endl;
        server_->ClearSession(uuid_);
    }
}

Server::Server(boost::asio::io_context &ioc, short port)
    : ioc_(ioc),
      acceptor_(ioc, tcp::endpoint(tcp::v4(), port))
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
    shared_ptr<Session> new_session = make_shared<Session>(ioc_, this);
    acceptor_.async_accept(new_session->Socket(),
                           std::bind(&Server::handle_accept, this, new_session, placeholders::_1));
}

void Server::handle_accept(shared_ptr<Session> new_session, const boost::system::error_code &error)
{
    if (!error)
    {
        cout << "new session accepted" << endl;
        new_session->Start();
        session_[new_session->GetUuid()] = new_session;
    }
    else
    {
        cout << "accept error: " << error.message() << endl;
    }
    start_accept();
}