# Chatter
This is a 
This is a cluster chat server working in an nginx tcp load balancing environment，based on [muduo](https://github.com/chenshuo/muduo)
implementing user register, login, one-to-one chat and chat in groups 

This project is composed of **network module**，**service module** and **data module**

### dependencies
* [muduo](https://github.com/chenshuo/muduo) multithreaded network library
* [nginx](https://www.nginx.com/) load balancer
* [hiredis](https://github.com/redis/hiredis) Redis for C++ support
* [JSON for Modern C++](https://github.com/nlohmann/json) JSON library

# Server cluster for high parallelism
using Nginx load balancer to configure the server cluster. The client only connect to the balancer and balancer select (round-robin) a server of the cluster to connect to. The communication between the clients in different servers is supported by the message queue of Redis.

![alt text](https://github.com/fangzhou-ye/Chatter/blob/master/architecture.png)
# Database design
### User
<table>
  <tr>
    <th>attribute</th>
    <th>type</th>
    <th>constraint</th>
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
    <th>attribute</th>
    <th>type</th>
    <th>constraint</th>
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
    <th>attribute</th>
    <th>type</th>
    <th>constraint</th>
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
    <th>attribute</th>
    <th>type</th>
    <th>constraint</th>
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
    <th>attribute</th>
    <th>type</th>
    <th>constraint</th>
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

```sql
CREATE TABLE User(
    id INT PRIMARY KEY AUTO_INCREMENT,
    name VARCHAR(50) NOT NULL UNIQUE,
    password VARCHAR(50) NOT NULL,
    state ENUM('online', 'offline') DEFAULT 'offline' 
);

CREATE TABLE Friend(
    userid INT NOT NULL,
    friendid INT NOT NULL,
    PRIMARY KEY(userid, friendid)
);

CREATE TABLE AllGroup(
    id INT PRIMARY KEY AUTO_INCREMENT,
    groupname VARCHAR(50) NOT NULL UNIQUE,
    groupdesc VARCHAR(200) DEFAULT ''
);

CREATE TABLE GroupUser(
    groupid INT NOT NULL,
    userid INT NOT NULL,
    grouprole ENUM('creator', 'normal') DEFAULT 'normal',
    PRIMARY KEY(groupid, userid)
);

CREATE TABLE OfflineMessage(
    userid INT NOT NULL,
    message VARCHAR(500) NOT NULL
);
```
