#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"

// User表的数据操作类
class UserModel{
public:
    // 增加方法
    bool insert(User &user);

    // 根据用户id查询用户信息(返回USER的ORM对象)
    User query(int uid);

    // 更新用户状态信息
    bool updateState(User user);

    // 重置用户state
    void resetState();
};

#endif