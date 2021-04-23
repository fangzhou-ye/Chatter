#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo;
using namespace muduo::net;

class ChatServer{
public:
    // 初始化
    ChatServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const string& nameArg);
    // 启动服务
    void start();

private:
    // 上报连接相关的回调函数
    void onConnection(const TcpConnectionPtr&);

    void onMessage(const TcpConnectionPtr&,
                            Buffer*,
                            Timestamp);

    TcpServer _server; // muduo服务器对象
    EventLoop *_loop; // 事件循环对象的指针
};

#endif