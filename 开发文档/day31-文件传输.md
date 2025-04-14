---
title: 聊天项目(31) 文件传输
date: 2024-11-24 18:04:18
tags: [C++聊天项目]
categories: [C++聊天项目]
---

## 设计思路

文件传输必须满足以下几个条件：

- 限制文件大小(不超过`4G`)
- 长连接传输(效率高，支持大文件)
- 客户端和服务器都知道传输进度，以保证支持断点续传(后续实现)
- 先实现服务器单线程处理版本，在实现多线程处理版本

如遇问题可添加我的微信

<img src="https://cdn.llfc.club/wechat.jpg" alt="img" style="zoom: 33%;" />

也可以去我得哔哩哔哩主页查看项目视频详细讲解

B站主页 https://space.bilibili.com/271469206

## 客户端

客户端还是采用聊天项目客户端封装的`TcpClient`, 只是修改了发送逻辑

``` cpp
//发送数据槽函数
void TcpClient::slot_send_msg(quint16 id, QByteArray body)
{
    //如果连接异常则直接返回
    if(_socket->state() != QAbstractSocket::ConnectedState){
        emit sig_net_error(QString("断开连接无法发送"));
        return;
    }

    //获取body的长度
    quint32 bodyLength = body.size();

    //创建字节数组
    QByteArray data;
    //绑定字节数组
    QDataStream stream(&data, QIODevice::WriteOnly);
    //设置大端模式
    stream.setByteOrder(QDataStream::BigEndian);
    //写入ID
    stream << id;
    //写入长度
    stream << bodyLength;
    //写入包体
    data.append(body);

    //发送消息
     _socket->write(data);
}

```

这里着重叙述以下，发送的格式是`id + bodyLength + 文件流数据`

其中id 为2字节，`bodyLength`为4字节，之后就是传输的文件流

