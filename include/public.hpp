#ifndef PUBLIC_H
#define PUBLIC_H

/**
 * server和client的公共文件
*/
enum EnMsgType{
    LOGIN_MSG = 1, // 登录消息 1
    LOGIN_MSG_ACK, // 登录响应消息 2
    LOGOUT_MSG, // 登出 3
    REG_MSG, // 注册消息 4
    REG_MSG_ACK, // 注册响应消息 5
    ONE_CHAT_MSG, // 聊天消息 6
    ADD_FRIEND_MSG, // 添加好友消息 7
    CREATE_GROUP_MSG, // 创建群组 8
    ADD_GROUP_MSG, // 加入群组 9
    GROUP_CHAT_MSG, // 群聊天 10
};

#endif