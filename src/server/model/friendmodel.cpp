#include "friendmodel.hpp"
#include "db.h"
// 添加好友
void FriendModel::insert(int uid, int fid)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into Friend values('%d', '%d')", uid, fid);
    MySql mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}
// 通过用户id查询所有的好友
vector<User> FriendModel::query(int uid)
{
    char sql[1024];
    sprintf(sql, "select U.id, U.name, U.state from User U inner join Friend F on F.friendid=U.id where F.userid = %d", uid);
    vector<User> vec;
    MySql mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.emplace_back(user);
            }
            mysql_free_result(res);
        }
    }
    return vec;
}