## 好友认证

### 服务器响应

服务器接受客户端发送过来的好友认证请求

``` cpp
void LogicSystem::AuthFriendApply(std::shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
	
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);

	auto uid = root["fromuid"].asInt();
	auto touid = root["touid"].asInt();
	auto back_name = root["back"].asString();
	std::cout << "from " << uid << " auth friend to " << touid << std::endl;

	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	auto user_info = std::make_shared<UserInfo>();

	std::string base_key = USER_BASE_INFO + std::to_string(touid);
	bool b_info = GetBaseInfo(base_key, touid, user_info);
	if (b_info) {
		rtvalue["name"] = user_info->name;
		rtvalue["nick"] = user_info->nick;
		rtvalue["icon"] = user_info->icon;
		rtvalue["sex"] = user_info->sex;
		rtvalue["uid"] = touid;
	}
	else {
		rtvalue["error"] = ErrorCodes::UidInvalid;
	}


	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, ID_AUTH_FRIEND_RSP);
		});

	//先更新数据库
	MysqlMgr::GetInstance()->AuthFriendApply(uid, touid);

	//更新数据库添加好友
	MysqlMgr::GetInstance()->AddFriend(uid, touid,back_name);

	//查询redis 查找touid对应的server ip
	auto to_str = std::to_string(touid);
	auto to_ip_key = USERIPPREFIX + to_str;
	std::string to_ip_value = "";
	bool b_ip = RedisMgr::GetInstance()->Get(to_ip_key, to_ip_value);
	if (!b_ip) {
		return;
	}

	auto& cfg = ConfigMgr::Inst();
	auto self_name = cfg["SelfServer"]["Name"];
	//直接通知对方有认证通过消息
	if (to_ip_value == self_name) {
		auto session = UserMgr::GetInstance()->GetSession(touid);
		if (session) {
			//在内存中则直接发送通知对方
			Json::Value  notify;
			notify["error"] = ErrorCodes::Success;
			notify["fromuid"] = uid;
			notify["touid"] = touid;
			std::string base_key = USER_BASE_INFO + std::to_string(uid);
			auto user_info = std::make_shared<UserInfo>();
			bool b_info = GetBaseInfo(base_key, uid, user_info);
			if (b_info) {
				notify["name"] = user_info->name;
				notify["nick"] = user_info->nick;
				notify["icon"] = user_info->icon;
				notify["sex"] = user_info->sex;
			}
			else {
				notify["error"] = ErrorCodes::UidInvalid;
			}


			std::string return_str = notify.toStyledString();
			session->Send(return_str, ID_NOTIFY_AUTH_FRIEND_REQ);
		}

		return ;
	}


	AuthFriendReq auth_req;
	auth_req.set_fromuid(uid);
	auth_req.set_touid(touid);

	//发送通知
	ChatGrpcClient::GetInstance()->NotifyAuthFriend(to_ip_value, auth_req);
}

```

将请求注册到map里，在`LogicSystem::RegisterCallBacks`中添加

``` cpp
_fun_callbacks[ID_AUTH_FRIEND_REQ] = std::bind(&LogicSystem::AuthFriendApply, this,
    placeholders::_1, placeholders::_2, placeholders::_3);
```

因为上面的逻辑调用了grpc发送通知，所以实现grpc发送认证通知的逻辑

``` cpp
AuthFriendRsp ChatGrpcClient::NotifyAuthFriend(std::string server_ip, const AuthFriendReq& req) {
	AuthFriendRsp rsp;
	rsp.set_error(ErrorCodes::Success);

	Defer defer([&rsp, &req]() {
		rsp.set_fromuid(req.fromuid());
		rsp.set_touid(req.touid());
		});

	auto find_iter = _pools.find(server_ip);
	if (find_iter == _pools.end()) {
		return rsp;
	}

	auto& pool = find_iter->second;
	ClientContext context;
	auto stub = pool->getConnection();
	Status status = stub->NotifyAuthFriend(&context, req, &rsp);
	Defer defercon([&stub, this, &pool]() {
		pool->returnConnection(std::move(stub));
		});

	if (!status.ok()) {
		rsp.set_error(ErrorCodes::RPCFailed);
		return rsp;
	}

	return rsp;
}
```

