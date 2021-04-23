#include "usermodel.hpp"
#include "db.h"
// #include <iostream>

using namespace std;

bool UserModel::insert(User &user)
{
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into User(name, password, state) values('%s', '%s', '%s')",
            user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());
    MySql mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 获取插入成功的用户数据生成的主键id
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

User UserModel::query(int uid)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from User where id = %d", uid);
    MySql mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res); // 释放资源
                return user;
            }
        }
    }
    return User(); // 默认空user id = -1
}

bool UserModel::updateState(User user)
{
    char sql[1024] = {0};
    sprintf(sql, "update User set state = '%s' where id = %d",
            user.getState().c_str(), user.getId());
    MySql mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

void UserModel::resetState()
{
    char sql[1024] = "update User set state = 'offline' where state = 'online'";
    MySql mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}