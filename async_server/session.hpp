#pragma once
#include <iostream>
#include "server.hpp"
#include <boost/asio.hpp>
#include <map>
#include <queue>
#include <mutex>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
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
};

class MsgNode
{
    friend class Session;
    // 友元类访问私有成员
public:
    MsgNode(char *msg, int max_len)
    {
        data_ = new char[max_len];
        memcpy(data_, msg, max_len);
    }
    ~MsgNode()
    {
        delete[] data_;
    }

private:
    int cur_len_;
    int max_len_;
    char *data_;
};