这里注意，stub之所以能发送通知，是因为proto里定义了认证通知等服务，大家记得更新proto和我的一样，这事完整的proto

``` cpp
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

message AddFriendReq {
	int32  applyuid = 1;
	string name = 2;
	string desc = 3;
	string icon = 4;
	string nick = 5;
	int32  sex = 6;
	int32  touid = 7;
}

message AddFriendRsp {
	int32 error = 1;
	int32 applyuid = 2;
	int32 touid = 3;
}

message RplyFriendReq {
	int32 rplyuid = 1;
	bool  agree = 2;
	int32 touid = 3;
}

message RplyFriendRsp {
	int32 error = 1;
	int32 rplyuid = 2;
	int32 touid = 3;
}

message SendChatMsgReq{
		int32 fromuid = 1;
		int32 touid = 2;
		string message = 3;
}

message SendChatMsgRsp{
		int32 error = 1;
		int32 fromuid = 2;
		int32 touid = 3;
}

message AuthFriendReq{
	int32 fromuid = 1;
	int32 touid = 2;
}

message AuthFriendRsp{
	int32 error = 1;
	int32 fromuid = 2;
	int32 touid = 3;
}

message TextChatMsgReq {
	int32 fromuid = 1;
    int32 touid = 2;
	repeated TextChatData textmsgs = 3;
}

message TextChatData{
	string msgid = 1;
	string msgcontent = 2;
}

message TextChatMsgRsp {
	int32 error = 1;
	int32 fromuid = 2;
	int32 touid = 3; 
	repeated TextChatData textmsgs = 4;
}

service ChatService {
	rpc NotifyAddFriend(AddFriendReq) returns (AddFriendRsp) {}
	rpc RplyAddFriend(RplyFriendReq) returns (RplyFriendRsp) {}
	rpc SendChatMsg(SendChatMsgReq) returns (SendChatMsgRsp) {}
	rpc NotifyAuthFriend(AuthFriendReq) returns (AuthFriendRsp) {}
	rpc NotifyTextChatMsg(TextChatMsgReq) returns (TextChatMsgRsp){}
}

```

为了方便生成grpcpb文件，我写了一个start.bat批处理文件

