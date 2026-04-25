#pragma once
#include "const.h"
#include "MysqlDao.h"
#include "Singleton.h"
#include <vector>
#include "message.pb.h"
#include "FileInfo.h"

class MysqlMgr: public Singleton<MysqlMgr>
{
	friend class Singleton<MysqlMgr>;
public:
	~MysqlMgr();
	int RegUser(const std::string& name, const std::string& email,  const std::string& pwd);
	bool CheckEmail(const std::string& name, const std::string & email);
	bool UpdatePwd(const std::string& name, const std::string& email);
	bool CheckPwd(const std::string& name, const std::string& pwd, UserInfo& userInfo);
	bool AddFriendApply(const int& from, const int& to, const std::string& desc, const std::string& back_name);
	bool AuthFriendApply(const int& from, const int& to);
	bool AddFriend(const int& from, const int& to, std::string back_name, std::vector<std::shared_ptr<AddFriendMsg>>& msg_list);
	std::shared_ptr<UserInfo> GetUser(int uid);
	std::shared_ptr<UserInfo> GetUser(std::string name);
	bool GetApplyList(int touid, std::vector<std::shared_ptr<ApplyInfo>>& applyList, int begin, int limit=10);
	bool GetFriendList(int self_id, std::vector<std::shared_ptr<UserInfo> >& user_info);
	bool GetUserThreads(int64_t userId,
		int64_t lastId,
		int      pageSize,
		std::vector<std::shared_ptr<ChatThreadInfo>>& threads,
		bool& loadMore,
		int& nextLastId);

	bool CreatePrivateChat(int user1_id, int user2_id, int &thread_id);
	std::shared_ptr<PageResult> LoadChatMsg(int threadId, int lastId, int pageSize);
	bool AddChatMsg(std::vector<std::shared_ptr<ChatMessage>>& chat_datas);
	bool UpdateUserIcon(int uid, const std::string& icon);
	bool UpdateUploadStatus(int chat_messag_id);
	std::shared_ptr<ChatImgInfo> GetImgInfoByMsgId(int msg_id);
	std::shared_ptr<ChatMessage> GetChatMsgById(int message_id);
private:
	MysqlMgr();
	MysqlDao  _dao;
};

