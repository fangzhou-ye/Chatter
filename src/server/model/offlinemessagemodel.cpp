#include "offlinemessagemodel.hpp"
#include "db.h"

void OfflineMsgModel::insert(int uid, string msg)
{
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into OfflineMessage values('%d', '%s')", uid, msg.c_str());
    MySql mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

void OfflineMsgModel::remove(int uid)
{
    // 组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "delete from OfflineMessage where userid=%d", uid);
    MySql mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

vector<string> OfflineMsgModel::query(int uid)
{
    char sql[1024];
    sprintf(sql, "select message from OfflineMessage where userid=%d", uid);
    vector<string> vec;
    MySql mysql;
    if(mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res != nullptr){ // 循环所有的离线消息
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr){
                vec.emplace_back(row[0]);
            }
            mysql_free_result(res);
        }
    }
    return vec;
}