``` bat
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

执行这个批处理文件就能生成最新的pb文件了。

接下来实现grpc服务对认证的处理

``` cpp
Status ChatServiceImpl::NotifyAuthFriend(ServerContext* context, const AuthFriendReq* request,
	AuthFriendRsp* reply) {
	//查找用户是否在本服务器
	auto touid = request->touid();
	auto fromuid = request->fromuid();
	auto session = UserMgr::GetInstance()->GetSession(touid);

	Defer defer([request, reply]() {
		reply->set_error(ErrorCodes::Success);
		reply->set_fromuid(request->fromuid());
		reply->set_touid(request->touid());
		});

	//用户不在内存中则直接返回
	if (session == nullptr) {
		return Status::OK;
	}

	//在内存中则直接发送通知对方
	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	rtvalue["fromuid"] = request->fromuid();
	rtvalue["touid"] = request->touid();

	std::string base_key = USER_BASE_INFO + std::to_string(fromuid);
	auto user_info = std::make_shared<UserInfo>();
	bool b_info = GetBaseInfo(base_key, fromuid, user_info);
	if (b_info) {
		rtvalue["name"] = user_info->name;
		rtvalue["nick"] = user_info->nick;
		rtvalue["icon"] = user_info->icon;
		rtvalue["sex"] = user_info->sex;
	}
	else {
		rtvalue["error"] = ErrorCodes::UidInvalid;
	}

	std::string return_str = rtvalue.toStyledString();

	session->Send(return_str, ID_NOTIFY_AUTH_FRIEND_REQ);
	return Status::OK;
}
```

所以A认证B为好友，A所在的服务器会给A回复一个ID_AUTH_FRIEND_RSP的消息，B所在的服务器会给B回复一个ID_NOTIFY_AUTH_FRIEND_REQ消息。

### 客户端响应

客户端需要响应服务器发过来的ID_AUTH_FRIEND_RSP和ID_NOTIFY_AUTH_FRIEND_REQ消息

客户端响应ID_AUTH_FRIEND_RSP，在initHandlers中添加

``` cpp
_handlers.insert(ID_AUTH_FRIEND_RSP, [this](ReqId id, int len, QByteArray data) {
    Q_UNUSED(len);
    qDebug() << "handle id is " << id << " data is " << data;
    // 将QByteArray转换为QJsonDocument
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

    // 检查转换是否成功
    if (jsonDoc.isNull()) {
        qDebug() << "Failed to create QJsonDocument.";
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();

    if (!jsonObj.contains("error")) {
        int err = ErrorCodes::ERR_JSON;
        qDebug() << "Auth Friend Failed, err is Json Parse Err" << err;
        return;
    }

    int err = jsonObj["error"].toInt();
    if (err != ErrorCodes::SUCCESS) {
        qDebug() << "Auth Friend Failed, err is " << err;
        return;
    }

    auto name = jsonObj["name"].toString();
    auto nick = jsonObj["nick"].toString();
    auto icon = jsonObj["icon"].toString();
    auto sex = jsonObj["sex"].toInt();
    auto uid = jsonObj["uid"].toInt();
    auto rsp = std::make_shared<AuthRsp>(uid, name, nick, icon, sex);
    emit sig_auth_rsp(rsp);

    qDebug() << "Auth Friend Success " ;
  });
```

在initHandlers中添加ID_NOTIFY_AUTH_FRIEND_REQ

``` cpp
_handlers.insert(ID_NOTIFY_AUTH_FRIEND_REQ, [this](ReqId id, int len, QByteArray data) {
    Q_UNUSED(len);
    qDebug() << "handle id is " << id << " data is " << data;
    // 将QByteArray转换为QJsonDocument
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

    // 检查转换是否成功
    if (jsonDoc.isNull()) {
        qDebug() << "Failed to create QJsonDocument.";
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    if (!jsonObj.contains("error")) {
        int err = ErrorCodes::ERR_JSON;
        qDebug() << "Auth Friend Failed, err is " << err;
        return;
    }

    int err = jsonObj["error"].toInt();
    if (err != ErrorCodes::SUCCESS) {
        qDebug() << "Auth Friend Failed, err is " << err;
        return;
    }

    int from_uid = jsonObj["fromuid"].toInt();
    QString name = jsonObj["name"].toString();
    QString nick = jsonObj["nick"].toString();
    QString icon = jsonObj["icon"].toString();
    int sex = jsonObj["sex"].toInt();

    auto auth_info = std::make_shared<AuthInfo>(from_uid,name,
                                                nick, icon, sex);

    emit sig_add_auth_friend(auth_info);
    });
```

客户端ChatDialog中添加对sig_add_auth_friend响应，实现添加好友到聊天列表中

``` cpp
void ChatDialog::slot_add_auth_friend(std::shared_ptr<AuthInfo> auth_info) {
    qDebug() << "receive slot_add_auth__friend uid is " << auth_info->_uid
        << " name is " << auth_info->_name << " nick is " << auth_info->_nick;

    //判断如果已经是好友则跳过
    auto bfriend = UserMgr::GetInstance()->CheckFriendById(auth_info->_uid);
    if(bfriend){
        return;
    }

    UserMgr::GetInstance()->AddFriend(auth_info);

    int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
    int str_i = randomValue % strs.size();
    int head_i = randomValue % heads.size();
    int name_i = randomValue % names.size();

    auto* chat_user_wid = new ChatUserWid();
    auto user_info = std::make_shared<UserInfo>(auth_info);
    chat_user_wid->SetInfo(user_info);
    QListWidgetItem* item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);
    _chat_items_added.insert(auth_info->_uid, item);
}
```

客户端ChatDialog中添加对sig_auth_rsp响应, 实现添加好友到聊天列表中

``` cpp
void ChatDialog::slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp)
{
    qDebug() << "receive slot_auth_rsp uid is " << auth_rsp->_uid
        << " name is " << auth_rsp->_name << " nick is " << auth_rsp->_nick;

    //判断如果已经是好友则跳过
    auto bfriend = UserMgr::GetInstance()->CheckFriendById(auth_rsp->_uid);
    if(bfriend){
        return;
    }

    UserMgr::GetInstance()->AddFriend(auth_rsp);
    int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
    int str_i = randomValue % strs.size();
    int head_i = randomValue % heads.size();
    int name_i = randomValue % names.size();

    auto* chat_user_wid = new ChatUserWid();
    auto user_info = std::make_shared<UserInfo>(auth_rsp);
    chat_user_wid->SetInfo(user_info);
    QListWidgetItem* item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);
    _chat_items_added.insert(auth_rsp->_uid, item);
}
```

因为认证对方为好友后，需要将申请页面的添加按钮变成已添加，所以ApplyFriendPage响应sig_auth_rsp信号

``` cpp
void ApplyFriendPage::slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp) {
    auto uid = auth_rsp->_uid;
    auto find_iter = _unauth_items.find(uid);
    if (find_iter == _unauth_items.end()) {
        return;
    }

    find_iter->second->ShowAddBtn(false);
}
```

同意并认证对方为好友后，也需要将对方添加到联系人列表，ContactUserList响应sig_auth_rsp信号

``` cpp
void ContactUserList::slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp)
{
    qDebug() << "slot auth rsp called";
    bool isFriend = UserMgr::GetInstance()->CheckFriendById(auth_rsp->_uid);
    if(isFriend){
        return;
    }
    // 在 groupitem 之后插入新项
    int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
    int str_i = randomValue%strs.size();
    int head_i = randomValue%heads.size();

    auto *con_user_wid = new ConUserItem();
    con_user_wid->SetInfo(auth_rsp->_uid ,auth_rsp->_name, heads[head_i]);
    QListWidgetItem *item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(con_user_wid->sizeHint());

    // 获取 groupitem 的索引
    int index = this->row(_groupitem);
    // 在 groupitem 之后插入新项
    this->insertItem(index + 1, item);

    this->setItemWidget(item, con_user_wid);

}
```

### 登录加载好友

因为添加好友后，如果客户端重新登录，服务器LoginHandler需要加载好友列表，所以服务器要返回好友列表

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

	return;
}
```

