---
title: 跨服踢人逻辑实现
date: 2025-04-19 10:25:18
tags: [C++聊天项目]
categories: [C++聊天项目]
---

## 前情回顾

前文我们实现了单服务器踢人的逻辑，通过分布式锁锁住登录过程，在这个期间对用户相关的信息进行更改，主要包括用户id对应的`serverip`, `sessionid`等。

同时对用户离线消息进行了处理，也是通过分布式锁锁住退出过程，判断此时用户`id`对应的`sessionid`是否和本服记录相等，如果不相等则说明有用户异地登录，此时只要退出即可，否则要清理id对应的`sessionid`以及`serverip`等信息。

接下来我们实现跨服踢人逻辑



## RPC封装

因为跨服踢人，所以要调用`Grpc`踢人，我们在`message.proto`中添加踢人消息

``` cpp
message KickUserReq{
	int32 uid = 1;
}

message KickUserRsp{
	int32 error = 1;
	int32 uid = 2;
}
```

同时添加服务调用

``` cpp
service ChatService {
    //...其他服务略去
	rpc NotifyKickUser(KickUserReq) returns (KickUserRsp){}
}
```

编写bat脚本自动生成， `start.bat`内容如下

``` bash
@echo off
set PROTOC_PATH=D:\cppsoft\grpc\visualpro\third_party\protobuf\Debug\protoc.exe
set GRPC_PLUGIN_PATH=D:\cppsoft\grpc\visualpro\Debug\grpc_cpp_plugin.exe
set PROTO_FILE=message.proto

echo Generating gRPC code...
%PROTOC_PATH% -I="." --grpc_out="." --plugin=protoc-gen-grpc="%GRPC_PLUGIN_PATH%" "%PROTO_FILE%"

echo Generating C++ code...
%PROTOC_PATH% --cpp_out=. "%PROTO_FILE%"

echo Done.
```

双击`start.bat`或者在`cmd`中执行`start.bat`也可以

执行后可以发现产生了四个文件

![image-20250419114210735](https://cdn.llfc.club/img/image-20250419114210735.png)

## 跨服踢人示意图

<img src="https://cdn.llfc.club/img/image-20250419115212041.png" alt="image-20250419115212041" style="zoom: 80%;" />

## 逻辑编写

### `StatusServer`动态分配

`StatusServer`中修改动态分配server逻辑

``` cpp
ChatServer StatusServiceImpl::getChatServer() {
	std::lock_guard<std::mutex> guard(_server_mtx);
	auto minServer = _servers.begin()->second;
	auto lock_key = LOCK_COUNT;
	auto identifier = RedisMgr::GetInstance()->acquireLock(lock_key, LOCK_TIME_OUT, ACQUIRE_TIME_OUT);
	//利用defer解锁
	Defer defer2([this, identifier, lock_key]() {
		RedisMgr::GetInstance()->releaseLock(lock_key, identifier);
		});

	auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, minServer.name);
	if (count_str.empty()) {
		//不存在则默认设置为最大
		minServer.con_count = INT_MAX;
	}
	else {
		minServer.con_count = std::stoi(count_str);
	}


	// 使用范围基于for循环
	for ( auto& server : _servers) {
		
		if (server.second.name == minServer.name) {
			continue;
		}

		auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server.second.name);
		if (count_str.empty()) {
			server.second.con_count = INT_MAX;
		}
		else {
			server.second.con_count = std::stoi(count_str);
		}

		if (server.second.con_count < minServer.con_count) {
			minServer = server.second;
		}
	}

	return minServer;
}
```

注意这里用到了另一个分布式锁，用来控制服务器人数记录

``` cpp
auto identifier = RedisMgr::GetInstance()->acquireLock(lock_key, LOCK_TIME_OUT, ACQUIRE_TIME_OUT);
```

### `ChatServer`踢人逻辑

`ChatSever`中登录逻辑里添加跨服踢人调用

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
	auto b_apply = GetFriendApplyInfo(uid, apply_list);
	if (b_apply) {
		for (auto& apply : apply_list) {
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
	{
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
				//发送通知
				KickUserReq kick_req;
				kick_req.set_uid(uid);
				ChatGrpcClient::GetInstance()->NotifyKickUser(uid_ip_value, kick_req);
			}
		}

		//session绑定用户uid
		session->SetUserId(uid);
		//为用户设置登录ip server的名字
		std::string  ipkey = USERIPPREFIX + uid_str;
		RedisMgr::GetInstance()->Set(ipkey, server_name);
		//uid和session绑定管理,方便以后踢人操作
		UserMgr::GetInstance()->SetUserSession(uid, session);
		std::string  uid_session_key = USER_SESSION_PREFIX + uid_str;
		RedisMgr::GetInstance()->Set(uid_session_key, session->GetSessionId());

	}

	RedisMgr::GetInstance()->IncreaseCount(server_name);
	return;
}
```

注意上面代码，这段代码就是跨服踢人逻辑。

``` cpp
			else {
				//如果不是本服务器，则通知grpc通知其他服务器踢掉
				//发送通知
				KickUserReq kick_req;
				kick_req.set_uid(uid);
				ChatGrpcClient::GetInstance()->NotifyKickUser(uid_ip_value, kick_req);
			}
