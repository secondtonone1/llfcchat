## 客户端登录功能
登录界面新增err_tip，用来提示用户登陆结果。至于密码输入框大家可以根据注册界面的逻辑实现隐藏和显示的功能。这里留给大家自己实现。

![https://cdn.llfc.club/1713228480446.jpg](https://cdn.llfc.club/1713228480446.jpg)

点击登录需要发送http 请求到GateServer，GateServer先验证登录密码，再调用grpc请求给StatusServer，获取聊天服务器ip信息和token信息反馈给客户端。

结构图如下

![https://cdn.llfc.club/1713230325540.jpg](https://cdn.llfc.club/1713230325540.jpg)

我们先实现客户端登录,为登录按钮添加槽函数响应

``` cpp
void LoginDialog::on_login_btn_clicked()
{
    qDebug()<<"login btn clicked";
    if(checkUserValid() == false){
        return;
    }

    if(checkPwdValid() == false){
        return ;
    }

    auto user = ui->user_edit->text();
    auto pwd = ui->pass_edit->text();
    //发送http请求登录
    QJsonObject json_obj;
    json_obj["user"] = user;
    json_obj["passwd"] = xorString(pwd);
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/user_login"),
                                        json_obj, ReqId::ID_LOGIN_USER,Modules::LOGINMOD);
}
```
增加检测函数
``` cpp
bool LoginDialog::checkUserValid(){

    auto user = ui->user_edit->text();
    if(user.isEmpty()){
        qDebug() << "User empty " ;
        return false;
    }

    return true;
}

bool LoginDialog::checkPwdValid(){
    auto pwd = ui->pass_edit->text();
    if(pwd.length() < 6 || pwd.length() > 15){
        qDebug() << "Pass length invalid";
        return false;
    }

    return true;
}
```
在HttpMgr中添加sig_login_mod_finish信号，收到http请求完成回包的槽函数中添加登录模块的响应，将登录模块的消息发送到登录界面

``` cpp
void HttpMgr::slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod)
{
    if(mod == Modules::REGISTERMOD){
        //发送信号通知指定模块http响应结束
        emit sig_reg_mod_finish(id, res, err);
    }

    if(mod == Modules::RESETMOD){
        //发送信号通知指定模块http响应结束
        emit sig_reset_mod_finish(id, res, err);
    }

    if(mod == Modules::LOGINMOD){
        emit sig_login_mod_finish(id, res, err);
    }
}
```
在LoginDialog的构造函数中添加消息对应的槽函数连接
``` cpp
LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    connect(ui->reg_btn, &QPushButton::clicked, this, &LoginDialog::switchRegister);
    ui->forget_label->SetState("normal","hover","","selected","selected_hover","");
    ui->forget_label->setCursor(Qt::PointingHandCursor);
    connect(ui->forget_label, &ClickedLabel::clicked, this, &LoginDialog::slot_forget_pwd);
    initHttpHandlers();
    //连接登录回包信号
    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_login_mod_finish, this,
            &LoginDialog::slot_login_mod_finish);
}
```
initHttpHandlers为初始化http回调逻辑, 并添加_handlers成员
```cpp
void LoginDialog::initHttpHandlers()
{
    //注册获取登录回包逻辑
    _handlers.insert(ReqId::ID_LOGIN_USER, [this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"),false);
            return;
        }
        auto user = jsonObj["user"].toString();
        showTip(tr("登录成功"), true);
        qDebug()<< "user is " << user ;
    });
}
```

在LoginDialog中添加槽函数slot_login_mod_finish

``` cpp
void LoginDialog::slot_login_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if(err != ErrorCodes::SUCCESS){
        showTip(tr("网络请求错误"),false);
        return;
    }

    // 解析 JSON 字符串,res需转化为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    //json解析错误
    if(jsonDoc.isNull()){
        showTip(tr("json解析错误"),false);
        return;
    }

    //json解析错误
    if(!jsonDoc.isObject()){
        showTip(tr("json解析错误"),false);
        return;
    }


    //调用对应的逻辑,根据id回调。
    _handlers[id](jsonDoc.object());

    return;
}
```
到此客户端登陆请求发送的模块封装完了

## GateServer完善登陆逻辑
在LogicSystem的构造函数中添加登陆请求的注册。
``` cpp
//用户登录逻辑
RegPost("/user_login", [](std::shared_ptr<HttpConnection> connection) {
    auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
    std::cout << "receive body is " << body_str << std::endl;
    connection->_response.set(http::field::content_type, "text/json");
    Json::Value root;
    Json::Reader reader;
    Json::Value src_root;
    bool parse_success = reader.parse(body_str, src_root);
    if (!parse_success) {
        std::cout << "Failed to parse JSON data!" << std::endl;
        root["error"] = ErrorCodes::Error_Json;
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->_response.body()) << jsonstr;
        return true;
    }

    auto name = src_root["user"].asString();
    auto pwd = src_root["passwd"].asString();
    UserInfo userInfo;
    //查询数据库判断用户名和密码是否匹配
    bool pwd_valid = MysqlMgr::GetInstance()->CheckPwd(name, pwd, userInfo);
    if (!pwd_valid) {
        std::cout << " user pwd not match" << std::endl;
        root["error"] = ErrorCodes::PasswdInvalid;
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->_response.body()) << jsonstr;
        return true;
    }

    //查询StatusServer找到合适的连接
    auto reply = StatusGrpcClient::GetInstance()->GetChatServer(userInfo.uid);
    if (reply.error()) {
        std::cout << " grpc get chat server failed, error is " << reply.error()<< std::endl;
        root["error"] = ErrorCodes::RPCGetFailed;
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->_response.body()) << jsonstr;
        return true;
    }

    std::cout << "succeed to load userinfo uid is " << userInfo.uid << std::endl;
    root["error"] = 0;
    root["user"] = name;
    root["uid"] = userInfo.uid;
    root["token"] = reply.token();
    root["host"] = reply.host();
    std::string jsonstr = root.toStyledString();
    beast::ostream(connection->_response.body()) << jsonstr;
    return true;
    });
```

在MysqlMgr中添加CheckPwd函数

``` cpp
bool MysqlMgr::CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo) {
	return _dao.CheckPwd(name, pwd, userInfo);
}
```
在DAO层添加根据用户名查询sql逻辑,并且判断pwd是否匹配。
``` cpp
bool MysqlDao::CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo) {
	auto con = pool_->getConnection();
	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
		});

	try {
		if (con == nullptr) {
			return false;
		}

		// 准备SQL语句
		std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("SELECT * FROM user WHERE name = ?"));
		pstmt->setString(1, name); // 将username替换为你要查询的用户名

		// 执行查询
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		std::string origin_pwd = "";
		// 遍历结果集
		while (res->next()) {
			origin_pwd = res->getString("pwd");
			// 输出查询到的密码
			std::cout << "Password: " << origin_pwd << std::endl;
			break;
		}

		if (pwd != origin_pwd) {
			return false;
		}
		userInfo.name = name;
		userInfo.email = res->getString("email");
		userInfo.uid = res->getInt("uid");
		userInfo.pwd = origin_pwd;
		return true;
	}
	catch (sql::SQLException& e) {
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}
```
因为要调用grpc访问StatusServer，所以我们这里先完善协议proto文件
``` proto
syntax = "proto3";

package message;

service VarifyService {
  rpc GetVarifyCode (GetVarifyReq) returns (GetVarifyRsp) {}
}

message GetVarifyReq {
  string email = 1;
}

message GetVarifyRsp {
  int32 error = 1;
  string email = 2;
  string code = 3;
}

message GetChatServerReq {
  int32 uid = 1;
}

message GetChatServerRsp {
  int32 error = 1;
  string host = 2;
  string port = 3;
  string token = 4;
}

service StatusService {
	rpc GetChatServer (GetChatServerReq) returns (GetChatServerRsp) {}
}
```
我们用下面两条命令重新生成pb.h和grpc.pb.h
``` bash
D:\cppsoft\grpc\visualpro\third_party\protobuf\Debug\protoc.exe --cpp_out=. "message.proto"
```
生成grpc.pb.h
``` bash
D:\cppsoft\grpc\visualpro\third_party\protobuf\Debug\protoc.exe  -I="." --grpc_out="." --plugin=protoc-gen-grpc="D:\cppsoft\grpc\visualpro\Debug\grpc_cpp_plugin.exe" "message.proto"
```
这俩命令执行完成后总计生成四个文件

![https://cdn.llfc.club/1713239066360.jpg](https://cdn.llfc.club/1713239066360.jpg)

实现StatusGrpcClient
``` cpp
#include "const.h"
#include "Singleton.h"
#include "ConfigMgr.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::StatusService;

class StatusGrpcClient :public Singleton<StatusGrpcClient>
{
	friend class Singleton<StatusGrpcClient>;
public:
	~StatusGrpcClient() {

	}
	GetChatServerRsp GetChatServer(int uid);

private:
	StatusGrpcClient();
	std::unique_ptr<StatusConPool> pool_;
	
};
```
具体实现
``` cpp
#include "StatusGrpcClient.h"

GetChatServerRsp StatusGrpcClient::GetChatServer(int uid)
{
	ClientContext context;
	GetChatServerRsp reply;
	GetChatServerReq request;
	request.set_uid(uid);
	auto stub = pool_->getConnection();
	Status status = stub->GetChatServer(&context, request, &reply);
	Defer defer([&stub, this]() {
		pool_->returnConnection(std::move(stub));
		});
	if (status.ok()) {	
		return reply;
	}
	else {
		reply.set_error(ErrorCodes::RPCFailed);
		return reply;
	}
}

StatusGrpcClient::StatusGrpcClient()
{
	auto& gCfgMgr = ConfigMgr::Inst();
	std::string host = gCfgMgr["StatusServer"]["Host"];
	std::string port = gCfgMgr["StatusServer"]["Port"];
	pool_.reset(new StatusConPool(5, host, port));
}
```
当然GateServer的config.ini文件也要做更新
``` ini
[GateServer]
Port = 8080
[VarifyServer]
Host = 127.0.0.1
Port = 50051
[StatusServer]
Host = 127.0.0.1
Port = 50052
[Mysql]
Host = 81.68.86.146
Port = 3308
User = root
Passwd = 123456
Schema = llfc
[Redis]
Host = 81.68.86.146
Port = 6380
Passwd = 123456
```
StatusGrpcClient用到了StatusConPool, 将其实现放在StatusGrpcClient类之上

``` cpp
class StatusConPool {
public:
	StatusConPool(size_t poolSize, std::string host, std::string port)
		: poolSize_(poolSize), host_(host), port_(port), b_stop_(false) {
		for (size_t i = 0; i < poolSize_; ++i) {

			std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port,
				grpc::InsecureChannelCredentials());

			connections_.push(StatusService::NewStub(channel));
		}
	}

	~StatusConPool() {
		std::lock_guard<std::mutex> lock(mutex_);
		Close();
		while (!connections_.empty()) {
			connections_.pop();
		}
	}

	std::unique_ptr<StatusService::Stub> getConnection() {
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

	void returnConnection(std::unique_ptr<StatusService::Stub> context) {
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
	std::queue<std::unique_ptr<StatusService::Stub>> connections_;
	std::mutex mutex_;
	std::condition_variable cond_;
};
```
## StatusServer状态服务

我们要实现状态服务，主要是用来监听其他服务器的查询请求, 用visual studio创建项目，名字为StatusServer.

在主函数所在文件StatusServer.cpp中实现如下逻辑
``` cpp
#include <iostream>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "const.h"
#include "ConfigMgr.h"
#include "hiredis.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"
#include "AsioIOServicePool.h"
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <boost/asio.hpp>
#include "StatusServiceImpl.h"
void RunServer() {
	auto & cfg = ConfigMgr::Inst();
	
	std::string server_address(cfg["StatusServer"]["Host"]+":"+ cfg["StatusServer"]["Port"]);
	StatusServiceImpl service;

	grpc::ServerBuilder builder;
	// 监听端口和添加服务
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);

	// 构建并启动gRPC服务器
	std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;

	// 创建Boost.Asio的io_context
	boost::asio::io_context io_context;
	// 创建signal_set用于捕获SIGINT
	boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);

	// 设置异步等待SIGINT信号
	signals.async_wait([&server](const boost::system::error_code& error, int signal_number) {
		if (!error) {
			std::cout << "Shutting down server..." << std::endl;
			server->Shutdown(); // 优雅地关闭服务器
		}
		});

	// 在单独的线程中运行io_context
	std::thread([&io_context]() { io_context.run(); }).detach();

	// 等待服务器关闭
	server->Wait();
	io_context.stop(); // 停止io_context
}

int main(int argc, char** argv) {
	try {
		RunServer();
	}
	catch (std::exception const& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return 0;
}

```
在开始逻辑之前，我们需要先更新下config.ini文件
``` ini
[StatusServer]
Port = 50052
Host = 0.0.0.0
[Mysql]
Host = 81.68.86.146
Port = 3308
User = root
Passwd = 123456
Schema = llfc
[Redis]
Host = 81.68.86.146
Port = 6380
Passwd = 123456
[ChatServer1]
Host = 127.0.0.1
Port = 8090
[ChatServer2]
Host = 127.0.0.1
Port = 8091
```
然后我们将GateServer之前生成的pb文件和proto文件拷贝到StatusServer中。并且加入到项目中。

我们在项目中添加一个新的类StatusServiceImpl，该类主要继承自StatusService::Service。
``` cpp
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::StatusService;

struct ChatServer {
	std::string host;
	std::string port;
};
class StatusServiceImpl final : public StatusService::Service
{
public:
	StatusServiceImpl();
	Status GetChatServer(ServerContext* context, const GetChatServerReq* request,
		GetChatServerRsp* reply) override;

	std::vector<ChatServer> _servers;
	int _server_index;
};

```
具体实现
``` cpp
#include "StatusServiceImpl.h"
#include "ConfigMgr.h"
#include "const.h"

std::string generate_unique_string() {
	// 创建UUID对象
	boost::uuids::uuid uuid = boost::uuids::random_generator()();

	// 将UUID转换为字符串
	std::string unique_string = to_string(uuid);

	return unique_string;
}

Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* reply)
{
	std::string prefix("llfc status server has received :  ");
	_server_index = (_server_index++) % (_servers.size());
	auto &server = _servers[_server_index];
	reply->set_host(server.host);
	reply->set_port(server.port);
	reply->set_error(ErrorCodes::Success);
	reply->set_token(generate_unique_string());
	return Status::OK;
}

StatusServiceImpl::StatusServiceImpl():_server_index(0)
{
	auto& cfg = ConfigMgr::Inst();
	ChatServer server;
	server.port = cfg["ChatServer1"]["Port"];
	server.host = cfg["ChatServer1"]["Host"];
	_servers.push_back(server);

	server.port = cfg["ChatServer2"]["Port"];
	server.host = cfg["ChatServer2"]["Host"];
	_servers.push_back(server);
}
```
其余的文件为了保持复用，我们不重复开发，将GateServer中的RedisMgr,MysqlMgr,Singleton,IOSerivePool等统统拷贝过来并添加到项目中。

## 联调测试
我们启动StatusServer，GateServer以及QT客户端，输入密码和用户名，点击登陆，会看到前端收到登陆成功的消息

![https://cdn.llfc.club/1713248019373.jpg](https://cdn.llfc.club/1713248019373.jpg)