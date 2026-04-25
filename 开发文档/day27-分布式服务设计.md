## 简介

本文介绍如何将chatserver设置为分布式服务，并且实现statusserver的负载均衡处理，根据每个chatserver现有的连接数匹配最小的chatserver返回给GateServer并返回给客户端。

为了实现这一系列分布式设计，我们需要先完善chatserver，增加grpc客户端和服务端。这样能实现两个chatserver之间端对端的通信。

visual studio中右键chatserver项目选择添加新文件ChatGrpcClient, 会为我们生成ChatGrpcClient.h和ChatGrpcClient.cpp文件。

## 连接池客户端
先实现ChatConPool连接池

``` cpp
class ChatConPool {
public:
	ChatConPool(size_t poolSize, std::string host, std::string port):
		poolSize_(poolSize), host_(host),port_(port),b_stop_(false){
		for (size_t i = 0; i < poolSize_; ++i) {
			std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());
			connections_.push(ChatService::NewStub(channel));
		}
	}

	~ChatConPool() {
		std::lock_guard<std::mutex> lock(mutex_);
		Close();
		while (!connections_.empty()) {
			connections_.pop();
		}
	}

	std::unique_ptr<ChatService::Stub> getConnection() {
		std::unique_lock<std::mutex> lock(mutex_);
		cond_.wait(lock, [this] {
			if (b_stop_) {
				return true;
			}
			return !connections_.empty();
			});
		//如果停止则直接返回空指针
		if (b_stop_) {
			return  nullptr;
		}
		auto context = std::move(connections_.front());
		connections_.pop();
		return context;
	}

	void returnConnection(std::unique_ptr<ChatService::Stub> context) {
		std::lock_guard<std::mutex> lock(mutex_);
		if (b_stop_) {
			return;
		}
		connections_.push(std::move(context));
		cond_.notify_one();
	}

	void Close() {
		b_stop_ = true;
		cond_.notify_all();
	}

private:
	atomic<bool> b_stop_;
	size_t poolSize_;
	std::string host_;
	std::string port_;
	std::queue<std::unique_ptr<ChatService::Stub> > connections_;
	std::mutex mutex_;
	std::condition_variable cond_;
};
```
然后利用单例模式实现grpc通信的客户端
``` cpp
class ChatGrpcClient: public Singleton<ChatGrpcClient>
{
	friend class Singleton<ChatGrpcClient>;

public:
	~ChatGrpcClient() {

	}

	AddFriendRsp NotifyAddFriend(std::string server_ip, const AddFriendReq& req);
	AuthFriendRsp NotifyAuthFriend(std::string server_ip, const AuthFriendReq& req);
	bool GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo);
	TextChatMsgRsp NotifyTextChatMsg(std::string server_ip, const TextChatMsgReq& req, const Json::Value& rtvalue);
private:
	ChatGrpcClient();
	unordered_map<std::string, std::unique_ptr<ChatConPool>> _pools;
};
```
实现具体的ChatGrpcClient
``` cpp
ChatGrpcClient::ChatGrpcClient()
{
	auto& cfg = ConfigMgr::Inst();
	auto server_list = cfg["PeerServer"]["Servers"];

	std::vector<std::string> words;

	std::stringstream ss(server_list);
	std::string word;

	while (std::getline(ss, word, ',')) {
		words.push_back(word);
	}

	for (auto& word : words) {
		if (cfg[word]["Name"].empty()) {
			continue;
		}
		_pools[cfg[word]["Name"]] = std::make_unique<ChatConPool>(5, cfg[word]["Host"], cfg[word]["Port"]);
	}

}

AddFriendRsp ChatGrpcClient::NotifyAddFriend(std::string server_ip, const AddFriendReq& req) {
	AddFriendRsp rsp;
	return rsp;
}

AuthFriendRsp ChatGrpcClient::NotifyAuthFriend(std::string server_ip, const AuthFriendReq& req) {
	AuthFriendRsp rsp;
	return rsp;
}

bool ChatGrpcClient::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo) {
	return true;
}

TextChatMsgRsp ChatGrpcClient::NotifyTextChatMsg(std::string server_ip, 
	const TextChatMsgReq& req, const Json::Value& rtvalue) {

	TextChatMsgRsp rsp;
	return rsp;
}
```
## 连接池服务端
向ChatServer中添加ChatServiceImpl类，自动生成头文件和源文件
``` cpp
class ChatServiceImpl final : public ChatService::Service
{
public:
	ChatServiceImpl();
	Status NotifyAddFriend(ServerContext* context, const AddFriendReq* request,
		AddFriendRsp* reply) override;

	Status NotifyAuthFriend(ServerContext* context,
		const AuthFriendReq* request, AuthFriendRsp* response) override;

	Status NotifyTextChatMsg(::grpc::ServerContext* context,
		const TextChatMsgReq* request, TextChatMsgRsp* response) override;

	bool GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo);

private:
};
```
实现服务逻辑,先简单写成不处理直接返回。
``` cpp
ChatServiceImpl::ChatServiceImpl()
{

}

Status ChatServiceImpl::NotifyAddFriend(ServerContext* context, const AddFriendReq* request,
	AddFriendRsp* reply) {
	return Status::OK;
}

Status ChatServiceImpl::NotifyAuthFriend(ServerContext* context,
	const AuthFriendReq* request, AuthFriendRsp* response) {
	return Status::OK;
}

Status ChatServiceImpl::NotifyTextChatMsg(::grpc::ServerContext* context,
	const TextChatMsgReq* request, TextChatMsgRsp* response) {
	return Status::OK;
}

bool ChatServiceImpl::GetBaseInfo(std::string base_key, int uid, std::shared_ptr<UserInfo>& userinfo) {
	return true;
}
```
并且完善chatserver配置
``` ini
[GateServer]
Port = 8080
[VarifyServer]
Host = 127.0.0.1
Port = 50051
[StatusServer]
Host = 127.0.0.1
Port = 50052
[SelfServer]
Name = chatserver1
Host = 0.0.0.0
Port  = 8090
RPCPort = 50055
[Mysql]
Host = 81.68.86.146
Port = 3308
User = root
Passwd = 123456.
Schema = llfc
[Redis]
Host = 81.68.86.146
Port = 6380
Passwd = 123456
[PeerServer]
Servers = chatserver2
[chatserver2]
Name = chatserver2
Host = 127.0.0.1
Port = 50056
```
增加了PeerServer字段，存储对端server列表，通过逗号分隔，可以通过逗号切割对端服务器名字，再根据名字去配置里查找对应字段。

