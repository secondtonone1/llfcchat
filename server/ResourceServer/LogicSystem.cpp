#include "LogicSystem.h"
#include "const.h"
#include "base64.h"
#include <fstream>
#include <boost/filesystem.hpp>
#include "ConfigMgr.h"
#include "FileSystem.h"

using namespace std;

LogicSystem::LogicSystem(){
	for (int i = 0; i < LOGIC_WORKER_COUNT; i++) {
		_workers.push_back(std::make_shared<LogicWorker>());
	}
}

LogicSystem::~LogicSystem(){

}

void LogicSystem::PostMsgToQue(shared_ptr < LogicNode> msg, int index) {
	_workers[index]->PostTask(msg);
}


void LogicSystem::AddMD5File(std::string md5, std::shared_ptr<FileInfo> fileinfo) {
	std::lock_guard<std::mutex> lock(_file_mtx);
	_map_md5_files[md5] = fileinfo;
}

std::shared_ptr<FileInfo> LogicSystem::GetFileInfo(std::string md5) {
	std::lock_guard<std::mutex> lock(_file_mtx);
	auto iter = _map_md5_files.find(md5);
	if (iter == _map_md5_files.end()) {
		return nullptr;
	}

	return iter->second;
}