```

关于`KickUserReq`其实是我们在`message.pb.h`中生成的。但是我们在自己的文件中使用要用作用域`messag::`, 所以我们在`GrpcClient.h`中添加声明

``` cpp
using message::KickUserReq;
using message::KickUserRsp;
```

以后我们包含`GrpcClient.h`就可以使用这些类了。

### 封装`rpc`踢人

接下来我们封装`rpc`接口实现踢人逻辑

`rpc`客户端接口

``` cpp
KickUserRsp ChatGrpcClient::NotifyKickUser(std::string server_ip, const KickUserReq& req)
{
	KickUserRsp rsp;
	Defer defer([&rsp, &req]() {
		rsp.set_error(ErrorCodes::Success);
		rsp.set_uid(req.uid());
		});

	auto find_iter = _pools.find(server_ip);
	if (find_iter == _pools.end()) {
		return rsp;
	}

	auto& pool = find_iter->second;
	ClientContext context;
	auto stub = pool->getConnection();
	Defer defercon([&stub, this, &pool]() {
		pool->returnConnection(std::move(stub));
		});
	Status status = stub->NotifyKickUser(&context, req, &rsp);

	if (!status.ok()) {
		rsp.set_error(ErrorCodes::RPCFailed);
		return rsp;
	}

	return rsp;
}
```

`rpc`服务端接口实现

``` cpp
Status ChatServiceImpl::NotifyKickUser(::grpc::ServerContext* context, 
	const KickUserReq* request, KickUserRsp* reply)
{
	//查找用户是否在本服务器
	auto uid = request->uid();
	auto session = UserMgr::GetInstance()->GetSession(uid);

	Defer defer([request, reply]() {
		reply->set_error(ErrorCodes::Success);
		reply->set_uid(request->uid());
		});

	//用户不在内存中则直接返回
	if (session == nullptr) {
		return Status::OK;
	}

	//在内存中则直接发送通知对方
	session->NotifyOffline(uid);
	//清除旧的连接
	_p_server->ClearSession(session->GetSessionId());

	return Status::OK;
}
```

为了让`ChatServiceImpl` 获取`CServer`, 所以我们提供了注册函数

``` cpp
void ChatServiceImpl::RegisterServer(std::shared_ptr<CServer> pServer)
{
	_p_server = pServer;
}
```

这个函数在`main`函数中启动`grpc`服务前注册即可。

### 登录数量统计

在`StatusServer`中利用分布式锁获取登录数量，动态分配`Server`给客户端，这里我们也要用`ChatServer`启动和退出时清空登录数量

重新调整`ChatServer`启动逻辑

``` cpp
using namespace std;
bool bstop = false;
std::condition_variable cond_quit;
std::mutex mutex_quit;

int main()
{
	auto& cfg = ConfigMgr::Inst();
	auto server_name = cfg["SelfServer"]["Name"];
	try {
		auto pool = AsioIOServicePool::GetInstance();
		//将登录数设置为0
		RedisMgr::GetInstance()->InitCount(server_name);
		Defer derfer ([server_name]() {
				RedisMgr::GetInstance()->HDel(LOGIN_COUNT, server_name);
				RedisMgr::GetInstance()->Close();
			});

		boost::asio::io_context  io_context;
		auto port_str = cfg["SelfServer"]["Port"];
		//创建Cserver智能指针
		auto pointer_server = std::make_shared<CServer>(io_context, atoi(port_str.c_str()));
		//定义一个GrpcServer
		std::string server_address(cfg["SelfServer"]["Host"] + ":" + cfg["SelfServer"]["RPCPort"]);
		ChatServiceImpl service;
		grpc::ServerBuilder builder;
		// 监听端口和添加服务
		builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
		builder.RegisterService(&service);
		service.RegisterServer(pointer_server);
		// 构建并启动gRPC服务器
		std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
		std::cout << "RPC Server listening on " << server_address << std::endl;

		//单独启动一个线程处理grpc服务
		std::thread  grpc_server_thread([&server]() {
				server->Wait();
			});

		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
		signals.async_wait([&io_context, pool, &server](auto, auto) {
			io_context.stop();
			pool->Stop();
			server->Shutdown();
			});
		
		//将Cserver注册给逻辑类方便以后清除连接
		LogicSystem::GetInstance()->SetServer(pointer_server);
		io_context.run();

		grpc_server_thread.join();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << endl;
	}

}
```

上面的逻辑有这样一段，要格外注意

``` cpp
		//将登录数设置为0
		RedisMgr::GetInstance()->InitCount(server_name);
		Defer derfer ([server_name]() {
				RedisMgr::GetInstance()->HDel(LOGIN_COUNT, server_name);
				RedisMgr::GetInstance()->Close();
			});
```

这段逻辑是在服务器启动后将对应服务器中连接数清零写入`redis`，在服务器结束后从`redis`中删除数量信息，最后关闭`Redis`连接池

## 源码

源码地址

https://gitee.com/secondtonone1/llfcchat

