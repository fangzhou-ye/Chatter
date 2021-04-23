#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H

#include "user.hpp"
#include <vector>
using namespace std;

// Friend表接口操作方法
class FriendModel
{
public:
    // 添加好友
    void insert(int uid, int fid);
    // 返回用户好友列表
    vector<User> query(int uid);
};

#endif