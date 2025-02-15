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
