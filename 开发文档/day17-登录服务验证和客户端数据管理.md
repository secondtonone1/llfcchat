## 完善proto
在proto文件里新增登陆验证服务
``` proto
message LoginReq{
	int32 uid = 1;
	string token= 2;
}

message LoginRsp {
	int32 error = 1;
	int32 uid = 2;
	string token = 3;
}

service StatusService {
	rpc GetChatServer (GetChatServerReq) returns (GetChatServerRsp) {}
	rpc Login(LoginReq) returns(LoginRsp);
}
```
接下来是调用grpc命令生成新的pb文件覆盖原有的，并且也拷贝给StatusServer一份

我们完善登陆逻辑，先去StatusServer验证token是否合理，如果合理再从内存中寻找用户信息，如果没找到则从数据库加载一份。
``` cpp
void LogicSystem::LoginHandler(shared_ptr<CSession> session, const short &msg_id, const string &msg_data) {
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid = root["uid"].asInt();
	std::cout << "user login uid is  " << uid << " user token  is "
		<< root["token"].asString() << endl;
	//从状态服务器获取token匹配是否准确
	auto rsp = StatusGrpcClient::GetInstance()->Login(uid, root["token"].asString());
	Json::Value  rtvalue;
	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, MSG_CHAT_LOGIN_RSP);
	});

	rtvalue["error"] = rsp.error();
	if (rsp.error() != ErrorCodes::Success) {
		return;
	}

	//内存中查询用户信息
	auto find_iter = _users.find(uid);
	std::shared_ptr<UserInfo> user_info = nullptr;
	if (find_iter == _users.end()) {
		//查询数据库
		user_info = MysqlMgr::GetInstance()->GetUser(uid);
		if (user_info == nullptr) {
			rtvalue["error"] = ErrorCodes::UidInvalid;
			return;
		}

		_users[uid] = user_info;
	}
	else {
		user_info = find_iter->second;
	}

	rtvalue["uid"] = uid;
	rtvalue["token"] = rsp.token();
	rtvalue["name"] = user_info->name;
}
```
## StatusServer验证token

在StatusServer验证token之前，我们需要在StatusServer中的GetServer的服务里将token写入内存
``` cpp
Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* reply)
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
接下来我们实现登陆验证服务
``` cpp
Status StatusServiceImpl::Login(ServerContext* context, const LoginReq* request, LoginRsp* reply)
{
	auto uid = request->uid();
	auto token = request->token();
	std::lock_guard<std::mutex> guard(_token_mtx);
	auto iter = _tokens.find(uid);
	if (iter == _tokens.end()) {
		reply->set_error(ErrorCodes::UidInvalid);
		return Status::OK;
	}
	if (iter->second != token) {
		reply->set_error(ErrorCodes::TokenInvalid);
		return Status::OK;
	}
	reply->set_error(ErrorCodes::Success);
	reply->set_uid(uid);
	reply->set_token(token);
	return Status::OK;
}
```
这样当GateServer访问StatusServer的Login服务做验证后，就可以将数据返回给QT前端了。
## 客户端处理登陆回包

QT 的客户端TcpMgr收到请求后要进行对应的逻辑处理。所以我们在TcpMgr的构造函数中调用initHandlers注册消息
``` cpp
void TcpMgr::initHandlers()
{
    //auto self = shared_from_this();
    _handlers.insert(ID_CHAT_LOGIN_RSP, [this](ReqId id, int len, QByteArray data){
        qDebug()<< "handle id is "<< id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if(jsonDoc.isNull()){
           qDebug() << "Failed to create QJsonDocument.";
           return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        if(!jsonObj.contains("error")){
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Login Failed, err is Json Parse Err" << err ;
            emit sig_login_failed(err);
            return;
        }

        int err = jsonObj["error"].toInt();
        if(err != ErrorCodes::SUCCESS){
            qDebug() << "Login Failed, err is " << err ;
            emit sig_login_failed(err);
            return;
        }

        UserMgr::GetInstance()->SetUid(jsonObj["uid"].toInt());
        UserMgr::GetInstance()->SetName(jsonObj["name"].toString());
        UserMgr::GetInstance()->SetToken(jsonObj["token"].toString());
        emit sig_swich_chatdlg();
    });
}
```
并且增加处理请求
``` cpp
void TcpMgr::handleMsg(ReqId id, int len, QByteArray data)
{
   auto find_iter =  _handlers.find(id);
   if(find_iter == _handlers.end()){
        qDebug()<< "not found id ["<< id << "] to handle";
        return ;
   }

   find_iter.value()(id,len,data);
}
```
## 用户管理
为管理用户数据，需要创建一个UserMgr类，统一管理用户数据，我们这么声明
``` cpp
#ifndef USERMGR_H
#define USERMGR_H
#include <QObject>
#include <memory>
#include <singleton.h>

class UserMgr:public QObject,public Singleton<UserMgr>,
        public std::enable_shared_from_this<UserMgr>
{
    Q_OBJECT
public:
    friend class Singleton<UserMgr>;
    ~ UserMgr();
    void SetName(QString name);
    void SetUid(int uid);
    void SetToken(QString token);
private:
    UserMgr();
    QString _name;
    QString _token;
    int _uid;
};

#endif // USERMGR_H
```
简单实现几个功能
``` cpp
#include "usermgr.h"

UserMgr::~UserMgr()
{

}

void UserMgr::SetName(QString name)
{
    _name = name;
}

void UserMgr::SetUid(int uid)
{
    _uid = uid;
}

void UserMgr::SetToken(QString token)
{
    _token = token;
}

UserMgr::UserMgr()
{

}
```
详细和复杂的管理后续不断往这里补充就行了。
## 登陆界面
登陆界面响应TcpMgr返回的登陆请求，在其构造函数中添加
``` cpp
   //连接tcp管理者发出的登陆失败信号
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_login_failed, this, &LoginDialog::slot_login_failed);
```
并实现槽函数
``` cpp
void LoginDialog::slot_login_failed(int err)
{
    QString result = QString("登录失败, err is %1")
                             .arg(err);
    showTip(result,false);
    enableBtn(true);
}
```
到此完成了登陆的请求和响应，接下来要实现响应登陆成功后跳转到聊天界面。下一篇先实现聊天布局。