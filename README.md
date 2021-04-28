# Chatter
这是一个工作在nginx tcp负载均衡环境下的集群聊天服务器，基于[muduo多线程网络库](https://github.com/chenshuo/muduo)实现
实现了用户注册，登录，一对一聊天，群组聊天的常用功能。

该项目主要由**网络模块**，**业务逻辑处理模块**和**数据模块**构成。

### 项目的依赖
* [muduo](https://github.com/chenshuo/muduo) 多线程网络库
* [nginx](https://www.nginx.com/) 中的tcp负载均衡模块
* [hiredis](https://github.com/redis/hiredis) Redis对C++语言的支持
* [JSON for Modern C++](https://github.com/nlohmann/json) 第三方JSON库

# 使用服务器集群提高并发
使用nginx tcp负载均衡模块配置服务器集群，提高并发。客户端连接负载均衡器，由负载均衡器 **选择(轮询方式)** 具体的一台服务器与客户端连接。使用基于**发布-订阅**的消息队列(Redis)实现跨服务器通信。

![alt text](https://github.com/fangzhou-ye/Chatter/blob/master/architecture.png)
# 数据库设计
### User
<table>
  <tr>
    <th>属性</th>
    <th>类型</th>
    <th>约束</th>
  </tr>
  <tr>
    <td>id</td>
    <td>INT</td>
    <td>PRIMARY KEY, AUTO_INCREMENT</td>
  </tr>
  <tr>
    <td>name</td>
    <td>VARCHAR(50)</td>
    <td>NOT NULL, UNIQUE</td>
  </tr>
  <tr>
    <td>password</td>
    <td>VARCHAR(50)</td>
    <td>NOT NULL</td>
  </tr>
  <tr>
    <td>state</td>
    <td>ENUM('online', 'offline')</td>
    <td>DEFAULT 'offline'</td>
  </tr>
</table>

### Friend
<table>
  <tr>
    <th>属性</th>
    <th>类型</th>
    <th>约束</th>
  </tr>
  <tr>
    <td>userid</td>
    <td>INT</td>
    <td>NOT NULL, COMPOSITE KEY</td>
  </tr>
  <tr>
    <td>friendid</td>
    <td>INT</td>
    <td>NOT NULL, COMPOSITE KEY</td>
  </tr>
</table>

### AllGroup
<table>
  <tr>
    <th>属性</th>
    <th>类型</th>
    <th>约束</th>
  </tr>
  <tr>
    <td>id</td>
    <td>INT</td>
    <td>PRIMARY KEY, AUTO_INCREMENT</td>
  </tr>
  <tr>
    <td>groupname</td>
    <td>VARCHAR(50)</td>
    <td>NOT NULL, UNIQUE</td>
  </tr>
  <tr>
    <td>groupdesc</td>
    <td>VARCHAR(200)</td>
    <td>DEFAULT ''</td>
  </tr>
</table>

### GroupUser
<table>
  <tr>
    <th>属性</th>
    <th>类型</th>
    <th>约束</th>
  </tr>
  <tr>
    <td>groupid</td>
    <td>INT</td>
    <td>NOT NULL, COMPOSITE_KEY</td>
  </tr>
  <tr>
    <td>userid</td>
    <td>INT</td>
    <td>NOT NULL, COMPOSITE_KEY</td>
  </tr>
  <tr>
    <td>grouprole</td>
    <td>ENUM('creator', 'normal')</td>
    <td>DEFAULT 'normal'</td>
  </tr>
</table>

### OfflineMessage
<table>
  <tr>
    <th>属性</th>
    <th>类型</th>
    <th>约束</th>
  </tr>
  <tr>
    <td>userid</td>
    <td>INT</td>
    <td>NOT NULL</td>
  </tr>
  <tr>
    <td>message</td>
    <td>VARCHAR(500)</td>
    <td>NOT NULL</td>
  </tr>
</table>