客户端在initHandlers中加载聊天列表

``` cpp
_handlers.insert(ID_CHAT_LOGIN_RSP, [this](ReqId id, int len, QByteArray data){
    Q_UNUSED(len);
    qDebug()<< "handle id is "<< id ;
    // 将QByteArray转换为QJsonDocument
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

    // 检查转换是否成功
    if(jsonDoc.isNull()){
       qDebug() << "Failed to create QJsonDocument.";
       return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    qDebug()<< "data jsonobj is " << jsonObj ;

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

    auto uid = jsonObj["uid"].toInt();
    auto name = jsonObj["name"].toString();
    auto nick = jsonObj["nick"].toString();
    auto icon = jsonObj["icon"].toString();
    auto sex = jsonObj["sex"].toInt();
    auto user_info = std::make_shared<UserInfo>(uid, name, nick, icon, sex);

    UserMgr::GetInstance()->SetUserInfo(user_info);
    UserMgr::GetInstance()->SetToken(jsonObj["token"].toString());
    if(jsonObj.contains("apply_list")){
        UserMgr::GetInstance()->AppendApplyList(jsonObj["apply_list"].toArray());
    }

    //添加好友列表
    if (jsonObj.contains("friend_list")) {
        UserMgr::GetInstance()->AppendFriendList(jsonObj["friend_list"].toArray());
    }

    emit sig_swich_chatdlg();
});
```

## 好友聊天

### 客户端发送聊天消息

客户端发送聊天消息，在输入框输入消息后，点击发送回执行下面的槽函数

