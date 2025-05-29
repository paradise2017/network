#include "session.hpp"

Session::Session(std::shared_ptr<asio::ip::tcp::socket> socket)
    : send_pending_(false),
      recv_pending_(false)
{
}

void Session::Connect(const asio::ip::tcp::endpoint &ep)
{
}

void Session::WriteToSocket(const std::string &buf)
{
    // 插入发送队列
    send_queue_.emplace(new MsgNode(buf.c_str(), buf.length()));
    // pending状态说明上一次有未发送完的数据
    if (send_pending_)
    {
        return;
    }
    // 异步发送数据，因为异步所以不会一下发送完
    this->socket_->async_write_some(asio::buffer(buf), std::bind(&Session::WriteCallBack, this, std::placeholders::_1, std::placeholders::_2));
    send_pending_ = true;
}

void Session::WriteAllToSocket(const std::string &buf)
{
    // 插入发送队列
    send_queue_.emplace(new MsgNode(buf.c_str(), buf.length()));
    // pending状态说明上一次有未发送完的数据
    if (send_pending_)
    {
        return;
    }
    // 异步发送数据，因为异步所以不会一下发送完
    // async_send内部多次调用 async_write_some，直到所有数据发送完毕
    this->socket_->async_send(asio::buffer(buf),
                              std::bind(&Session::WriteAllCallBack, this,
                                        std::placeholders::_1, std::placeholders::_2));
    send_pending_ = true;
}

void Session::WriteAllCallBack(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
    if (ec.value() != 0)
    {
        std::cout << "Error occured! Error code = "
                  << ec.value()
                  << ". Message: " << ec.message();
        return;
    }
    // 如果发送完，则pop出队首元素
    send_queue_.pop();
    // 如果队列为空，则说明所有数据都发送完,将pending设置为false
    if (send_queue_.empty())
    {
        send_pending_ = false;
    }
    // 如果队列不是空，则继续将队首元素发送
    if (!send_queue_.empty())
    {
        auto &send_data = send_queue_.front();
        // 第一次发送 异步发送一定要地址偏移
        this->socket_->async_send(asio::buffer(send_data->msg_ + send_data->cur_len_, send_data->total_len_ - send_data->cur_len_),
                                  std::bind(&Session::WriteAllCallBack,
                                            this, std::placeholders::_1, std::placeholders::_2));
    }
}

void Session::ReadFromSocket()
{
    if (recv_pending_)
    {
        return;
    }
    // 可以调用构造函数直接构造，但不可用已经构造好的智能指针赋值
    /*auto _recv_nodez = std::make_unique<MsgNode>(RECVSIZE);
    _recv_node = _recv_nodez;*/
    recv_node_ = std::make_shared<MsgNode>(RECV_SIZE);
    socket_->async_read_some(asio::buffer(recv_node_->msg_, recv_node_->total_len_), std::bind(&Session::ReadCallBack, this,
                                                                                               std::placeholders::_1, std::placeholders::_2));
    recv_pending_ = true;
}

void Session::ReadCallBack(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
    recv_node_->cur_len_ += bytes_transferred;
    // 没读完继续读
    if (recv_node_->cur_len_ < recv_node_->total_len_)
    {
        socket_->async_read_some(asio::buffer(recv_node_->msg_ + recv_node_->cur_len_,
                                              recv_node_->total_len_ - recv_node_->cur_len_),
                                 std::bind(&Session::ReadCallBack, this,
                                           std::placeholders::_1, std::placeholders::_2));
        return;
    }
    // 将数据投递到队列里交给逻辑线程处理，此处略去
    // 如果读完了则将标记置为false
    recv_pending_ = false;
    // 指针置空
    recv_node_ = nullptr;
}

