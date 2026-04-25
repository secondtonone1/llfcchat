#pragma once
#include "Singleton.h"
#include <queue>
#include <thread>
#include "CSession.h"
#include <queue>
#include <map>
#include <functional>
#include "const.h"
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include <unordered_map>
#include "LogicWorker.h"
#include "FileInfo.h"


typedef  function<void(shared_ptr<CSession>, const short &msg_id, const string &msg_data)> FunCallBack;
class LogicSystem:public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem();
	void PostMsgToQue(shared_ptr < LogicNode> msg, int index);
	void AddMD5File(std::string md5, std::shared_ptr<FileInfo> fileinfo);
	std::shared_ptr<FileInfo> GetFileInfo(std::string md5);
private:
	LogicSystem();
	std::vector<std::shared_ptr<LogicWorker> > _workers;
	std::mutex _file_mtx;
	std::unordered_map<std::string, std::shared_ptr<FileInfo>> _map_md5_files;
};

