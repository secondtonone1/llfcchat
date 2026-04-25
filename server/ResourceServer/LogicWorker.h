#pragma once
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <memory>
#include "MsgNode.h"
#include <unordered_map>

class CSession;
class LogicNode {
public:
	LogicNode(std::shared_ptr<CSession>, std::shared_ptr<RecvNode>);
	shared_ptr<CSession> _session;
	shared_ptr<RecvNode> _recvnode;
};

typedef  function<void(shared_ptr<CSession>, 
	const short& msg_id, const string& msg_data)> FunCallBack;

class LogicWorker
{
public:
	LogicWorker();
	~LogicWorker();
	void PostTask(std::shared_ptr<LogicNode> task);
	void RegisterCallBacks();
private:
	void task_callback(std::shared_ptr<LogicNode>);
	std::thread _work_thread;
	std::queue<std::shared_ptr<LogicNode>> _task_que;
	std::atomic<bool> _b_stop;
	std::mutex  _mtx;
	std::condition_variable _cv;
	std::unordered_map<short, FunCallBack> _fun_callbacks;
};