![https://cdn.llfc.club/1732450428990.jpg](https://cdn.llfc.club/1732450428990.jpg)

`slot_send_msg`是槽函数，和 `sig_send_msg`信号连接

``` cpp
 //连接 发送数据信号和槽函数
 connect(this, &TcpClient::sig_send_msg, this, &TcpClient::slot_send_msg);
```

客户端在发送数据的时候调用

``` cpp
void TcpClient::sendMsg(quint16 id,QByteArray data)
{
    //发送信号，统一交给槽函数处理，这么做的好处是多线程安全
    emit sig_send_msg(id, data);
}
```

客户端在打开文件对话框后选择文件，接下来，点击发送会将文件切分成固定大小的报文发送

``` cpp
void MainWindow::on_uploadBtn_clicked()
{
    ui->uploadBtn->setEnabled(false);
    // 打开文件
       QFile file(_file_name);
       if (!file.open(QIODevice::ReadOnly)) {
           qWarning() << "Could not open file:" << file.errorString();
           return;
       }

       // 保存当前文件指针位置
       qint64 originalPos = file.pos();
       QCryptographicHash hash(QCryptographicHash::Md5);
       if (!hash.addData(&file)) {
           qWarning() << "Failed to read data from file:" << _file_name;
           return ;
       }

       _file_md5 = hash.result().toHex(); // 返回十六进制字符串

    // 读取文件内容并发送
    QByteArray buffer;
    int seq = 0;

    QFileInfo fileInfo(_file_name); // 创建 QFileInfo 对象

    QString fileName = fileInfo.fileName(); // 获取文件名
    qDebug() << "文件名是：" << fileName; // 输出文件名
    int total_size = fileInfo.size();
    int last_seq = 0;
    if(total_size % MAX_FILE_LEN){
        last_seq = (total_size/MAX_FILE_LEN)+1;
    }else{
        last_seq = total_size/MAX_FILE_LEN;
    }

    // 恢复文件指针到原来的位置
    file.seek(originalPos);

    while (!file.atEnd()) {
        //每次读取2048字节发送
        buffer = file.read(MAX_FILE_LEN);
        QJsonObject jsonObj;
        // 将文件内容转换为 Base64 编码（可选）
        QString base64Data = buffer.toBase64();
        //qDebug() << "send data is " << base64Data;
        ++seq;
        jsonObj["md5"] = _file_md5;
        jsonObj["name"] = fileName;
        jsonObj["seq"] = seq;
        jsonObj["trans_size"] = buffer.size() + (seq-1)*MAX_FILE_LEN;
        jsonObj["total_size"] = total_size;
        if(buffer.size() < MAX_FILE_LEN){
            jsonObj["last"] = 1;
        }else{
            jsonObj["last"] = 0;
        }

        jsonObj["data"]= base64Data;
        jsonObj["last_seq"] = last_seq;
        QJsonDocument doc(jsonObj);
        auto send_data = doc.toJson();
        TcpClient::Inst().sendMsg(ID_UPLOAD_FILE_REQ, send_data);
        //startDelay(500);
     }

    //关闭文件
    file.close();

}
```

发送时数据字段分别为：

- 文件`md5 ` : 以后用来做断点续传校验

- `name` : 文件名
- `seq`:  报文序列号，类似于TCP序列号，自己定义的，服务器根据这个序列号组合数据写入文件。
- `trans_size`:   当前已经传输的大小

- `total_size`： 传输文件的总大小。

客户端需要接受服务器返回的消息更新进度条

``` cpp
//接受服务器发送的信息
void TcpClient::slot_ready_read()
{
    //读取所有数据
    QByteArray data = _socket->readAll();

    //将数据缓存起来
    _buffer.append(data);

    //处理收到的数据
    processData();
}
```

处理消息更新进度条

``` cpp
void TcpClient::processData()
{
    while(_buffer.size() >= TCP_HEAD_LEN){
        //先取出八字节头部
        auto head_byte = _buffer.left(TCP_HEAD_LEN);
        QDataStream stream(head_byte);
        //设置为大端模式
        stream.setByteOrder(QDataStream::BigEndian);
        //读取ID
        quint16 msg_id;
        stream >> msg_id;
        //读取长度
        quint32 body_length;
        stream >> body_length;
        if(_buffer.size() >= TCP_HEAD_LEN+body_length){
            //完整的消息体已经接受
            QByteArray body = _buffer.mid(TCP_HEAD_LEN,body_length);
            //去掉完整的消息包
            _buffer = _buffer.mid(TCP_HEAD_LEN+body_length);
            // 解析服务器发过来的消息
            QJsonDocument jsonDoc = QJsonDocument::fromJson(body);
            if(jsonDoc.isNull()){
                qDebug() << "Failed to create JSON doc.";
                this->_socket->close();
                return;
            }

            if(!jsonDoc.isObject()){
                qDebug() << "JSON is not an object.";
                this->_socket->close();
                return;
            }
            //qDebug() << "receive data is " << body;
            // 获取 JSON 对象
            QJsonObject jsonObject = jsonDoc.object();
            emit sig_logic_process(msg_id, jsonObject);
        }else{
            //消息未完全接受，所以中断
            break;
        }
    }
}
```

## 单线程逻辑服务器

我们先讲解单线程处理收包逻辑的服务器，以后再给大家将多线程的。

服务器要配合客户端，对报文头部大小做修改

``` cpp
//头部总长度
#define HEAD_TOTAL_LEN 6
//头部id长度
#define HEAD_ID_LEN 2
//头部数据长度
#define HEAD_DATA_LEN 4
// 接受队列最大个数
#define MAX_RECVQUE  2000000
// 发送队列最大个数
#define MAX_SENDQUE 2000000
```

其余逻辑和我们在网络编程中讲的`IocontextPool`模型服务器一样

服务器收到报文头后调用`LogicSystem`来处理

``` cpp
void CSession::AsyncReadBody(int total_len)
{
	auto self = shared_from_this();
	asyncReadFull(total_len, [self, this, total_len](const boost::system::error_code& ec, std::size_t bytes_transfered) {
		try {
			if (ec) {
				std::cout << "handle read failed, error is " << ec.what() << endl;
				Close();
				_server->ClearSession(_session_id);
				return;
			}

			if (bytes_transfered < total_len) {
				std::cout << "read length not match, read [" << bytes_transfered << "] , total ["
					<< total_len<<"]" << endl;
				Close();
				_server->ClearSession(_session_id);
				return;
			}

			memcpy(_recv_msg_node->_data , _data , bytes_transfered);
			_recv_msg_node->_cur_len += bytes_transfered;
			_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
			cout << "receive data is " << _recv_msg_node->_data << endl;
			//此处将消息投递到逻辑队列中
			LogicSystem::GetInstance()->PostMsgToQue(make_shared<LogicNode>(shared_from_this(), _recv_msg_node));
			//继续监听头部接受事件
			AsyncReadHead(HEAD_TOTAL_LEN);
		}
		catch (std::exception& e) {
			std::cout << "Exception code is " << e.what() << endl;
		}
		});
}
```

我们知道`LogicSystem`会将消息投递到队列里，然后单线程处理,  服务器`LogicSystem`注册上传逻辑

``` cpp
void LogicSystem::RegisterCallBacks() {
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
			auto data = root["data"].asString();
			//std::cout << "recv file data is  " << data << std::endl;

			Json::Value  rtvalue;
			Defer defer([this, &rtvalue, session]() {
				std::string return_str = rtvalue.toStyledString();
				session->Send(return_str, ID_UPLOAD_FILE_RSP);
				});

			// 解码
			std::string decoded = base64_decode(data);

			auto seq = root["seq"].asInt();
			auto name = root["name"].asString();
			auto total_size = root["total_size"].asInt();
			auto trans_size = root["trans_size"].asInt();
			auto file_path = ConfigMgr::Inst().GetFileOutPath();
			auto file_path_str = (file_path / name).string();
			std::cout << "file_path_str is " << file_path_str << std::endl;
			std::ofstream outfile;
			//第一个包
			if (seq == 1) {
				// 打开文件，如果存在则清空，不存在则创建
				outfile.open(file_path_str, std::ios::binary | std::ios::trunc);
			}
			else {
				// 保存为文件
				outfile.open(file_path_str, std::ios::binary | std::ios::app);
			}

			
			if (!outfile) {
				std::cerr << "无法打开文件进行写入。" << std::endl;
				return 1;
			}

			outfile.write(decoded.data(), decoded.size());
			if (!outfile) {
				std::cerr << "写入文件失败。" << std::endl;
				return 1;
			}

			outfile.close();
			std::cout << "文件已成功保存为: " << name <<  std::endl;

			rtvalue["error"] = ErrorCodes::Success;
			rtvalue["total_size"] = total_size;
			rtvalue["seq"] = seq;
			rtvalue["name"] = name;
			rtvalue["trans_size"] = trans_size;
	};
}
```

收到上传消息后写入文件。

## 多线程逻辑服务器

多线程逻辑服务器主要是为了缓解单线程接受数据造成的瓶颈，因为单线程接收数据，就会影响其他线程接收数据，所以考虑引入线程池处理收到的数据。

在多线程编程中我们讲过划分多线程设计的几种思路：

1. 按照任务划分，将不同的任务投递给不同的线程
2. 按照线程数轮询处理
3. 按照递归的方式划分

很明显我们不是做二分查找之类的算法处理，所以不会采用第三种。

现在考虑第二种，如果客户端发送一个很大的文件，客户端将文件切分为几个小份发送，服务器通过`iocontext`池接受数据, 将接受的数据投递到线程池。



我们知道线程池处理任务是不分先后顺序的，只要投递到队列中的都会被无序取出处理。

![https://cdn.llfc.club/1732945106584.jpg](https://cdn.llfc.club/1732945106584.jpg)

会造成数据包处理的乱序，当然可以最后交给一个线程去组合，统一写入文件，这么做的一个弊端就是如果文件很大，那就要等待完全重组完成才能组合为一个统一的包，如果文件很大，这个时间就会很长，当然也可以暂时缓存这些数据，每次收到后排序组合，比较麻烦。

所以这里推荐按照任务划分。

按照任务划分就是按照不同的客户端做区分，一个客户端传输的数据按照文件名字的hash值划分给不同的线程单独处理，也就是一个线程专门处理对应的hash值的任务，这样既能保证有序，又能保证其他线程可以处理其他任务，也有概率会命中hash同样的值投递给一个队列，但也扩充了并发能力。



![https://cdn.llfc.club/1732948742965.jpg](https://cdn.llfc.club/1732948742965.jpg)

因为我们之前的逻辑处理也是单线程，所以考虑在逻辑层这里做一下解耦合，因为这个服务只是用来处理数据接受，不涉及多个连接互相访问。所以可以讲logic线程扩充为多个，按照`sessionid`将不同的逻辑分配给不同的线程处理。

![https://cdn.llfc.club/1732952125218.jpg](https://cdn.llfc.club/1732952125218.jpg)

## 多线程处理逻辑

将`LogicSystem`中添加多个`LogicWorker`用来处理逻辑

``` cpp
typedef  function<void(shared_ptr<CSession>, const short &msg_id, const string &msg_data)> FunCallBack;
class LogicSystem:public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;
public:
	~LogicSystem();
	void PostMsgToQue(shared_ptr < LogicNode> msg, int index);
private:
	LogicSystem();
	std::vector<std::shared_ptr<LogicWorker> > _workers;
};
```

实现投递逻辑

``` cpp
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
```

每一个LogicWorker都包含一个线程，这样LogicWorker可以在独立的线程里处理任务

``` cpp
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
```

LogicWorker启动一个线程处理任务

``` cpp

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
```

当然要提前注册好任务

``` cpp
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
			auto seq = root["seq"].asInt();
			auto name = root["name"].asString();
			auto total_size = root["total_size"].asInt();
			auto trans_size = root["trans_size"].asInt();
			auto last = root["last"].asInt();
			auto file_data = root["data"].asString();
			Json::Value  rtvalue;
			Defer defer([this, &rtvalue, session]() {
				std::string return_str = rtvalue.toStyledString();
				session->Send(return_str, ID_UPLOAD_FILE_RSP);
				});

			// 使用 std::hash 对字符串进行哈希
			std::hash<std::string> hash_fn;
			size_t hash_value = hash_fn(name); // 生成哈希值
			int index = hash_value % FILE_WORKER_COUNT;
			std::cout << "Hash value: " << hash_value << std::endl;

			FileSystem::GetInstance()->PostMsgToQue(
				std::make_shared<FileTask>(session, name, seq, total_size,
					trans_size, last, file_data),
				index
			);

			rtvalue["error"] = ErrorCodes::Success;
			rtvalue["total_size"] = total_size;
			rtvalue["seq"] = seq;
			rtvalue["name"] = name;
			rtvalue["trans_size"] = trans_size;
			rtvalue["last"] = last;
	};
}
```

处理逻辑

``` cpp
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
```

比如对于文件上传，`ID_UPLOAD_FILE_REQ`就调用对应的回调，在回调函数里我们再次将要处理的任务封装好投递到文件系统

``` cpp
FileSystem::GetInstance()->PostMsgToQue(
				std::make_shared<FileTask>(session, name, seq, total_size,
					trans_size, last, file_data),
				index
			);
```

文件系统和逻辑系统类似，包含一堆FileWorker

``` cpp
class FileSystem :public Singleton<FileSystem>
{
	friend class Singleton<FileSystem>;
public:
	~FileSystem();
	void PostMsgToQue(shared_ptr <FileTask> msg, int index);
private:
	FileSystem();
	std::vector<std::shared_ptr<FileWorker>>  _file_workers;
};
```

实现投递逻辑

``` cpp
FileSystem::~FileSystem()
{

}

void FileSystem::PostMsgToQue(shared_ptr<FileTask> msg, int index)
{
	_file_workers[index]->PostTask(msg);
}

FileSystem::FileSystem()
{
	for (int i = 0; i < FILE_WORKER_COUNT; i++) {
		_file_workers.push_back(std::make_shared<FileWorker>());
	}
}
```

定义文件任务

``` cpp
class CSession;
struct FileTask {
	FileTask(std::shared_ptr<CSession> session, std::string name,
		int seq, int total_size, int trans_size, int last, 
		std::string file_data) :_session(session),
		_seq(seq),_name(name),_total_size(total_size),
		_trans_size(trans_size),_last(last),_file_data(file_data)
	{}
	~FileTask(){}
	std::shared_ptr<CSession> _session;

	int _seq ;
	std::string _name ;
	int _total_size ;
	int _trans_size ;
	int _last ;
	std::string _file_data;
};
```

实现文件工作者

``` cpp
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
```

构造函数启动线程

``` cpp
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
```

析构需等待线程

``` cpp
FileWorker::~FileWorker()
{
	_b_stop = true;
	_cv.notify_one();
	_work_thread.join();
}
```

投递任务

``` cpp
void FileWorker::PostTask(std::shared_ptr<FileTask> task)
{
	{
		std::lock_guard<std::mutex> lock(_mtx);
		_task_que.push(task);
	}

	_cv.notify_one();
}
```

因为线程会触发回调函数保存文件，所以我们实现回调函数

``` cpp
void FileWorker::task_callback(std::shared_ptr<FileTask> task)
{

	// 解码
	std::string decoded = base64_decode(task->_file_data);

	auto file_path = ConfigMgr::Inst().GetFileOutPath();
	auto file_path_str = (file_path / task->_name).string();
	auto last = task->_last;
	//std::cout << "file_path_str is " << file_path_str << std::endl;
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
		return ;
	}

	outfile.write(decoded.data(), decoded.size());
	if (!outfile) {
		std::cerr << "写入文件失败。" << std::endl;
		return ;
	}

	outfile.close();
	if (last) {
		std::cout << "文件已成功保存为: " << task->_name << std::endl;
	}

}
```

## 测试效果

![https://cdn.llfc.club/1732955339237.jpg](https://cdn.llfc.club/1732955339237.jpg)

## 源码链接

[https://gitee.com/secondtonone1/boostasio-learn/tree/master/network/day26-multithread-res-server](https://gitee.com/secondtonone1/boostasio-learn/tree/master/network/day26-multithread-res-server)

