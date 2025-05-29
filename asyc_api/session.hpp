#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <queue>
#include <iostream>
using namespace boost;
using namespace std;
// 最大报文接收大小
const int RECV_SIZE = 1024;
class MsgNode
{
public:
    MsgNode(const char *msg, int total_len) : total_len_(total_len), cur_len_(0)
    {
        msg_ = new char[total_len];
        memcpy(msg_, msg, total_len);
    }
    MsgNode(int total_len) : total_len_(total_len), cur_len_(0)
    {
        msg_ = new char[total_len];
    }
    ~MsgNode()
    {
        delete[] msg_;
    }
    // 消息首地址
    char *msg_;
    // 总长度
    int total_len_;
    // 当前长度
    int cur_len_;
};

class Session
{
public:
    Session(std::shared_ptr<asio::ip::tcp::socket> socket);
    void Connect(const asio::ip::tcp::endpoint &ep);

    /*
    // 写回调
    void WriteCallBackErr(const boost::system::error_code &ec, std::size_t bytes_transferred, std::shared_ptr<MsgNode> msg_node_);
    // 封装的函数
    void WriteToSocketErr(const std::string &buf);
    */

    // 写回调
    void WriteCallBack(const boost::system::error_code &ec, std::size_t bytes_transferred);
    // 封装的函数
    void WriteToSocket(const std::string &buf);
    // 异步写入所有数据到socket
    void WriteAllToSocket(const std::string &buf);
    void WriteAllCallBack(const boost::system::error_code &ec, std::size_t bytes_transferred);

    // 读取
    void ReadFromSocket();
    void ReadCallBack(const boost::system::error_code &ec, std::size_t bytes_transferred);

    // 读取所有数据到socket
    void Session::ReadAllFromSocket(const std::string &buf);
    void Session::ReadAllCallBack(const boost::system::error_code &ec, std::size_t bytes_transferred);

private:
    bool send_pending_;
    bool recv_pending_;
    std::queue<std::shared_ptr<MsgNode>> send_queue_;
    std::shared_ptr<MsgNode> recv_node_;
    std::shared_ptr<asio::ip::tcp::socket> socket_;
    std::shared_ptr<MsgNode> send_node_;
};
