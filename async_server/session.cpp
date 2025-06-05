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

// bytes_transferred表示收到的字节数
void Session::handle_read(const boost::system::error_code &error, std::size_t bytes_transferred, shared_ptr<Session> self)
{

    if (!error)
    {
        // 已经移动的字符数（已处理的数据）
        int copy_len = 0;
        // bytes_transferred （未处理的数据）
        while (bytes_transferred > 0)
        {
            // 首次进入，头部未解析
            if (!b_head_parse_)
            {
                // 收到的数据是否满足消息长度要求？
                // 未处理的数据+头部当前数据（最开始是0） < 2
                // 就是当前的数据未接收完
                if (bytes_transferred + recv_head_node_->cur_len_ < HEAD_LENGTH)
                {
                    // 拷贝到当前节点
                    memcpy(recv_head_node_->data_ + recv_head_node_->cur_len_, data_ + copy_len, bytes_transferred);
                    recv_head_node_->cur_len_ += bytes_transferred;
                    ::memset(data_, 0, MAX_LENGTH);
                    socket_.async_read_some(boost::asio::buffer(data_, MAX_LENGTH),
                                            std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, self));
                    return;
                }
                // 收到的数据比头部多
                // 头部剩余未复制的长度
                int head_remain = HEAD_LENGTH - recv_head_node_->cur_len_;
                memcpy(recv_head_node_->data_ + recv_head_node_->cur_len_, data_ + copy_len, head_remain);
                // 更新已处理的data长度和剩余未处理的长度
                copy_len += head_remain;
                bytes_transferred -= head_remain;
                // 获取头部数据
                short data_len = 0;
                memcpy(&data_len, recv_head_node_->data_, HEAD_LENGTH);
                cout << "data_len is " << data_len << endl;
                // 头部长度非法
                if (data_len > MAX_LENGTH)
                {
                    std::cout << "invalid data length is " << data_len << endl;
                    server_->ClearSession(uuid_);
                    return;
                }
                recv_msg_node_ = make_shared<MsgNode>(data_len);
                // 消息的长度小于头部规定的长度，说明数据未收全，则先将部分消息放到接收节点里
                if (bytes_transferred < data_len)
                {
                    memcpy(recv_msg_node_->data_ + recv_msg_node_->cur_len_, data_ + copy_len, bytes_transferred);
                    recv_msg_node_->cur_len_ += bytes_transferred;
                    ::memset(data_, 0, MAX_LENGTH);
                    socket_.async_read_some(boost::asio::buffer(data_, MAX_LENGTH),
                                            std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, self));
                    // 头部处理完成
                    b_head_parse_ = true;
                    return;
                }
                memcpy(recv_msg_node_->data_ + recv_msg_node_->cur_len_, data_ + copy_len, data_len);
                recv_msg_node_->cur_len_ += data_len;
                copy_len += data_len;
                bytes_transferred -= data_len;
                recv_msg_node_->data_[recv_msg_node_->total_len_] = '\0';
                cout << "receive data is " << recv_msg_node_->data_ << endl;
                // 此处可以调用Send发送测试
                Send(recv_msg_node_->data_, recv_msg_node_->total_len_);
                // 继续轮询剩余未处理数据
                b_head_parse_ = false;
                recv_head_node_->Clear();
                if (bytes_transferred <= 0)
                {
                    ::memset(data_, 0, MAX_LENGTH);
                    socket_.async_read_some(boost::asio::buffer(data_, MAX_LENGTH),
                                            std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, self));
                    return;
                }
                continue;
            }
            // 已经处理完头部，处理上次未接受完的消息数据
            // 接收的数据仍不足剩余未处理的
            int remain_msg = recv_msg_node_->total_len_ - recv_msg_node_->cur_len_;
            if (bytes_transferred < remain_msg)
            {
                memcpy(recv_msg_node_->data_ + recv_msg_node_->cur_len_, data_ + copy_len, bytes_transferred);
                recv_msg_node_->cur_len_ += bytes_transferred;
                ::memset(data_, 0, max_length);
                socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                        std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, self));
                return;
            }
            memcpy(recv_msg_node_->data_ + recv_msg_node_->cur_len_, data_ + copy_len, remain_msg);
            recv_msg_node_->cur_len_ += remain_msg;
            bytes_transferred -= remain_msg;
            copy_len += remain_msg;
            recv_msg_node_->data_[recv_msg_node_->total_len_] = '\0';
            std::cout << "receive data is " << recv_msg_node_->data_ << endl;
            // 此处可以调用Send发送测试
            Send(recv_msg_node_->data_, recv_msg_node_->total_len_);
            // 继续轮询剩余未处理数据
            b_head_parse_ = false;
            recv_head_node_->Clear();
            if (bytes_transferred <= 0)
            {
                ::memset(data_, 0, max_length);
                socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                        std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, self));
                return;
            }
            continue;
        }
    }
    else
    {
        std::cout << "handle read failed, error is " << error.what() << endl;
        Close();
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
