#include "FileWorker.h"
#include "CSession.h"
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "base64.h"
#include "ConfigMgr.h"
#include "MysqlMgr.h"
#include "RedisMgr.h"

FileWorker::FileWorker():_b_stop(false)
{
	_work_thread = std::thread([this]() {
		while (!_b_stop) {
			std::unique_lock<std::mutex> lock(_mtx);
			_cv.wait(lock, [this]() {
				if (_b_stop) {
					return true;
				}

				if (_task_que.empty()) {
					return false;
				}

				return true;
			});

			if (_b_stop) {
				break;
			}

			auto task = _task_que.front();
			_task_que.pop();
			task_callback(task);
		}
		
	});
}

FileWorker::~FileWorker()
{
	_b_stop = true;
	_cv.notify_one();
	_work_thread.join();
}

void FileWorker::PostTask(std::shared_ptr<FileTask> task)
{
	{
		std::lock_guard<std::mutex> lock(_mtx);
		_task_que.push(task);
	}

	_cv.notify_one();
}

void FileWorker::task_callback(std::shared_ptr<FileTask> task)
{
	// 解码
	std::string decoded = base64_decode(task->_file_data);

	auto file_path_str = task->_path;
	auto last = task->_last;
	//std::cout << "file_path_str is " << file_path_str << std::endl;

	boost::filesystem::path file_path(file_path_str);
	boost::filesystem::path dir_path = file_path.parent_path();
	// 获取完整文件名（包含扩展名）
	std::string filename = file_path.filename().string();
	Json::Value result;
	result["error"] = ErrorCodes::Success;

	// Check if directory exists, if not, create it
	if (!boost::filesystem::exists(dir_path)) {
		if (!boost::filesystem::create_directories(dir_path)) {
			std::cerr << "Failed to create directory: " << dir_path.string() << std::endl;
			result["error"] = ErrorCodes::FileNotExists;
			task->_callback(result);
			return;
		}
	}


	std::ofstream outfile;
	//第一个包
	if (task->_seq == 1) {
		// 打开文件，如果存在则清空，不存在则创建
		outfile.open(file_path_str, std::ios::binary | std::ios::trunc);
	}
	else {
		// 保存为文件
		outfile.open(file_path_str, std::ios::binary | std::ios::app);
	}


	if (!outfile) {
		std::cerr << "无法打开文件进行写入。" << std::endl;
		result["error"] = ErrorCodes::FileWritePermissionFailed;
		task->_callback(result);
		return ;
	}

	outfile.write(decoded.data(), decoded.size());
	if (!outfile) {
		std::cerr << "写入文件失败。" << std::endl;
		result["error"] = ErrorCodes::FileWritePermissionFailed;
		task->_callback(result);
		return ;
	}

	outfile.close();
	if (last) {
		std::cout << "文件已成功保存为: " << task->_name << std::endl;
		//更新头像
		MysqlMgr::GetInstance()->UpdateUserIcon(task->_uid, filename);
		//获取用户信息
		auto user_info = MysqlMgr::GetInstance()->GetUser(task->_uid);
		if (user_info == nullptr) {
			return ;
		}

		//将数据库内容写入redis缓存
		Json::Value redis_root;
		redis_root["uid"] = task->_uid;
		redis_root["pwd"] = user_info->pwd;
		redis_root["name"] = user_info->name;
		redis_root["email"] = user_info->email;
		redis_root["nick"] = user_info->nick;
		redis_root["desc"] = user_info->desc;
		redis_root["sex"] = user_info->sex;
		redis_root["icon"] = user_info->icon;
		std::string base_key = USER_BASE_INFO + std::to_string(task->_uid);
		RedisMgr::GetInstance()->Set(base_key, redis_root.toStyledString());
	}

	if (task->_callback) {
		task->_callback(result);
	}

}
