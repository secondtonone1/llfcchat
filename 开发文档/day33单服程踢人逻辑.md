---
title: 单服务器踢人逻辑实现
date: 2025-04-12 11:37:54
tags: [C++聊天项目]
categories: [C++聊天项目]

---

## 1. 为什么要有踢人逻辑

在服务器中经常会设计的同账户异地登陆时，将旧有账号的连接断开，必要时先发送下线消息通知旧账号的客户端，然后关闭这个连接。

服务器设计中尽量不要采用服务器主动关闭连接，那样会造成大量TIME_WAIT问题，这个之后再说。

先用一个图示说明踢人逻辑

旧客户端登录

![image-20250412120427206](https://cdn.llfc.club/img/image-20250412120427206.png)

当有新客户端连接时

![image-20250412121139480](https://cdn.llfc.club/img/image-20250412121139480.png)

上述图形演示的是单服务器踢人逻辑，多服务器踢人逻辑应配合分布式锁，锁住分布式的操作，保证在一个时刻只能一个客户端登录，客户端登陆完再解锁。

分布式登录我们放在下一节，这一节我们模拟两个客户端同账号登录同一个服务器，实现踢人逻辑。

## 2. 分布式锁和`redis`封装

为了更方便操作，我们将分布式锁加锁和解锁的操作封装到`redis`接口中

因为分布式锁也会占用连接，为了防止连接被占用耗尽连接池，所以我们提前扩大连接池的数量为10

``` cpp
RedisMgr::RedisMgr() {
	auto& gCfgMgr = ConfigMgr::Inst();
	auto host = gCfgMgr["Redis"]["Host"];
	auto port = gCfgMgr["Redis"]["Port"];
	auto pwd = gCfgMgr["Redis"]["Passwd"];
	_con_pool.reset(new RedisConPool(10, host.c_str(), atoi(port.c_str()), pwd.c_str()));
}
```

封装加锁操作, 内部调用了之前封装的分布式锁DistLock

``` cpp
std::string RedisMgr::acquireLock(const std::string& lockName,
	int lockTimeout, int acquireTimeout) {

	auto connect = _con_pool->getConnection();
	if (connect == nullptr) {
		return "";
	}

	Defer defer([&connect, this]() {
		_con_pool->returnConnection(connect);
	});

	return DistLock::Inst().acquireLock(connect, lockName, lockTimeout, acquireTimeout);
}
```

解锁操作

``` cpp
bool RedisMgr::releaseLock(const std::string& lockName,
	const std::string& identifier) {
	if (identifier.empty()) {
		return true;
	}
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) {
		return false;
	}


	Defer defer([&connect, this]() {
		_con_pool->returnConnection(connect);
		});

	return DistLock::Inst().releaseLock(connect, lockName, identifier);
}
```

## 3. 加锁和解锁调用

对于踢人逻辑，最难的就是思考如何加锁和解锁，进行踢人，以保证将来分布式登录也会安全。

这里我们先考虑几个情形

1. B新登录，此时A已登录，这种最简单，根据uid找到A的session发送踢人通知。
2. B新登录，此时A将下线，这种要保证B和A互斥，要么B先登陆完，A再下线，要么A先下线，B再登录。

​       这么做的好处就是保证互斥

​      如果B先登录，会将uid对应的session更新为最新的。A下线时会优先查找uid对应的session，发现不是自己，则直接退出即可，同时不需要修改uid对应的session为空。

​      如果A先退出，A下线时会优先查找uid对应的session, 发现uid对应的session和自己的连接吻合，则会将uid对应的session设置为空，然后B登录，将uid对应的session设置为新连接，这样是安全的。

3. B登录，A退出，此时C查找uid发送消息，三个操作都会添加分布式锁。谁先竞争到锁谁操作，能保证操作的互斥。

基本就是这三种情况。接下来我们回顾下uid和Session的对应关系

## 4. 用户和会话关系

添加用户和会话关联

``` cpp
class UserMgr: public Singleton<UserMgr>
{
	friend class Singleton<UserMgr>;
public:
	~UserMgr();
	std::shared_ptr<CSession> GetSession(int uid);
	void SetUserSession(int uid, std::shared_ptr<CSession> session);
	void RmvUserSession(int uid, std::string session_id);
private:
	UserMgr();
	std::mutex _session_mtx;
	std::unordered_map<int, std::shared_ptr<CSession>> _uid_to_session;
};
```

`UserMgr`中可以根据`uid`查找到对应的`CSession`。具体实现

``` cpp
#include "UserMgr.h"
#include "CSession.h"
#include "RedisMgr.h"

UserMgr:: ~ UserMgr(){
	_uid_to_session.clear();
}


std::shared_ptr<CSession> UserMgr::GetSession(int uid)
{
	std::lock_guard<std::mutex> lock(_session_mtx);
	auto iter = _uid_to_session.find(uid);
	if (iter == _uid_to_session.end()) {
		return nullptr;
	}

	return iter->second;
}

void UserMgr::SetUserSession(int uid, std::shared_ptr<CSession> session)
{
	std::lock_guard<std::mutex> lock(_session_mtx);
	_uid_to_session[uid] = session;
}

void UserMgr::RmvUserSession(int uid, std::string session_id)
{ 
	{
		std::lock_guard<std::mutex> lock(_session_mtx);
		auto iter = _uid_to_session.find(uid);
		if (iter != _uid_to_session.end()) {
			return;
		}
	
		auto session_id_ = iter->second->GetSessionId();
		//不相等说明是其他地方登录了
		if (session_id_ != session_id) {
			return;
		}
		_uid_to_session.erase(uid);
	}

}

UserMgr::UserMgr()
{

}
```

大家有没有注意到，对Session的操作没有加分布式锁，只加了线程锁，因为我的思路是在最外层加分布式锁，而接口内部只加线程锁，保证同一个服务器操作的原子性。

`CSession`类和之前一样, 里面有`user_id`和`session_id`

``` cpp
class CSession: public std::enable_shared_from_this<CSession>
{
public:
	CSession(boost::asio::io_context& io_context, CServer* server);
	~CSession();
	tcp::socket& GetSocket();
	std::string& GetSessionId();
	void SetUserId(int uid);
	int GetUserId();
	void Start();
	void Send(char* msg,  short max_length, short msgid);
	void Send(std::string msg, short msgid);
	void Close();
	std::shared_ptr<CSession> SharedSelf();
	void AsyncReadBody(int length);
	void AsyncReadHead(int total_len);
	void NotifyOffline(int uid);
private:
	void asyncReadFull(std::size_t maxLength, std::function<void(const boost::system::error_code& , std::size_t)> handler);
	void asyncReadLen(std::size_t  read_len, std::size_t total_len,
		std::function<void(const boost::system::error_code&, std::size_t)> handler);
	
	
	void HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> shared_self);
	tcp::socket _socket;
	std::string _session_id;
	char _data[MAX_LENGTH];
	CServer* _server;
	bool _b_close;
	std::queue<shared_ptr<SendNode> > _send_que;
	std::mutex _send_lock;
	//收到的消息结构
	std::shared_ptr<RecvNode> _recv_msg_node;
	bool _b_head_parse;
	//收到的头部结构
	std::shared_ptr<MsgNode> _recv_head_node;
	int _user_uid;
};
```

通过上述结构，我们可以通过`UserMgr`查找到`CSession`， 也可以通过`CSession`查找到`userid`, 实现了双向关联

## 5.登录添加分布式锁

我们需要对登录流程添加分布式锁，收到登录请求会做如下事情

1. 判断`token`和`uid`是否合理
2. 根据`uid`构造分布式锁`key`，然后实现分布式锁加锁操作。比如`uid`为1001，则分布式锁的key为"lock_1001"
3. 加锁后通过defer自动析构解锁
4. 通过`uid`获取用户之前登录的服务器，如果存在则说明uid对应的用户还在线，此时要做踢人，判断serverip和现在的服务器ip是否相等，如果相等则说明是

​       本服务器踢人，只需要通过线程锁控制好并发逻辑即可，将`uid`对应的旧`session`发送信息通知客户端下线，并且将旧`session`从`server`中移除。

​       如果不是本服务器，则要做跨服踢人，调用`grpc`踢人即可，留作之后做。

5. 登录成功后，要将`uid`和对应的`ip`信息写入`redis`,方便以后跨服查找。另外`uid`对应的`session`信息也要写入`redis`， 同时将`uid`和`session`关联，这样可以通过`uid`快速找到`session`

``` cpp
void LogicSystem::LoginHandler(shared_ptr<CSession> session, const short &msg_id, const string &msg_data) {
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid = root["uid"].asInt();
	auto token = root["token"].asString();
	std::cout << "user login uid is  " << uid << " user token  is "
		<< token << endl;

	Json::Value  rtvalue;
	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, MSG_CHAT_LOGIN_RSP);
		});


	//从redis获取用户token是否正确
	std::string uid_str = std::to_string(uid);
	std::string token_key = USERTOKENPREFIX + uid_str;
	std::string token_value = "";
	bool success = RedisMgr::GetInstance()->Get(token_key, token_value);
	if (!success) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return ;
	}

	if (token_value != token) {
		rtvalue["error"] = ErrorCodes::TokenInvalid;
		return ;
	}

	rtvalue["error"] = ErrorCodes::Success;

	//此处添加分布式锁，让该线程独占登录
	//拼接用户ip对应的key
	auto lock_key = LOCK_PREFIX + uid_str;
	auto identifier = RedisMgr::GetInstance()->acquireLock(lock_key, LOCK_TIME_OUT, ACQUIRE_TIME_OUT);
	//利用defer解锁
	Defer defer2([this, identifier, lock_key]() {
		RedisMgr::GetInstance()->releaseLock(lock_key, identifier);
		});
	//此处判断该用户是否在别处或者本服务器登录
	
	std::string uid_ip_value = "";
	auto uid_ip_key = USERIPPREFIX + uid_str;
	bool b_ip = RedisMgr::GetInstance()->Get(uid_ip_key, uid_ip_value);
	//说明用户已经登录了，此处应该踢掉之前的用户登录状态
	if (b_ip) {
		//获取当前服务器ip信息
		auto& cfg = ConfigMgr::Inst();
		auto self_name = cfg["SelfServer"]["Name"];
		//如果之前登录的服务器和当前相同，则直接在本服务器踢掉
		if (uid_ip_value == self_name) {
			//查找旧有的连接
			auto old_session = UserMgr::GetInstance()->GetSession(uid);
	
			//此处应该发送踢人消息
			if (old_session) {
				old_session->NotifyOffline(uid);
				//清除旧的连接
				_p_server->ClearSession(old_session->GetSessionId());
			}
				
		}
		else {
			//如果不是本服务器，则通知grpc通知其他服务器踢掉
		}
	}


	std::string base_key = USER_BASE_INFO + uid_str;
	auto user_info = std::make_shared<UserInfo>();
	bool b_base = GetBaseInfo(base_key, uid, user_info);
	if (!b_base) {
		rtvalue["error"] = ErrorCodes::UidInvalid;
		return;
	}
	rtvalue["uid"] = uid;
	rtvalue["pwd"] = user_info->pwd;
	rtvalue["name"] = user_info->name;
	rtvalue["email"] = user_info->email;
	rtvalue["nick"] = user_info->nick;
	rtvalue["desc"] = user_info->desc;
	rtvalue["sex"] = user_info->sex;
	rtvalue["icon"] = user_info->icon;

	//从数据库获取申请列表
	std::vector<std::shared_ptr<ApplyInfo>> apply_list;
	auto b_apply = GetFriendApplyInfo(uid,apply_list);
	if (b_apply) {
		for (auto & apply : apply_list) {
			Json::Value obj;
			obj["name"] = apply->_name;
			obj["uid"] = apply->_uid;
			obj["icon"] = apply->_icon;
			obj["nick"] = apply->_nick;
			obj["sex"] = apply->_sex;
			obj["desc"] = apply->_desc;
			obj["status"] = apply->_status;
			rtvalue["apply_list"].append(obj);
		}
	}

	//获取好友列表
	std::vector<std::shared_ptr<UserInfo>> friend_list;
	bool b_friend_list = GetFriendList(uid, friend_list);
	for (auto& friend_ele : friend_list) {
		Json::Value obj;
		obj["name"] = friend_ele->name;
		obj["uid"] = friend_ele->uid;
		obj["icon"] = friend_ele->icon;
		obj["nick"] = friend_ele->nick;
		obj["sex"] = friend_ele->sex;
		obj["desc"] = friend_ele->desc;
		obj["back"] = friend_ele->back;
		rtvalue["friend_list"].append(obj);
	}

	auto server_name = ConfigMgr::Inst().GetValue("SelfServer", "Name");
	//将登录数量增加
	auto rd_res = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server_name);
	int count = 0;
	if (!rd_res.empty()) {
		count = std::stoi(rd_res);
	}

	count++;
	auto count_str = std::to_string(count);
	RedisMgr::GetInstance()->HSet(LOGIN_COUNT, server_name, count_str);

	//session绑定用户uid
	session->SetUserId(uid);
	//为用户设置登录ip server的名字
	std::string  ipkey = USERIPPREFIX + uid_str;
	RedisMgr::GetInstance()->Set(ipkey, server_name);
	//uid和session绑定管理,方便以后踢人操作
	UserMgr::GetInstance()->SetUserSession(uid, session);
	std::string  uid_session_key = USER_SESSION_PREFIX + uid_str;
	RedisMgr::GetInstance()->Set(uid_session_key, session->GetSessionId());
	return;
}
```



## 6. 检测离线处理

服务器也会检测到离线也会清理连接，但是要注意，连接可以不按照分布式锁加锁清理，但是连接的信息要加分布式锁后再更新。

比如是否将`uid`对应的`session`更新到`redis`中，因为很可能用户在别的新服务器登录，新服务器给旧的客户端通知离线，旧的客户端不按理连接，导致旧的服务器检测连接断开，此时不能将`uid`对应的`session`清空，因为`uid`对应的`session`已经被新服务器更新了。

![image-20250412144455898](https://cdn.llfc.club/img/image-20250412144455898.png)

在发送和接收的时候都可能检测到对方离线而报错，所以在`AsyncReadBody`和`AsyncReadHead`以及`AsyncWrite`等错误处理的时候记得加上连接清理操作

我们以读取body为例

``` cpp
void CSession::AsyncReadBody(int total_len)
{
	auto self = shared_from_this();
	asyncReadFull(total_len, [self, this, total_len](const boost::system::error_code& ec, std::size_t bytes_transfered) {
		try {
			if (ec) {
				std::cout << "handle read failed, error is " << ec.what() << endl;
				Close();

				//加锁清除session
				auto uid_str = std::to_string(_user_uid);
				auto lock_key = LOCK_PREFIX + uid_str;
				auto identifier = RedisMgr::GetInstance()->acquireLock(lock_key, LOCK_TIME_OUT, ACQUIRE_TIME_OUT);
				Defer defer([identifier, lock_key,self,this]() {
					_server->ClearSession(_session_id);
					RedisMgr::GetInstance()->releaseLock(lock_key, identifier);
					});

				if (identifier.empty()) {
					return;
				}	
				std::string redis_session_id = "";
				auto bsuccess = RedisMgr::GetInstance()->Get(USER_SESSION_PREFIX + uid_str, redis_session_id);
				if (!bsuccess) {
					return;
				}

				if (redis_session_id != _session_id) {
					//说明有客户在其他服务器异地登录了
					return;
				}

				RedisMgr::GetInstance()->Del(USER_SESSION_PREFIX + uid_str);
				//清除用户登录信息
				RedisMgr::GetInstance()->Del(USERIPPREFIX + uid_str);
				return;
			}

			if (bytes_transfered < total_len) {
				std::cout << "read length not match, read [" << bytes_transfered << "] , total ["
					<< total_len<<"]" << endl;
				Close();
				_server->ClearSession(_session_id);
				return;
			}

			memcpy(_recv_msg_node->_data , _data , bytes_transfered);
			_recv_msg_node->_cur_len += bytes_transfered;
			_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
			cout << "receive data is " << _recv_msg_node->_data << endl;
			//此处将消息投递到逻辑队列中
			LogicSystem::GetInstance()->PostMsgToQue(make_shared<LogicNode>(shared_from_this(), _recv_msg_node));
			//继续监听头部接受事件
			AsyncReadHead(HEAD_TOTAL_LEN);
		}
		catch (std::exception& e) {
			std::cout << "Exception code is " << e.what() << endl;
		}
		});
}
```

## 7. 测试效果

本节先测试单服务器同账号不同客户端登录情况，为了将同账号客户端派发到同一个服务器，暂时修改`StatusServer`的派发逻辑为同一个服务器

``` cpp
ChatServer StatusServiceImpl::getChatServer() {
	std::lock_guard<std::mutex> guard(_server_mtx);
	auto minServer = _servers.begin()->second;
	//暂时注释，测试单服务器模式
	//auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, minServer.name);
	//if (count_str.empty()) {
	//	//不存在则默认设置为最大
	//	minServer.con_count = INT_MAX;
	//}
	//else {
	//	minServer.con_count = std::stoi(count_str);
	//}


	//// 使用范围基于for循环
	//for ( auto& server : _servers) {
	//	
	//	if (server.second.name == minServer.name) {
	//		continue;
	//	}

	//	auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server.second.name);
	//	if (count_str.empty()) {
	//		server.second.con_count = INT_MAX;
	//	}
	//	else {
	//		server.second.con_count = std::stoi(count_str);
	//	}

	//	if (server.second.con_count < minServer.con_count) {
	//		minServer = server.second;
	//	}
	//}

	return minServer;
}
```



![image-20250412150004397](https://cdn.llfc.club/img/image-20250412150004397.png)

![image-20250412150038381](https://cdn.llfc.club/img/image-20250412150038381.png)

## 8. 待做事项

1. 跨服踢人留作下一节处理
2. 心跳检测未作，留作以后处理
3. 心跳检测发现僵尸连接，需要踢人，留作以后处理。

## 9. 源码

https://gitee.com/secondtonone1/llfcchat
