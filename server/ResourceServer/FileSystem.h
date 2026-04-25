#pragma once
#include "Singleton.h"
#include "FileWorker.h"
#include <memory>
#include <vector>

class FileSystem :public Singleton<FileSystem>
{
	friend class Singleton<FileSystem>;
public:
	~FileSystem();
	void PostMsgToQue(shared_ptr <FileTask> msg, int index);
	void PostDownloadTaskToQue(std::shared_ptr<DownloadTask> msg, int index);
private:
	FileSystem();
	std::vector<std::shared_ptr<FileWorker>>  _file_workers;
	std::vector<std::shared_ptr<DownloadWorker>> _down_load_worker;
};