``` cpp
void ChatPage::on_send_btn_clicked()
{
    if (_user_info == nullptr) {
        qDebug() << "friend_info is empty";
        return;
    }

    auto user_info = UserMgr::GetInstance()->GetUserInfo();
    auto pTextEdit = ui->chatEdit;
    ChatRole role = ChatRole::Self;
    QString userName = user_info->_name;
    QString userIcon = user_info->_icon;

    const QVector<MsgInfo>& msgList = pTextEdit->getMsgList();
    QJsonObject textObj;
    QJsonArray textArray;
    int txt_size = 0;

    for(int i=0; i<msgList.size(); ++i)
    {
        //消息内容长度不合规就跳过
        if(msgList[i].content.length() > 1024){
            continue;
        }

        QString type = msgList[i].msgFlag;
        ChatItemBase *pChatItem = new ChatItemBase(role);
        pChatItem->setUserName(userName);
        pChatItem->setUserIcon(QPixmap(userIcon));
        QWidget *pBubble = nullptr;

        if(type == "text")
        {   
            //生成唯一id
            QUuid uuid = QUuid::createUuid();
            //转为字符串
            QString uuidString = uuid.toString();

            pBubble = new TextBubble(role, msgList[i].content);
            if(txt_size + msgList[i].content.length()> 1024){
                textObj["fromuid"] = user_info->_uid;
                textObj["touid"] = _user_info->_uid;
                textObj["text_array"] = textArray;
                QJsonDocument doc(textObj);
                QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
                //发送并清空之前累计的文本列表
                txt_size = 0;
                textArray = QJsonArray();
                textObj = QJsonObject();
                //发送tcp请求给chat server
                emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ, jsonData);
            }

            //将bubble和uid绑定，以后可以等网络返回消息后设置是否送达
            //_bubble_map[uuidString] = pBubble;
            txt_size += msgList[i].content.length();
            QJsonObject obj;
            QByteArray utf8Message = msgList[i].content.toUtf8();
            obj["content"] = QString::fromUtf8(utf8Message);
            obj["msgid"] = uuidString;
            textArray.append(obj);
            auto txt_msg = std::make_shared<TextChatData>(uuidString, obj["content"].toString(),
                user_info->_uid, _user_info->_uid);
            emit sig_append_send_chat_msg(txt_msg);
        }
        else if(type == "image")
        {
             pBubble = new PictureBubble(QPixmap(msgList[i].content) , role);
        }
        else if(type == "file")
        {

        }
        //发送消息
        if(pBubble != nullptr)
        {
            pChatItem->setWidget(pBubble);
            ui->chat_data_list->appendChatItem(pChatItem);
        }

    }

    qDebug() << "textArray is " << textArray ;
    //发送给服务器
    textObj["text_array"] = textArray;
    textObj["fromuid"] = user_info->_uid;
    textObj["touid"] = _user_info->_uid;
    QJsonDocument doc(textObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    //发送并清空之前累计的文本列表
    txt_size = 0;
    textArray = QJsonArray();
    textObj = QJsonObject();
    //发送tcp请求给chat server
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ, jsonData);
}
```

TcpMgr响应发送信号

``` cpp
void TcpMgr::slot_send_data(ReqId reqId, QByteArray dataBytes)
{
    uint16_t id = reqId;

    // 计算长度（使用网络字节序转换）
    quint16 len = static_cast<quint16>(dataBytes.length());

    // 创建一个QByteArray用于存储要发送的所有数据
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    // 设置数据流使用网络字节序
    out.setByteOrder(QDataStream::BigEndian);

    // 写入ID和长度
    out << id << len;

    // 添加字符串数据
    block.append(dataBytes);

    // 发送数据
    _socket.write(block);
    qDebug() << "tcp mgr send byte data is " << block ;
}
```

###  服务器响应

服务器响应客户端发送过来文本消息，在initHandlers中添加处理文本消息的逻辑

