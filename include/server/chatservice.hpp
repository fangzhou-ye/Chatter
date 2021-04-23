#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>
#include "json.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"

using namespace std;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

// 表示处理消息的回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &json, Timestamp)>;

// 聊天服务器业务类（单例模式)
class ChatService
{
public:
    // 获取单例对象的接口函数
    static ChatService *instance();
    // 登录业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 注册业务
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 一对一聊天
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 添加好友
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 创建群组业务
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 加入群组
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 群聊
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 注销
    void logout(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 获取msgid对应的handler
    MsgHandler getHandler(int msgid);
    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
    // 服务器异常，业务重置
    void reset();
    // 从redis消息队列中获取订阅的消息
    void handleRedisSubscribeMessage(int, string);

private:
    ChatService();
    
    // msgid => 业务处理方法 实现网络模块和业务模块的解耦
    unordered_map<int, MsgHandler> _msgHandlerMap;
    // 存储在线用户的通信连接 uid => conn (由多线程调用需要保证线程安全)
    unordered_map<int, TcpConnectionPtr> _userConnMap;
    // 互斥锁，保证_userConnMap多线程安全问题
    mutex _connMutex;

    // 数据操作类对象
    UserModel _userModel;
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;

    // Redis操作对象
    Redis _redis;
};

#endif