对应的chatserver复制一份，改名为chatserver2，然后修改config.ini配置。要和server1配置不同，实现端对端的配置。具体详见服务器代码。

## 服务器连接数管理
每当服务器chatserver启动后，都要重新设置一下用户连接数管理,并且我们每个chatserver既要有tcp服务监听也要有grpc服务监听
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
		RedisMgr::GetInstance()->HSet(LOGIN_COUNT, server_name, "0");
		//定义一个GrpcServer

		std::string server_address(cfg["SelfServer"]["Host"] + ":" + cfg["SelfServer"]["RPCPort"]);
		ChatServiceImpl service;
		grpc::ServerBuilder builder;
		// 监听端口和添加服务
		builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
		builder.RegisterService(&service);
		// 构建并启动gRPC服务器
		std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
		std::cout << "RPC Server listening on " << server_address << std::endl;


		//单独启动一个线程处理grpc服务
		std::thread  grpc_server_thread([&server]() {
			server->Wait();
			});


		boost::asio::io_context  io_context;
		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
		signals.async_wait([&io_context, pool, &server](auto, auto) {
			io_context.stop();
			pool->Stop();
			server->Shutdown();
			});

		auto port_str = cfg["SelfServer"]["Port"];
		CServer s(io_context, atoi(port_str.c_str()));
		io_context.run();

		RedisMgr::GetInstance()->HDel(LOGIN_COUNT, server_name);
		RedisMgr::GetInstance()->Close();
		grpc_server_thread.join();
	}
	catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << endl;
	}
}
```
我们在服务器启动后将本服务器的登录数量设置为0.

同样的道理，我们将服务器关闭后，也要删除对应key。

## 用户连接管理
因为我们用户登录后，要将连接(session)和用户uid绑定。为以后登陆踢人做准备。所以新增UserMgr管理类.

其声明如下
``` cpp
class CSession;
class UserMgr : public Singleton<UserMgr>
{
	friend class Singleton<UserMgr>;
public:
	~UserMgr();
	std::shared_ptr<CSession> GetSession(int uid);
	void SetUserSession(int uid, std::shared_ptr<CSession> session);
	void RmvUserSession(int uid);

private:
	UserMgr();
	std::mutex _session_mtx;
	std::unordered_map<int, std::shared_ptr<CSession>> _uid_to_session;
};
```
其实现如下
``` cpp
UserMgr:: ~UserMgr() {
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

void UserMgr::RmvUserSession(int uid)
{
	auto uid_str = std::to_string(uid);
	//因为再次登录可能是其他服务器，所以会造成本服务器删除key，其他服务器注册key的情况
	// 有可能其他服务登录，本服删除key造成找不到key的情况
	//RedisMgr::GetInstance()->Del(USERIPPREFIX + uid_str);

	{
		std::lock_guard<std::mutex> lock(_session_mtx);
		_uid_to_session.erase(uid);
	}

}

UserMgr::UserMgr()
{

}

```

RmvUserSession 暂时屏蔽，以后做登录踢人后能保证有序移除用户ip操作。

当有连接异常时，可以调用移除用户Session的接口

``` cpp
void CServer::ClearSession(std::string session_id) {
	
	if (_sessions.find(session_id) != _sessions.end()) {
		//移除用户和session的关联
		UserMgr::GetInstance()->RmvUserSession(_sessions[session_id]->GetUserId());
	}
	
	{
		lock_guard<mutex> lock(_mutex);
		_sessions.erase(session_id);
	}

}
```
聊天服务完善用户登录，当用户登录后, 设置其uid对应的serverip。以及更新其所在服务器的连接数。
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
		return;
	}

	if (token_value != token) {
		rtvalue["error"] = ErrorCodes::TokenInvalid;
		return;
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

	//获取好友列表

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

	return;
}
```
## 状态服务
状态服务更新配置
``` ini
[StatusServer]
Port = 50052
Host = 0.0.0.0
[Mysql]
Host = 81.68.86.146
Port = 3308
User = root
Passwd = 123456.
Schema = llfc
[Redis]
Host = 81.68.86.146
Port = 6380
Passwd = 123456
[chatservers]
Name = chatserver1,chatserver2
[chatserver1]
Name = chatserver1
Host = 127.0.0.1
Port = 8090
[chatserver2]
Name = chatserver2
Host = 127.0.0.1
Port = 8091
```
配置文件同样增加了chatservers列表，用来管理多个服务，接下来实现根据连接数动态返回chatserverip的功能
``` cpp
Status StatusServiceImpl::GetChatServer(ServerContext* context, 
	const GetChatServerReq* request, GetChatServerRsp* reply)
{
	std::string prefix("llfc status server has received :  ");
	const auto& server = getChatServer();
	reply->set_host(server.host);
	reply->set_port(server.port);
	reply->set_error(ErrorCodes::Success);
	reply->set_token(generate_unique_string());
	insertToken(request->uid(), reply->token());
	return Status::OK;
}
```
getChatServer用来获取最小连接数的chatserver 名字
``` cpp
ChatServer StatusServiceImpl::getChatServer() {
	std::lock_guard<std::mutex> guard(_server_mtx);
	auto minServer = _servers.begin()->second;
	auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, minServer.name);
	if (count_str.empty()) {
		//不存在则默认设置为最大
		minServer.con_count = INT_MAX;
	}
	else {
		minServer.con_count = std::stoi(count_str);
	}


	// 使用范围基于for循环
	for (auto& server : _servers) {

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
## 测试

分别启动两个chatserver，gateserver，以及statusserver，并且启动两个客户端登录，

分别查看登录信息，发现两个客户端被分配到不同的chatserver了，说明我们实现了负载均衡的分配方式。

![https://cdn.llfc.club/1722314087856.jpg](https://cdn.llfc.club/1722314087856.jpg)

## 源码连接

[https://gitee.com/secondtonone1/llfcchat](https://gitee.com/secondtonone1/llfcchat)

## 视频连接

[https://www.bilibili.com/video/BV17r421K7Px/?spm_id_from=333.999.0.0&vd_source=8be9e83424c2ed2c9b2a3ed1d01385e9](https://www.bilibili.com/video/BV17r421K7Px/?spm_id_from=333.999.0.0&vd_source=8be9e83424c2ed2c9b2a3ed1d01385e9)