``` cpp
void LogicSystem::DealChatTextMsg(std::shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);

	auto uid = root["fromuid"].asInt();
	auto touid = root["touid"].asInt();

	const Json::Value  arrays = root["text_array"];
	
	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	rtvalue["text_array"] = arrays;
	rtvalue["fromuid"] = uid;
	rtvalue["touid"] = touid;

	Defer defer([this, &rtvalue, session]() {
		std::string return_str = rtvalue.toStyledString();
		session->Send(return_str, ID_TEXT_CHAT_MSG_RSP);
		});


	//查询redis 查找touid对应的server ip
	auto to_str = std::to_string(touid);
	auto to_ip_key = USERIPPREFIX + to_str;
	std::string to_ip_value = "";
	bool b_ip = RedisMgr::GetInstance()->Get(to_ip_key, to_ip_value);
	if (!b_ip) {
		return;
	}

	auto& cfg = ConfigMgr::Inst();
	auto self_name = cfg["SelfServer"]["Name"];
	//直接通知对方有认证通过消息
	if (to_ip_value == self_name) {
		auto session = UserMgr::GetInstance()->GetSession(touid);
		if (session) {
			//在内存中则直接发送通知对方
			std::string return_str = rtvalue.toStyledString();
			session->Send(return_str, ID_NOTIFY_TEXT_CHAT_MSG_REQ);
		}

		return ;
	}


	TextChatMsgReq text_msg_req;
	text_msg_req.set_fromuid(uid);
	text_msg_req.set_touid(touid);
	for (const auto& txt_obj : arrays) {
		auto content = txt_obj["content"].asString();
		auto msgid = txt_obj["msgid"].asString();
		std::cout << "content is " << content << std::endl;
		std::cout << "msgid is " << msgid << std::endl;
		auto *text_msg = text_msg_req.add_textmsgs();
		text_msg->set_msgid(msgid);
		text_msg->set_msgcontent(content);
	}


	//发送通知 todo...
	ChatGrpcClient::GetInstance()->NotifyTextChatMsg(to_ip_value, text_msg_req, rtvalue);
}
```

服务器实现发送消息的rpc客户端

``` cpp
TextChatMsgRsp ChatGrpcClient::NotifyTextChatMsg(std::string server_ip, 
	const TextChatMsgReq& req, const Json::Value& rtvalue) {
	
	TextChatMsgRsp rsp;
	rsp.set_error(ErrorCodes::Success);

	Defer defer([&rsp, &req]() {
		rsp.set_fromuid(req.fromuid());
		rsp.set_touid(req.touid());
		for (const auto& text_data : req.textmsgs()) {
			TextChatData* new_msg = rsp.add_textmsgs();
			new_msg->set_msgid(text_data.msgid());
			new_msg->set_msgcontent(text_data.msgcontent());
		}
		
		});

	auto find_iter = _pools.find(server_ip);
	if (find_iter == _pools.end()) {
		return rsp;
	}

	auto& pool = find_iter->second;
	ClientContext context;
	auto stub = pool->getConnection();
	Status status = stub->NotifyTextChatMsg(&context, req, &rsp);
	Defer defercon([&stub, this, &pool]() {
		pool->returnConnection(std::move(stub));
		});

	if (!status.ok()) {
		rsp.set_error(ErrorCodes::RPCFailed);
		return rsp;
	}

	return rsp;
}
```

服务器实现rpc服务端处理消息通知

``` cpp
Status ChatServiceImpl::NotifyTextChatMsg(::grpc::ServerContext* context,
	const TextChatMsgReq* request, TextChatMsgRsp* reply) {
	//查找用户是否在本服务器
	auto touid = request->touid();
	auto session = UserMgr::GetInstance()->GetSession(touid);
	reply->set_error(ErrorCodes::Success);

	//用户不在内存中则直接返回
	if (session == nullptr) {
		return Status::OK;
	}

	//在内存中则直接发送通知对方
	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	rtvalue["fromuid"] = request->fromuid();
	rtvalue["touid"] = request->touid();

	//将聊天数据组织为数组
	Json::Value text_array;
	for (auto& msg : request->textmsgs()) {
		Json::Value element;
		element["content"] = msg.msgcontent();
		element["msgid"] = msg.msgid();
		text_array.append(element);
	}
	rtvalue["text_array"] = text_array;

	std::string return_str = rtvalue.toStyledString();

	session->Send(return_str, ID_NOTIFY_TEXT_CHAT_MSG_REQ);
	return Status::OK;
}
```

### 客户端响应通知

客户端响应服务器返回的消息，包括两种：

1. A给B发送文本消息，A所在的服务器会给A发送ID_TEXT_CHAT_MSG_RSP消息。
2. B所在的服务器会通知B，告诉B有来自A的消息，通知消息为ID_NOTIFY_TEXT_CHAT_MSG_REQ

所以在tcpmgr的initHandlers中添加响应ID_TEXT_CHAT_MSG_RSP消息

