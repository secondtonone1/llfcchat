#include "LogicWorker.h"
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "FileSystem.h"
#include "CSession.h"
#include "LogicSystem.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"

LogicWorker::LogicWorker():_b_stop(false)
{
	RegisterCallBacks();

	_work_thread = std::thread([this]() {
		while (!_b_stop) {
			std::unique_lock<std::mutex> lock(_mtx);
			_cv.wait(lock, [this]() {
				if(_b_stop) {
					return true;
				}

				if (_task_que.empty()) {
					return false;
				}

				return true;

			});

			if (_b_stop) {
				return;
			}

			auto task = _task_que.front();
			task_callback(task);
			_task_que.pop();
		}
	});

}

LogicWorker::~LogicWorker()
{
	_b_stop = true;
	_cv.notify_one();
	_work_thread.join();
}

void LogicWorker::PostTask(std::shared_ptr<LogicNode> task)
{
	std::lock_guard<std::mutex> lock(_mtx);
	_task_que.push(task);
	_cv.notify_one();
}

void LogicWorker::RegisterCallBacks()
{
	_fun_callbacks[ID_TEST_MSG_REQ] = [this](shared_ptr<CSession> session, const short& msg_id,
		const string& msg_data) {
			Json::Reader reader;
			Json::Value root;
			reader.parse(msg_data, root);
			auto data = root["data"].asString();
			std::cout << "recv test data is  " << data << std::endl;

			Json::Value  rtvalue;
			Defer defer([this, &rtvalue, session]() {
				std::string return_str = rtvalue.toStyledString();
				session->Send(return_str, ID_TEST_MSG_RSP);
				});

			rtvalue["error"] = ErrorCodes::Success;
			rtvalue["data"] = data;
	};

	_fun_callbacks[ID_UPLOAD_FILE_REQ] = [this](shared_ptr<CSession> session, const short& msg_id,
		const string& msg_data) {
			Json::Reader reader;
			Json::Value root;
			reader.parse(msg_data, root);
			auto md5 = root["md5"].asString();
			auto seq = root["seq"].asInt();
			auto name = root["name"].asString();
			auto total_size = root["total_size"].asInt();
			auto trans_size = root["trans_size"].asInt();
			auto last = root["last"].asInt();
			auto file_data = root["data"].asString();
			auto file_path = ConfigMgr::Inst().GetFileOutPath();
			auto uid = root["uid"].asInt();
			//转化为字符串
			auto uid_str = std::to_string(uid);
			auto file_path_str = (file_path / uid_str/ name).string();
			Json::Value  rtvalue;

			auto callback = [=](const Json::Value& result) {

				// 在异步任务完成后调用
				Json::Value rtvalue = result;
				rtvalue["error"] = ErrorCodes::Success;
				rtvalue["total_size"] = total_size;
				rtvalue["seq"] = seq;
				rtvalue["name"] = name;
				rtvalue["trans_size"] = trans_size;
				rtvalue["last"] = last;
				rtvalue["md5"] = md5;
				rtvalue["uid"] = uid;
				std::string return_str = rtvalue.toStyledString();
				session->Send(return_str, ID_UPLOAD_FILE_RSP);
			};
			
			// 使用 std::hash 对字符串进行哈希
			std::hash<std::string> hash_fn;
			size_t hash_value = hash_fn(name); // 生成哈希值
			int index = hash_value % FILE_WORKER_COUNT;
			std::cout << "Hash value: " << hash_value << std::endl;

			//第一个包
			if (seq == 1) {
				//构造数据存储
				auto file_info = std::make_shared<FileInfo>();
				file_info->_file_path_str = file_path_str;
				file_info->_name = name;
				file_info->_seq = seq;
				file_info->_total_size = total_size;
				file_info->_trans_size = trans_size;
				//todo... 后期改为redis,以及mysql 持久化存储
				LogicSystem::GetInstance()->AddMD5File(md5, file_info);
			}
			else {
				auto file_info = LogicSystem::GetInstance()->GetFileInfo(md5);
				if (file_info == nullptr) {
					rtvalue["error"] = ErrorCodes::FileNotExists;
					std::string return_str = rtvalue.toStyledString();
					session->Send(return_str, ID_UPLOAD_FILE_RSP);
					return;
				}
				file_info->_seq = seq;
				file_info->_trans_size = trans_size;
			}


			FileSystem::GetInstance()->PostMsgToQue(
				std::make_shared<FileTask>(session, uid, file_path_str, name, seq, total_size,
					trans_size, last, file_data, callback),
				index
			);
	};



	_fun_callbacks[ID_SYNC_FILE_REQ] = [this](shared_ptr<CSession> session, const short& msg_id,
		const string& msg_data) {

			Json::Reader reader;
			Json::Value root;
			reader.parse(msg_data, root);

			Json::Value  rtvalue;
			Defer defer([this, &rtvalue, session]() {
				std::string return_str = rtvalue.toStyledString();
				session->Send(return_str, ID_SYNC_FILE_RSP);
				});

			auto md5 = root["md5"].asString();

			auto file = LogicSystem::GetInstance()->GetFileInfo(md5);
			if (file == nullptr) {
				rtvalue["error"] = ErrorCodes::FileNotExists;
				return;
			}

			rtvalue["error"] = ErrorCodes::Success;
			rtvalue["total_size"] = file->_total_size;
			rtvalue["seq"] = file->_seq;
			rtvalue["name"] = file->_name;
			rtvalue["trans_size"] = file->_trans_size;
			rtvalue["md5"] = md5;

	};


	_fun_callbacks[ID_UPLOAD_HEAD_ICON_REQ] = [this](shared_ptr<CSession> session, const short& msg_id,
		const string& msg_data) {
			Json::Reader reader;
			Json::Value root;
			reader.parse(msg_data, root);
			auto md5 = root["md5"].asString();
			auto seq = root["seq"].asInt();
			auto name = root["name"].asString();
			auto total_size = root["total_size"].asInt();
			auto trans_size = root["trans_size"].asInt();
			auto last = root["last"].asInt();
			auto file_data = root["data"].asString();
			auto uid = root["uid"].asInt();
			auto token = root["token"].asString();
			auto last_seq = root["last_seq"].asInt();
			//转化为字符串
			auto uid_str = std::to_string(uid);

			auto file_path = ConfigMgr::Inst().GetFileOutPath();
			auto file_path_str = (file_path / uid_str / name).string();
			Json::Value  rtvalue;
			auto callback = [=](const Json::Value& result) {

				// 在异步任务完成后调用
				Json::Value rtvalue = result;
				rtvalue["total_size"] = total_size;
				rtvalue["seq"] = seq;
				rtvalue["name"] = name;
				rtvalue["trans_size"] = trans_size;
				rtvalue["last"] = last;
				rtvalue["md5"] = md5;
				rtvalue["uid"] = uid;
				rtvalue["last_seq"] = last_seq;
				std::string return_str = rtvalue.toStyledString();
				session->Send(return_str, ID_UPLOAD_HEAD_ICON_RSP);
			};

			//第一个包校验一下token是否合理
			if (seq == 1) {
				//从redis获取用户token是否正确
				std::string uid_str = std::to_string(uid);
				std::string token_key = USERTOKENPREFIX + uid_str;
				std::string token_value = "";
				bool success = RedisMgr::GetInstance()->Get(token_key, token_value);
				if (!success) {
					rtvalue["error"] = ErrorCodes::UidInvalid;
					std::string return_str = rtvalue.toStyledString();
					session->Send(return_str, ID_UPLOAD_HEAD_ICON_RSP);
					return;
				}

				if (token_value != token) {
					rtvalue["error"] = ErrorCodes::TokenInvalid;
					std::string return_str = rtvalue.toStyledString();
					session->Send(return_str, ID_UPLOAD_HEAD_ICON_RSP);
					return;
				}
			}

			// 使用 std::hash 对字符串进行哈希
			std::hash<std::string> hash_fn;
			size_t hash_value = hash_fn(name); // 生成哈希值
			int index = hash_value % FILE_WORKER_COUNT;
			std::cout << "Hash value: " << hash_value << std::endl;

			//第一个包
			if (seq == 1) {
				//构造数据存储
				auto file_info = std::make_shared<FileInfo>();
				file_info->_file_path_str = file_path_str;
				file_info->_name = name;
				file_info->_seq = seq;
				file_info->_total_size = total_size;
				file_info->_trans_size = trans_size;
				//LogicSystem::GetInstance()->AddMD5File(md5, file_info);
				//改为用redis存储
				bool success = RedisMgr::GetInstance()->SetFileInfo(name, file_info);
				if (!success) {
					rtvalue["error"] = ErrorCodes::FileSaveRedisFailed;
					std::string return_str = rtvalue.toStyledString();
					session->Send(return_str, ID_UPLOAD_HEAD_ICON_RSP);
					return;
				}
			}
			else {
				//auto file_info = LogicSystem::GetInstance()->GetFileInfo(md5);
				//改为从redis中加载
				auto file_info = RedisMgr::GetInstance()->GetFileInfo(name);
				if (file_info == nullptr) {
					rtvalue["error"] = ErrorCodes::FileNotExists;
					std::string return_str = rtvalue.toStyledString();
					session->Send(return_str, ID_UPLOAD_HEAD_ICON_RSP);
					return;
				}
				file_info->_seq = seq;
				file_info->_trans_size = trans_size;
				bool success = RedisMgr::GetInstance()->SetFileInfo(name, file_info);
				if (!success) {
					rtvalue["error"] = ErrorCodes::FileSaveRedisFailed;
					std::string return_str = rtvalue.toStyledString();
					session->Send(return_str, ID_UPLOAD_HEAD_ICON_RSP);
					return;
				}
			}


			FileSystem::GetInstance()->PostMsgToQue(
				std::make_shared<FileTask>(session, uid, file_path_str, name, seq, total_size,
					trans_size, last, file_data, callback),
				index
			);

	};

	_fun_callbacks[ID_DOWN_LOAD_FILE_REQ] = [this](shared_ptr<CSession> session, const short& msg_id,
		const string& msg_data) {
			Json::Reader reader;
			Json::Value root;
			reader.parse(msg_data, root);
			auto seq = root["seq"].asInt();
			auto name = root["name"].asString();
			auto uid = root["uid"].asInt();
			auto token = root["token"].asString();
			auto client_path = root["client_path"].asString();

			//转化为字符串
			auto uid_str = std::to_string(uid);

			auto file_path = ConfigMgr::Inst().GetFileOutPath();
			auto file_path_str = (file_path / uid_str / name).string();
			Json::Value  rtvalue;
			auto callback = [=](const Json::Value& result) {

				// 在异步任务完成后调用
				Json::Value rtvalue = result;
				rtvalue["client_path"] = client_path;
				rtvalue["name"] = name;
				std::string return_str = rtvalue.toStyledString();
				session->Send(return_str, ID_DOWN_LOAD_FILE_RSP);
			};

			//第一个包校验一下token是否合理
			if (seq == 1) {
				//从redis获取用户token是否正确
				std::string uid_str = std::to_string(uid);
				std::string token_key = USERTOKENPREFIX + uid_str;
				std::string token_value = "";
				bool success = RedisMgr::GetInstance()->Get(token_key, token_value);
				if (!success) {
					rtvalue["error"] = ErrorCodes::UidInvalid;
					std::string return_str = rtvalue.toStyledString();
					session->Send(return_str, ID_DOWN_LOAD_FILE_RSP);
					return;
				}

				if (token_value != token) {
					rtvalue["error"] = ErrorCodes::TokenInvalid;
					std::string return_str = rtvalue.toStyledString();
					session->Send(return_str, ID_DOWN_LOAD_FILE_RSP);
					return;
				}
			}

			// 使用 std::hash 对字符串进行哈希
			std::hash<std::string> hash_fn;
			size_t hash_value = hash_fn(name); // 生成哈希值
			int index = hash_value % FILE_WORKER_COUNT;
			std::cout << "Hash value: " << hash_value << std::endl;

			FileSystem::GetInstance()->PostDownloadTaskToQue(
				std::make_shared<DownloadTask>(session, uid, name, seq, file_path_str, callback),
				index
			);

	};


}

void LogicWorker::task_callback(std::shared_ptr<LogicNode> task)
{
	cout << "recv_msg id  is " << task->_recvnode->_msg_id << endl;
	auto call_back_iter = _fun_callbacks.find(task->_recvnode->_msg_id);
	if (call_back_iter == _fun_callbacks.end()) {
		return;
	}
	call_back_iter->second(task->_session, task->_recvnode->_msg_id,
		std::string(task->_recvnode->_data, task->_recvnode->_cur_len));
}