void Session::ReadAllFromSocket(const std::string &buf)
{
    if (recv_pending_)
    {
        return;
    }
    // 可以调用构造函数直接构造，但不可用已经构造好的智能指针赋值
    /*auto _recv_nodez = std::make_unique<MsgNode>(RECVSIZE);
    _recv_node = _recv_nodez;*/
    recv_node_ = std::make_shared<MsgNode>(RECV_SIZE);
    socket_->async_receive(asio::buffer(recv_node_->msg_, recv_node_->total_len_), std::bind(&Session::ReadAllCallBack, this,
                                                                                             std::placeholders::_1, std::placeholders::_2));
    recv_pending_ = true;
}
void Session::ReadAllCallBack(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
    recv_node_->cur_len_ += bytes_transferred;
    // 将数据投递到队列里交给逻辑线程处理，此处略去
    // 如果读完了则将标记置为false
    recv_pending_ = false;
    // 指针置空
    recv_node_ = nullptr;
}

// 通过队列保证异步发送的持久性
void Session::WriteCallBack(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
    // 异步写入错误
    if (ec.value() != 0)
    {
        std::cout << "Error , code is " << ec.value() << " . Message is " << ec.message();
        return;
    }

    // 取出队首元素即当前未发送完数据
    auto &send_data = send_queue_.front();
    send_data->cur_len_ += bytes_transferred;

    // 数据未发送完， 则继续发送
    if (send_data->cur_len_ < send_data->total_len_)
    {
        this->socket_->async_write_some(asio::buffer(send_data->msg_ + send_data->cur_len_, send_data->total_len_ - send_data->cur_len_), std::bind(&Session::WriteCallBack, this, std::placeholders::_1, std::placeholders::_2));
        return;
    }

    // 如果发送完，则pop出队首元素
    send_queue_.pop();

    // 如果队列为空，则说明所有数据都发送完,将pending设置为false
    if (send_queue_.empty())
    {
        send_pending_ = false;
    }

    // 如果队列不是空，则继续将队首元素发送
    if (!send_queue_.empty())
    {
        auto &send_data = send_queue_.front();
        this->socket_->async_write_some(asio::buffer(send_data->msg_ + send_data->cur_len_, send_data->total_len_ - send_data->cur_len_), std::bind(&Session::WriteCallBack, this, std::placeholders::_1, std::placeholders::_2));
    }
}

/*
void Session::WriteCallBackErr(const boost::system::error_code &ec, std::size_t bytes_transferred, std::shared_ptr<MsgNode> msg_node_)
{
    // 当前发送的长度+上次发送的长度 == 总长度
    if (bytes_transferred + msg_node_->cur_len_ < msg_node_->total_len_)
    {
        // 更新当前发送的长度
        msg_node_->cur_len_ += bytes_transferred;

        // 继续写入剩余数据
        socket_->async_write_some(
            asio::buffer(msg_node_->msg_ + msg_node_->cur_len_, msg_node_->total_len_ - msg_node_->cur_len_),
            std::bind(&Session::WriteCallBackErr, this, std::placeholders::_1, std::placeholders::_2, msg_node_));
    }
    else if (bytes_transferred == 0)
    {
        // 连接关闭
        std::cout << "Connection closed by peer." << std::endl;
    }
}

void Session::WriteToSocketErr(const std::string &buf)
{
    send_node_ = std::make_shared<MsgNode>(buf.c_str(), buf.size());
    // 1：异步写的数据
    // 2：异步写的回调函数
    // 3：写buffer的数据，写完之后调用回调
    // 4：写完之后会自动填充  const boost::system::error_code &ec, std::size_t bytes_transferred,  这两个字段
    //*
      //  BOOST_ASIO_COMPLETION_TOKEN_FOR(void(boost::system::error_code,
                                             std::size_t))
     //   WriteToken
      //
    socket_->async_write_some(
    asio::buffer(send_node_->msg_, send_node_->total_len_),
    std::bind(&Session::WriteCallBackErr, this, std::placeholders::_1, std::placeholders::_2, send_node_));
}
*/