``` cpp
_handlers.insert(ID_TEXT_CHAT_MSG_RSP, [this](ReqId id, int len, QByteArray data) {
    Q_UNUSED(len);
    qDebug() << "handle id is " << id << " data is " << data;
    // 将QByteArray转换为QJsonDocument
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

    // 检查转换是否成功
    if (jsonDoc.isNull()) {
        qDebug() << "Failed to create QJsonDocument.";
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();

    if (!jsonObj.contains("error")) {
        int err = ErrorCodes::ERR_JSON;
        qDebug() << "Chat Msg Rsp Failed, err is Json Parse Err" << err;
        return;
    }

    int err = jsonObj["error"].toInt();
    if (err != ErrorCodes::SUCCESS) {
        qDebug() << "Chat Msg Rsp Failed, err is " << err;
        return;
    }

    qDebug() << "Receive Text Chat Rsp Success " ;
    //ui设置送达等标记 todo...
  });
```

在TcpMgr的initHandlers中添加ID_NOTIFY_TEXT_CHAT_MSG_REQ

``` cpp
_handlers.insert(ID_NOTIFY_TEXT_CHAT_MSG_REQ, [this](ReqId id, int len, QByteArray data) {
    Q_UNUSED(len);
    qDebug() << "handle id is " << id << " data is " << data;
    // 将QByteArray转换为QJsonDocument
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

    // 检查转换是否成功
    if (jsonDoc.isNull()) {
        qDebug() << "Failed to create QJsonDocument.";
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();

    if (!jsonObj.contains("error")) {
        int err = ErrorCodes::ERR_JSON;
        qDebug() << "Notify Chat Msg Failed, err is Json Parse Err" << err;
        return;
    }

    int err = jsonObj["error"].toInt();
    if (err != ErrorCodes::SUCCESS) {
        qDebug() << "Notify Chat Msg Failed, err is " << err;
        return;
    }

    qDebug() << "Receive Text Chat Notify Success " ;
    auto msg_ptr = std::make_shared<TextChatMsg>(jsonObj["fromuid"].toInt(),
            jsonObj["touid"].toInt(),jsonObj["text_array"].toArray());
    emit sig_text_chat_msg(msg_ptr);
  });
```

客户端ChatDialog添加对sig_text_chat_msg的响应

``` cpp
void ChatDialog::slot_text_chat_msg(std::shared_ptr<TextChatMsg> msg)
{
    auto find_iter = _chat_items_added.find(msg->_from_uid);
    if(find_iter != _chat_items_added.end()){
        qDebug() << "set chat item msg, uid is " << msg->_from_uid;
        QWidget *widget = ui->chat_user_list->itemWidget(find_iter.value());
        auto chat_wid = qobject_cast<ChatUserWid*>(widget);
        if(!chat_wid){
            return;
        }
        chat_wid->updateLastMsg(msg->_chat_msgs);
        //更新当前聊天页面记录
        UpdateChatMsg(msg->_chat_msgs);
        UserMgr::GetInstance()->AppendFriendChatMsg(msg->_from_uid,msg->_chat_msgs);
        return;
    }

    //如果没找到，则创建新的插入listwidget

    auto* chat_user_wid = new ChatUserWid();
    //查询好友信息
    auto fi_ptr = UserMgr::GetInstance()->GetFriendById(msg->_from_uid);
    chat_user_wid->SetInfo(fi_ptr);
    QListWidgetItem* item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(chat_user_wid->sizeHint());
    chat_user_wid->updateLastMsg(msg->_chat_msgs);
     UserMgr::GetInstance()->AppendFriendChatMsg(msg->_from_uid,msg->_chat_msgs);
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);
    _chat_items_added.insert(msg->_from_uid, item);

}
```

## 效果展示

![https://cdn.llfc.club/1724470182274.jpg](https://cdn.llfc.club/1724470182274.jpg)



## 源码连接

[https://gitee.com/secondtonone1/llfcchat](https://gitee.com/secondtonone1/llfcchat)

## 视频连接

[https://www.bilibili.com/video/BV1ib421J745/?vd_source=8be9e83424c2ed2c9b2a3ed1d01385e9](https://www.bilibili.com/video/BV1ib421J745/?vd_source=8be9e83424c2ed2c9b2a3ed1d01385e9)

