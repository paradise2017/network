#pragma once
#include <iostream>
#include "server.hpp"
#include <boost/asio.hpp>
#include <map>
#include <queue>
#include <mutex>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#define MAX_LENGTH 1024 * 2
#define HEAD_LENGTH 2
using boost::asio::ip::tcp;
using namespace std;
class Server;
class MsgNode;
// enable_shared_from_this模版类，enable_shared_from_this<Session> 模版类生成的真实类
class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(boost::asio::io_context &ioc, Server *server);
    tcp::socket &Socket();
    void Start();
    std::string GetUuid();
    void Send(char *msg, int max_length);

private:
    void handle_read(const boost::system::error_code &error, std::size_t bytes_transferred, shared_ptr<Session> self);
    void handle_write(const boost::system::error_code &error, shared_ptr<Session> self);
    void Close();

private:
    // 与客户端收发的socket
    tcp::socket socket_;
    enum
    {
        max_length = 1024
    };
    char data_[max_length];
    Server *server_;
    std::string uuid_;
    std::queue<shared_ptr<MsgNode>> send_que_;
    std::mutex send_lock_;
    // 收到的消息结构
    std::shared_ptr<MsgNode> recv_msg_node_;
    bool b_head_parse_;
    // 收到的头部结构
    std::shared_ptr<MsgNode> recv_head_node_;
};

// tlv 协议
// 消息头（存储） 消息体
// example
// 5 "hello"
class MsgNode
{
    friend class Session;
    // 友元类访问私有成员
public:
    MsgNode(char *msg, short max_len) : total_len_(max_len + HEAD_LENGTH), cur_len_(0)
    {
        // 为什么 + 1 存储\0
        data_ = new char[total_len_ + 1]();
        // 存储消息体的长度
        memcpy(data_, &max_len, HEAD_LENGTH);
        // 数据+2 存储长度
        memcpy(data_ + HEAD_LENGTH, msg, max_len);
        data_[total_len_] = '\0';
    }

    MsgNode(short max_len) : total_len_(max_len), cur_len_(0)
    {
        data_ = new char[total_len_ + 1]();
    }

    void Clear()
    {
        memset(data_, 0, total_len_);
        cur_len_ = 0;
    }
    ~MsgNode()
    {
        delete[] data_;
    }

private:
    short cur_len_;
    short total_len_;
    char *data_;
};