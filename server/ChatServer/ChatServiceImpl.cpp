#include "ChatServiceImpl.h"
#include "UserMgr.h"
#include "CSession.h"
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "RedisMgr.h"

ChatServiceImpl::ChatServiceImpl()
{

}

Status ChatServiceImpl::NotifyAddFriend(ServerContext* context, const AddFriendReq* request, AddFriendRsp* reply)
{
	//�����û��Ƿ��ڱ�������
	auto touid = request->touid();
	auto session = UserMgr::GetInstance()->GetSession(touid);

	Defer defer([request, reply]() {
		reply->set_error(ErrorCodes::Success);
		reply->set_applyuid(request->applyuid());
		reply->set_touid(request->touid());
		});

	//�û������ڴ�����ֱ�ӷ���
	if (session == nullptr) {
		return Status::OK;
	}
	
	//���ڴ�����ֱ�ӷ���֪ͨ�Է�
	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	rtvalue["applyuid"] = request->applyuid();
	rtvalue["name"] = request->name();
	rtvalue["desc"] = request->desc();

	std::string return_str = rtvalue.toStyledString();

	session->Send(return_str, ID_NOTIFY_ADD_FRIEND_REQ);
	return Status::OK;
}
