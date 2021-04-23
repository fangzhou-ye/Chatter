#ifndef OFFLINEMESSAGEMODE_H
#define OFFLINEMESSAGEMODE_H

#include <string>
#include <vector>
using namespace std;

// 离线消息表的操作接口方法
class OfflineMsgModel
{
public:
    // 存储用户离线消息
    void insert(int uid, string msg);
    // 删除用户离线消息
    void remove(int uid);
    // 查询用户离线消息
    vector<string> query(int uid);
};

#endif