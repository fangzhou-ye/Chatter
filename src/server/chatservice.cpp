#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <vector>

using namespace muduo;
using namespace std;

// 获取单例对象
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 注册消息以及对应的Handler回调
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGOUT_MSG, std::bind(&ChatService::logout, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    // 群组相关
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

    // 连接redis服务器
    if (_redis.connect())
    {
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
}

void ChatService::reset()
{
    // 所有online用户state => offline
    _userModel.resetState();
}

MsgHandler ChatService::getHandler(int msgid)
{
    // 记录错误日志，msgid没有对应的事件处理回调
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        // 返回默认的处理器，空操作
        return [=](const TcpConnectionPtr &conn, json &json, Timestamp) {
            LOG_ERROR << "msgid:" << msgid << " no found handler!";
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}
// ORM 业务层操作的都是对象 DAO
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    string pwd = js["password"];

    User user = _userModel.query(id);
    if (user.getId() == id && user.getPwd() == pwd)
    {
        if (user.getState() == "online")
        { // 用户已登录不允许重复登录
            json response;
            response["msgid"] = user.getId();
            response["errno"] = 2;
            response["errmsg"] = "user already online";
            conn->send(response.dump());
        }
        else
        { // 记录用户连接信息(多线程调用)
            {
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
            }

            // id用户登录成功向redis订阅channel(id)
            _redis.subscribe(id);

            // 登陆成功，更新state (offline=>online)
            user.setState("online");
            _userModel.updateState(user);
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            // 查询该用户是否有离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec; // json库自动转化
                _offlineMsgModel.remove(id);  // 删除用户读取过得所有离线消息
            }
            // 查询用户的好友信息并返回
            vector<User> friendsVec = _friendModel.query(id);
            if (!friendsVec.empty())
            {
                vector<string> vec2;
                for (User &user : friendsVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.emplace_back(js.dump());
                }
                response["friends"] = vec2;
            }

            // 查询用户的群组信息
            vector<Group> groupuserVec = _groupModel.queryGroups(id);
            if (!groupuserVec.empty())
            {
                // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                vector<string> groupV;
                for (Group &group : groupuserVec)
                {
                    json grpjson;
                    grpjson["id"] = group.getId();
                    grpjson["groupname"] = group.getName();
                    grpjson["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for (GroupUser &user : group.getUsers())
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"] = userV;
                    groupV.push_back(grpjson.dump());
                }

                response["groups"] = groupV;
            }

            conn->send(response.dump());
        }
    }
    else
    {
        // 用户不存在或密码错误，登录失败
        json response;
        response["msgid"] = user.getId();
        response["errno"] = 1;
        response["errmsg"] = "id or password invalid!!!";
        conn->send(response.dump());
    }
}

// 处理注册业务 name pwd
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string name = js["name"];
    string pwd = js["password"];
    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user);
    if (state)
    {
        // 注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    }
    else
    {
        // 注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

void ChatService::logout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }

    // 用户注销下线，在redis取消订阅通道
    _redis.unsubscribe(userid);

    // 更新该用户state : online => offline
    User user(userid, "", "", "offline");
    _userModel.updateState(user);
}

void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    { // 保证对用户连接map访问的线程安全
        lock_guard<mutex> lock(_connMutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if (it->second == conn)
            {
                // 从用户连接map中删除
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

    // 用户注销下线，在redis取消订阅通道
    _redis.unsubscribe(user.getId());

    // 更新该用户state : online => offline
    if (user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
}

void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toId = js["toid"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toId);
        if (it != _userConnMap.end()) // 对方在线且在集群上相同的服务器
        {
            // 转发消息，服务器主动推送消息给to
            it->second->send(js.dump());
            return;
        }
    }

    // 查询toId是否在线（集群上的其他服务器）
    User user = _userModel.query(toId);
    if (user.getState() == "online")
    {
        _redis.publish(toId, js.dump());
        return;
    }

    // toId对方不在线，需要存储离线消息
    _offlineMsgModel.insert(toId, js.dump());
}

void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int uid = js["id"].get<int>();
    int fid = js["friendid"].get<int>();
    // 存储好友信息
    _friendModel.insert(uid, fid);
}

void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    // 存储新创建的群组信息
    Group group(-1, name, desc);
    if (_groupModel.createGroup(group))
    {
        // 创建群的人加入到GroupUser表
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
}

void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}

void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);

    lock_guard<mutex> lock(_connMutex);
    for (int id : useridVec)
    {
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end())
        {
            // 转发群消息
            it->second->send(js.dump());
        }
        else
        { // 查询是否在集群中其他服务器上
            User user = _userModel.query(id);
            if (user.getState() == "online")
            {
                _redis.publish(id, js.dump()); // 通道号id上发布消息
            }
            else
            {
                // 存储离线群消息
                _offlineMsgModel.insert(id, js.dump());
            }
        }
    }
}

// userid == 通道号channel
void ChatService::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }

    // 存储该用户的离线消息(又下线了)
    _offlineMsgModel.insert(userid, msg);
}