#include "session.hpp"
Session::Session(boost::asio::io_context &ioc)
    : socket_(ioc)
{
}

tcp::socket &Session::Socket()
{
    return socket_;
}

void Session::Start()
{
    memset(data_, 0, max_length);
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
                            std::bind(&Session::handle_read, this, placeholders::_1,
                                      placeholders::_2));
}

void Session::handle_read(const boost::system::error_code &error, std::size_t bytes_transferred)
{
    if (!error)
    {
        cout << "server receive data is " << data_ << endl;
        boost::asio::async_write(socket_, boost::asio::buffer(data_, bytes_transferred),
                                 std::bind(&Session::handle_write, this, placeholders::_1));
    }
    else
    {
        std::cout << "read error" << std::endl;
        // If an error occurs, we delete the session object.
        delete this;
    }
}

void Session::handle_write(const boost::system::error_code &error)
{
    if (!error)
    {
        memset(data_, 0, max_length);
        socket_.async_read_some(boost::asio::buffer(data_, max_length), std::bind(&Session::handle_read, this, placeholders::_1, placeholders::_2));
    }
    else
    {
        std::cout << "write error" << std::endl;
        delete this;
    }
}

Server::Server(boost::asio::io_context &ioc, short port)
    : ioc_(ioc),
      acceptor_(ioc, tcp::endpoint(tcp::v4(), port))
{
    cout << "Server started on port " << port << endl;
    start_accept(); // Start accepting connections
}

void Server::start_accept()
{
    // shared_ptr<Session> new_session = make_shared<Session>(ioc_);
    Session *new_session = new Session(ioc_); // Create a new session object
    // Asynchronously accept a new connection
    acceptor_.async_accept(new_session->Socket(),
                           std::bind(&Server::handle_accept, this, new_session, placeholders::_1));
}

void Server::handle_accept(Session *new_session, const boost::system::error_code &error)
{
    if (!error)
    {
        cout << "new session accepted" << endl;
        new_session->Start();
        start_accept(); // Start accepting the next connection
    }
    else
    {
        cout << "accept error: " << error.message() << endl;
        delete new_session; // Clean up the session if there was an error
    }
}
