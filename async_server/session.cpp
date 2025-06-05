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

void Session::Send(char *msg, int max_length)
{
    // true 表示发送队列有数据,false表示发送队列没有数据
    // 表示此处发送之前
    bool pending = false;
    std::lock_guard<std::mutex> lock(send_lock_);
    if (send_que_.size() > 0)
    {
        pending = true;
    }
    send_que_.push(make_shared<MsgNode>(msg, max_length));
    if (pending)
    {
        return;
    }
    boost::asio::async_write(socket_, boost::asio::buffer(msg, max_length),
                             std::bind(&Session::handle_write, this, std::placeholders::_1, shared_from_this()));
}
void Session::handle_read(const boost::system::error_code &error, std::size_t bytes_transferred, shared_ptr<Session> self)
{
    if (!error)
    {
        cout << "server receive data is " << data_ << endl;
        Send(data_, bytes_transferred);
        memset(data_, 0, max_length);
        // 异步读取数据，当数据读取完成后，会调用 handle_read 回调函数。
        // 回调函数中，我们再次调用 async_read_some 函数，继续读取数据，
        boost::asio::async_read(socket_, boost::asio::buffer(data_, max_length),
                                std::bind(&Session::handle_read, this, placeholders::_1, placeholders::_2, self));
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
        std::lock_guard<std::mutex> lock(send_lock_);
        // 写完之后的回调函数
        send_que_.pop();
        if (send_que_.size() > 0)
        {
            // 继续发送
            shared_ptr<MsgNode> msg = send_que_.front();
            boost::asio::async_write(socket_, boost::asio::buffer(msg->data_, msg->max_len_),
                                     std::bind(&Session::handle_write, this, placeholders::_1, self));
        }
    }
    else
    {
        std::cout << "write error" << std::endl;
        server_->ClearSession(uuid_);
    }
}
