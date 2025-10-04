#pragma once
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <json/value.h>
#include <functional>

class CSession;
struct FileTask {
	FileTask(std::shared_ptr<CSession> session, int uid, std::string path, std::string name,
		int seq, int total_size, int trans_size, int last, 
		std::string file_data,
		std::function<void(const Json::Value&)> callback) :_session(session), _uid(uid),
		_seq(seq), _path(path), _name(name), _total_size(total_size),
		_trans_size(trans_size), _last(last), _file_data(file_data), _callback(callback)
	{}
	~FileTask(){}
	std::shared_ptr<CSession> _session;
	int _uid;
	int _seq ;
	std::string _path;
	std::string _name ;
	int _total_size ;
	int _trans_size ;
	int _last ;
	std::string _file_data;
	std::function<void(const Json::Value&)>  _callback;  //添加回调函数
};


struct DownloadTask {
	DownloadTask(std::shared_ptr<CSession> session, int uid, std::string name,
		int seq, std::string file_path,
		std::function<void(const Json::Value&)> callback) :_session(session), _uid(uid),
		_seq(seq), _name(name), _file_path(file_path), _callback(callback)
	{}
	~DownloadTask() {}
	std::shared_ptr<CSession> _session;
	int _uid;
	int _seq;
	std::string _name;
	std::string _file_path;
	std::function<void(const Json::Value&)>  _callback;  //添加回调函数
};

class FileWorker
{
public:
	FileWorker();
	~FileWorker();
	void PostTask(std::shared_ptr<FileTask> task);
private:
	void task_callback(std::shared_ptr<FileTask>);
	std::thread _work_thread;
	std::queue<std::shared_ptr<FileTask>> _task_que;
	std::atomic<bool> _b_stop;
	std::mutex  _mtx;
	std::condition_variable _cv;
};


class DownloadWorker {
public:
	DownloadWorker();
	~DownloadWorker();
	void PostTask(std::shared_ptr<DownloadTask> task);
private:
	void task_callback(std::shared_ptr<DownloadTask>);
	std::thread _work_thread;
	std::queue<std::shared_ptr<DownloadTask>> _task_que;
	std::atomic<bool> _b_stop;
	std::mutex  _mtx;
	std::condition_variable _cv;
};

