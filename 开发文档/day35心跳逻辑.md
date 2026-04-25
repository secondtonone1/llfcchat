---
title: 心跳检测实现
date: 2025-05-01 11:57:41
tags: [C++聊天项目]
categories: [C++聊天项目]
---

## 前情回顾

前文我们实现了跨服踢人逻辑，通过分布式锁锁住不同服务器相同用户登录的操作，保证逻辑的原子性。

今天我们来谈一谈心跳机制，以及为什么要有心跳机制，以及该如何实现心跳机制，而且是分布式情况下心跳配合踢人逻辑该如何实现。

## 心跳概念

在一个“长连接”（如 TCP 持久连接、WebSocket、gRPC 长流等）中，客户端和服务端之间会保持一个持续打开的通道，以便双方可以随时双向发送数据。与一次性请求／响应模型（短连接）相比，长连接最大的挑战在于如何做到“及时发现网络或对端异常”、并“防止连接在中间节点（如路由器、NAT、防火墙）被静默地回收”。心跳机制正是为了解决这两个核心问题而引入的：

**心跳示意图**

![image-20250501122225534](https://cdn.llfc.club/img/image-20250501122225534.png)

**没有心跳**

![image-20250501122318014](https://cdn.llfc.club/img/image-20250501122318014.png)

当没有心跳机制的时候，如果设备异常断线(拔掉网线)，`tcp`层面可能无法立即感知，导致僵尸连接挂在服务器上。除非服务器发送数据给客户端才会感知到。或者被中间设备超时回收。

### 防止中间设备超时回收

许多网络设备（尤其是 NAT、负载均衡、防火墙）会对空闲连接设定一个超时阈值：

- 如果某段时间内连接上没有任何数据包经过，它会自动“回收”这条路由／会话，导致真正的数据到达时被丢弃或重置。
- 心跳包可以视作“活动信号”，让中间设备认为连接仍在活跃，从而维持映射表或会话状态，避免意外断开。

## 服务器心跳实现

服务器可以启动一个定时器，每隔60s检测一下所有连接，判断连接是否''活着''， 所谓"活着"就是连接没有断开。

怎么设置"活着"呢？就是对每一个Session(会话)设置一个时间戳，这个Session收到消息后，就更新这个时间戳。

服务器定时检测当前时间和这个时间戳的差值，如果大于一个阈值就说明连接断开了。这个阈值看服务器设定，一般60s即可。

``` cpp
void CServer::on_timer(const boost::system::error_code& ec) {
	//此处加锁遍历session
	{
		lock_guard<mutex> lock(_mutex);
		time_t now = std::time(nullptr);
		for (auto iter = _sessions.begin(); iter != _sessions.end(); ) {
			auto b_expired = iter->second->IsHeartbeatExpired(now);
			if (b_expired) {
				//关闭socket, 其实这里也会触发async_read的错误处理
				iter->second->Close();
                iter = _sessions.erase(iter);
				//加分布式锁清理redis信息
				auto uid_str = std::to_string(_user_uid);
				auto lock_key = LOCK_PREFIX + uid_str;
				auto identifier = RedisMgr::GetInstance()->acquireLock(lock_key, LOCK_TIME_OUT, 								ACQUIRE_TIME_OUT);
					Defer defer([identifier, lock_key, self, this]() {
					RedisMgr::GetInstance()->releaseLock(lock_key, identifier);
					});

				if (identifier.empty()) {
					return;
					}
				std::string redis_session_id = "";
				auto bsuccess = RedisMgr::GetInstance()->Get(USER_SESSION_PREFIX + uid_str, 								redis_session_id);
				if (!bsuccess) {
					return;
				}

				if (redis_session_id != _session_id) {
					//说明有客户在其他服务器异地登录了
					return;
				}

				RedisMgr::GetInstance()->Del(USER_SESSION_PREFIX + uid_str);
				//清除用户登录信息
				RedisMgr::GetInstance()->Del(USERIPPREFIX + uid_str);
					continue;
			}
             
				session_count++;
		}
	}
    
	//再次设置，下一个60s检测
	_timer.expires_after(std::chrono::seconds(60));
	_timer.async_wait([this](boost::system::error_code ec) {
		on_timer(ec);
	});
}
```

大家仔细观察这个代码，有没有发现什么问题？

这段代码是先加线程锁`_mutex`, 然后加分布式锁`lock_key`

但是我们看下`Session`读取掉线连接信息时会清空`redis`信息，流程如下

``` cpp
void CSession::AsyncReadHead(int total_len)
{
	auto self = shared_from_this();
	asyncReadFull(HEAD_TOTAL_LEN, [self, this](const boost::system::error_code& ec, std::size_t 					bytes_transfered) {
		try {
			if (ec) {
				std::cout << "handle read failed, error is " << ec.what() << endl;
				Close();
				auto self = shared_from_this();
				//加锁清除session
				auto uid_str = std::to_string(_user_uid);
				auto lock_key = LOCK_PREFIX + uid_str;
				auto identifier = RedisMgr::GetInstance()->acquireLock(lock_key, LOCK_TIME_OUT, 							ACQUIRE_TIME_OUT);
				Defer defer([identifier, lock_key, self, this]() {
				_server->ClearSession(_session_id);
				RedisMgr::GetInstance()->releaseLock(lock_key, identifier);
				});

				if (identifier.empty()) {
					return;
				}
				std::string redis_session_id = "";
				auto bsuccess = RedisMgr::GetInstance()->Get(USER_SESSION_PREFIX + uid_str, 								redis_session_id);
				if (!bsuccess) {
					return;
				}

				if (redis_session_id != _session_id) {
					//说明有客户在其他服务器异地登录了
					return;
				}

				RedisMgr::GetInstance()->Del(USER_SESSION_PREFIX + uid_str);
				//清除用户登录信息
				RedisMgr::GetInstance()->Del(USERIPPREFIX + uid_str);
				return;
			}
            
            //....省略正常逻辑
        }catch (std::exception& e) {
			std::cout << "Exception code is " << e.what() << endl;
		}
    });
}
```

`AsyncReadHead`错误处理中先加了分布式锁`lock_key`，再加线程锁`_mutex`

**图示如下**

![image-20250501153401691](https://cdn.llfc.club/img/image-20250501153401691.png)

上面图示已经很明显了，有概率造成死锁。

接下来谈谈死锁如何避免

##  如何避免死锁

### 线程锁避免死锁

如果是两个线程锁，避免死锁的最简单方式就是同时加锁，或者顺序一致性加锁

在 C++17 里，`std::scoped_lock`（也有人称它为“scope lock”）提供了对多个互斥量无死锁地一次性加锁的能力。它的核心在于内部调用了函数模板 `std::lock(m1, m2, …)`，该函数会：

1. **尝试按某种顺序非阻塞地抓取所有 mutex**：
   - `std::lock` 会循环地对每一个 mutex 做 `try_lock()`，
   - 如果有任何一个 `try_lock()` 失败，就立刻释放前面已经成功抓到的所有 mutex，退避（backoff），然后重试。
2. **保证最终所有 mutex 要么全部抓到了，要么都没抓到**：
   - 这样就避免了“线程 A 拿了 m1 等待 m2，而线程 B 拿了 m2 等待 m1”这种经典死锁情形。

只要你的所有代码都用同一个调用 `std::scoped_lock(m1, m2, …)` 的方式去加这几把锁，就不会出现交叉锁导致的死锁。



**用法示例**

```
#include <mutex>
#include <thread>
#include <iostream>

std::mutex mtx1, mtx2;

void worker1() {
    // 同时加 mtx1、mtx2，不会与另一个线程交叉死锁
    std::scoped_lock lock(mtx1, mtx2);
    std::cout << "worker1 got both locks\n";
    // … 操作受两把锁保护的资源 …
}

void worker2() {
    // 即便另一线程也是先 mtx2、后 mtx1，只要都改成 scoped_lock(mtx1, mtx2)，
    // 底层 std::lock 会保证不会死锁
    std::scoped_lock lock(mtx1, mtx2);
    std::cout << "worker2 got both locks\n";
    // … 相同资源操作 …
}

int main() {
    std::thread t1(worker1);
    std::thread t2(worker2);
    t1.join();
    t2.join();
    return 0;
}
```

**为什么不会死锁？**

- `std::scoped_lock(mtx1, mtx2)` 在构造时等价于：

  ```
  std::lock(mtx1, mtx2);
  ```

- `std::lock` 会：

  1. 先 `try_lock()` mtx1、再 `try_lock()` mtx2。
  2. 如果第二步失败，就释放第一把、稍作退避后重试。
  3. 直到两把都一次性成功为止。

这样就不会出现“线程 A 拿到 mtx1 → 等 mtx2”同时“线程 B 拿到 mtx2 → 等 mtx1”互相卡死的情况。

### 分布式锁

要解决“分布式锁 ↔ 线程锁”互相嵌套导致死锁的问题，核心思路就是：

1. **统一锁的获取顺序**

   - 始终按同一个顺序去申请锁。
   - 比如：不论是业务 A（先分布式锁后线程锁）还是心跳（先线程锁后分布式锁），都改成 “先拿分布式锁 → 再拿线程锁” 或者 “先拿线程锁 → 再拿分布式锁” 之一即可。
   - 只要保证两个场景里锁的申请顺序一致，就不会互相等待导致死锁。

2. **使用带超时的尝试锁（tryLock）+ 重试／回退策略**

   - 对于线程锁（例如 `ReentrantLock`）和分布式锁（例如 `Redisson` 的 `tryLock(long waitTime, long leaseTime, TimeUnit unit)`），都用 `tryLock` 而非阻塞式 `lock()`。
   - 如果某把锁在指定时间内拿不到，就释放已持有的那把锁，稍微退避（sleep 随机短时长）后重试。
   - 这样可以在检测到可能的死锁倾向时主动放弃，避免无限等待。

3. **合并锁或升级锁策略**

   - 如果分布式节点上并发线程只是共享同一把“逻辑锁”，可以考虑把本地线程锁和分布式锁做一次封装：

     ```
     class CombinedLock {
       RLock distLock;           
       std::mutex mtx;  
       public void lock() {
         distLock.lock();
         mtx.lock();
       }
       public void unlock() {
         mtx.unlock();
         distLock.unlock();
       }
     }
     ```

   - 这样业务层只用 `combinedLock.lock()`，根本不用关心哪把先后，底层永远是固定顺序。

4. **只用分布式锁或只用线程锁**

   - 如果心跳更新 Redis 的操作本身就是分布式的，就完全用分布式锁保护它，不再加线程锁。
   - 反之，如果这段更新完全在本机线程间协作，也可直接把分布式锁封装进本地锁里，让它表现得像本地锁。

5. **利用 Redis Lua 脚本保证原子性**

   - 将所有对 Redis 的读写操作放到一个 Lua 脚本里一次执行，借助 Redis 的单线程特性保证原子。
   - 这样就不需要额外的分布式锁，线程里也不用再加锁。

## 改造心跳服务

------

### 举例：改造心跳服务

```cpp
void CServer::on_timer(const boost::system::error_code& ec) {
	std::vector<std::shared_ptr<CSession>> _expired_sessions;
	int session_count = 0;
	//此处加锁遍历session
	{
		lock_guard<mutex> lock(_mutex);
		time_t now = std::time(nullptr);
		for (auto iter = _sessions.begin(); iter != _sessions.end(); iter++) {
			auto b_expired = iter->second->IsHeartbeatExpired(now);
			if (b_expired) {
				//关闭socket, 其实这里也会触发async_read的错误处理
				iter->second->Close();
				//收集过期信息
				_expired_sessions.push_back(iter->second);
				continue;
			}
			session_count++;
		}
	}

	//设置session数量
	auto& cfg = ConfigMgr::Inst();
	auto self_name = cfg["SelfServer"]["Name"];
	auto count_str = std::to_string(session_count);
	RedisMgr::GetInstance()->HSet(LOGIN_COUNT, self_name, count_str);

	//处理过期session, 单独提出，防止死锁
	for (auto &session : _expired_sessions) {
		session->DealExceptionSession();
	}
	
	//再次设置，下一个60s检测
	_timer.expires_after(std::chrono::seconds(60));
	_timer.async_wait([this](boost::system::error_code ec) {
		on_timer(ec);
	});
}
```

将清除逻辑提炼到函数`DealExceptionSession`

``` cpp
void CSession::DealExceptionSession()
{
	auto self = shared_from_this();
	//加锁清除session
	auto uid_str = std::to_string(_user_uid);
	auto lock_key = LOCK_PREFIX + uid_str;
	auto identifier = RedisMgr::GetInstance()->acquireLock(lock_key, LOCK_TIME_OUT, ACQUIRE_TIME_OUT);
	Defer defer([identifier, lock_key, self, this]() {
		_server->ClearSession(_session_id);
		RedisMgr::GetInstance()->releaseLock(lock_key, identifier);
		});

	if (identifier.empty()) {
		return;
	}
	std::string redis_session_id = "";
	auto bsuccess = RedisMgr::GetInstance()->Get(USER_SESSION_PREFIX + uid_str, redis_session_id);
	if (!bsuccess) {
		return;
	}

	if (redis_session_id != _session_id) {
		//说明有客户在其他服务器异地登录了
		return;
	}

	RedisMgr::GetInstance()->Del(USER_SESSION_PREFIX + uid_str);
	//清除用户登录信息
	RedisMgr::GetInstance()->Del(USERIPPREFIX + uid_str);
}
```



1. 持有本地线程锁只做遍历、收集过期 UID，不做删除；

2. 释放线程锁后，对每个 UID 按 “分布式锁→线程锁” 顺序逐个清理。

这样，所有“同时持有两把锁”的位置，顺序均为:

``` bash
分布式锁 → 本地线程锁
```

从而避免死锁

### 提炼异常处理

比如异步读处理

``` cpp
void CSession::AsyncReadBody(int total_len)
{
	auto self = shared_from_this();
	asyncReadFull(total_len, [self, this, total_len](const boost::system::error_code& ec, std::size_t bytes_transfered) {
		try {
			if (ec) {
				std::cout << "handle read failed, error is " << ec.what() << endl;
				Close();
				DealExceptionSession();
				return;
			}

			if (bytes_transfered < total_len) {
				std::cout << "read length not match, read [" << bytes_transfered << "] , total ["
					<< total_len<<"]" << endl;
				Close();
				_server->ClearSession(_session_id);
				return;
			}

			//判断连接无效
			if (!_server->CheckValid(_session_id)) {
				Close();
				return;
			}

			memcpy(_recv_msg_node->_data , _data , bytes_transfered);
			_recv_msg_node->_cur_len += bytes_transfered;
			_recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
			cout << "receive data is " << _recv_msg_node->_data << endl;
			//更新session心跳时间
			UpdateHeartbeat();
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

类似的还有，读头部，写数据等。

### 过期判断

判断现在时间和上一次心跳时间戳的差值，超过20s就认为连接过期。实际服务器心跳阈值最好60s，这里为了方便演示效果。

``` cpp
bool CSession::IsHeartbeatExpired(std::time_t& now) {
	double diff_sec = std::difftime(now, _last_heartbeat);
	if (diff_sec > 20) {
		std::cout << "heartbeat expired, session id is  " << _session_id << endl;
		return true;
	}

	return false;
}
```

更新心跳

``` cpp
void CSession::UpdateHeartbeat()
{
	time_t now = std::time(nullptr);
	_last_heartbeat = now;
}
```

在读取消息时做了更新

## 增加心跳请求处理

服务器增加心跳处理请求

``` cpp
void LogicSystem::HeartBeatHandler(std::shared_ptr<CSession> session, const short& msg_id, const string& msg_data) {
	Json::Reader reader;
	Json::Value root;
	reader.parse(msg_data, root);
	auto uid = root["fromuid"].asInt();
	std::cout << "receive heart beat msg, uid is " << uid << std::endl;
	Json::Value  rtvalue;
	rtvalue["error"] = ErrorCodes::Success;
	session->Send(rtvalue.toStyledString(), ID_HEARTBEAT_RSP);
}

void LogicSystem::RegisterCallBacks() {
	_fun_callbacks[MSG_CHAT_LOGIN] = std::bind(&LogicSystem::LoginHandler, this,
		placeholders::_1, placeholders::_2, placeholders::_3);

	_fun_callbacks[ID_SEARCH_USER_REQ] = std::bind(&LogicSystem::SearchInfo, this,
		placeholders::_1, placeholders::_2, placeholders::_3);

	_fun_callbacks[ID_ADD_FRIEND_REQ] = std::bind(&LogicSystem::AddFriendApply, this,
		placeholders::_1, placeholders::_2, placeholders::_3);

	_fun_callbacks[ID_AUTH_FRIEND_REQ] = std::bind(&LogicSystem::AuthFriendApply, this,
		placeholders::_1, placeholders::_2, placeholders::_3);

	_fun_callbacks[ID_TEXT_CHAT_MSG_REQ] = std::bind(&LogicSystem::DealChatTextMsg, this,
		placeholders::_1, placeholders::_2, placeholders::_3);

	_fun_callbacks[ID_HEART_BEAT_REQ] = std::bind(&LogicSystem::HeartBeatHandler, this,
		placeholders::_1, placeholders::_2, placeholders::_3);
	
}
```

客户端增加心跳处理发包和回复

**发包处理**

在`ChatDialog`构造函数中添加

``` cpp
    _timer = new QTimer(this);
    connect(_timer, &QTimer::timeout, this, [this](){
            auto user_info = UserMgr::GetInstance()->GetUserInfo();
            QJsonObject textObj;
            textObj["fromuid"] = user_info->_uid;
            QJsonDocument doc(textObj);
            QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
            emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_HEART_BEAT_REQ, jsonData);
    });

    _timer->start(10000);
```

在析构函数中添加

``` cpp
ChatDialog::~ChatDialog()
{
    _timer->stop();
    delete ui;
}
```



**回包处理**

``` cpp
    _handlers.insert(ID_HEARTBEAT_RSP,[this](ReqId id, int len, QByteArray data){
        Q_UNUSED(len);
        qDebug() << "handle id is " << id << " data is " << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        // 检查转换是否成功
        if (jsonDoc.isNull()) {
            qDebug() << "Failed to create QJsonDocument.";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        if (!jsonObj.contains("error")) {
            int err = ErrorCodes::ERR_JSON;
            qDebug() << "Heart Beat Msg Failed, err is Json Parse Err" << err;
            return;
        }

        int err = jsonObj["error"].toInt();
        if (err != ErrorCodes::SUCCESS) {
            qDebug() << "Heart Beat Msg Failed, err is " << err;
            return;
        }

        qDebug() << "Receive Heart Beat Msg Success" ;

    });
```

**客户端增加断线提示**

在`TcpMgr`构造函数中添加

``` cpp
        // 处理连接断开
        QObject::connect(&_socket, &QTcpSocket::disconnected, [&]() {
            qDebug() << "Disconnected from server.";
            //并且发送通知到界面
            emit sig_connection_closed();
        });
```

`MainWindow`构造函数中添加信号连接

``` cpp
    //连接服务器断开心跳超时或异常连接信息
    connect(TcpMgr::GetInstance().get(),&TcpMgr::sig_connection_closed, this, &MainWindow::SlotExcepConOffline);
```

槽函数

``` cpp
void MainWindow::SlotExcepConOffline()
{
    // 使用静态方法直接弹出一个信息框
        QMessageBox::information(this, "下线提示", "心跳超时或临界异常，该终端下线！");
        TcpMgr::GetInstance()->CloseConnection();
        offlineLogin();
}
```



## 效果测试

为了方便测试，我们修改`StatusServer`中`GetServer`逻辑只返回第一个`ChatServer1`

```  cpp
ChatServer StatusServiceImpl::getChatServer() {
	std::lock_guard<std::mutex> guard(_server_mtx);
	auto minServer = _servers.begin()->second;
	auto lock_key = LOCK_COUNT;
	//暂时注释
	//auto identifier = RedisMgr::GetInstance()->acquireLock(lock_key, LOCK_TIME_OUT, ACQUIRE_TIME_OUT);
	////利用defer解锁
	//Defer defer2([this, identifier, lock_key]() {
	//	RedisMgr::GetInstance()->releaseLock(lock_key, identifier);
	//	});

	//auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, minServer.name);
	//if (count_str.empty()) {
	//	//不存在则默认设置为最大
	//	minServer.con_count = INT_MAX;
	//}
	//else {
	//	minServer.con_count = std::stoi(count_str);
	//}


	//// 使用范围基于for循环
	//for ( auto& server : _servers) {
	//	
	//	if (server.second.name == minServer.name) {
	//		continue;
	//	}

	//	auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server.second.name);
	//	if (count_str.empty()) {
	//		server.second.con_count = INT_MAX;
	//	}
	//	else {
	//		server.second.con_count = std::stoi(count_str);
	//	}

	//	if (server.second.con_count < minServer.con_count) {
	//		minServer = server.second;
	//	}
	//}

	return minServer;
}
```



我们启动客户端，以及服务器，先屏蔽客户端发送心跳逻辑，可以看到服务器检测心跳超时后会切断客户端连接

![image-20250501173202038](https://cdn.llfc.club/img/image-20250501173202038.png)

客户端添加心跳包发送

可以看到每隔10s客户端就发送心跳包给服务器，服务器收到后，打印日志，客户端也打印日志

![image-20250501173829231](https://cdn.llfc.club/img/image-20250501173829231.png)

## 优化连接数统计

之前我们统计一个服务器连接数，都是在服务器检测登录一个用户就增加连接数写入`redis`，以及`CSession`析构减少连接数写入`redis`, 还加了分布式锁，这种做法频繁加锁会影响效率，现在我们有了心跳检测，只需要在心跳检测结束后将统计的连接数写入`redis`即可

``` cpp
void CServer::on_timer(const boost::system::error_code& ec) {
	std::vector<std::shared_ptr<CSession>> _expired_sessions;
	int session_count = 0;
	//此处加锁遍历session
	{
		lock_guard<mutex> lock(_mutex);
		time_t now = std::time(nullptr);
		for (auto iter = _sessions.begin(); iter != _sessions.end(); iter++) {
			auto b_expired = iter->second->IsHeartbeatExpired(now);
			if (b_expired) {
				//关闭socket, 其实这里也会触发async_read的错误处理
				iter->second->Close();
				//收集过期信息
				_expired_sessions.push_back(iter->second);
				continue;
			}
			session_count++;
		}
	}

	//设置session数量
	auto& cfg = ConfigMgr::Inst();
	auto self_name = cfg["SelfServer"]["Name"];
	auto count_str = std::to_string(session_count);
	RedisMgr::GetInstance()->HSet(LOGIN_COUNT, self_name, count_str);

	//处理过期session, 单独提出，防止死锁
	for (auto &session : _expired_sessions) {
		session->DealExceptionSession();
	}
	
	//再次设置，下一个60s检测
	_timer.expires_after(std::chrono::seconds(60));
	_timer.async_wait([this](boost::system::error_code ec) {
		on_timer(ec);
	});
}
```

状态服务器中获取连接数返回`ChatServer`也可以简化了

``` cpp
ChatServer StatusServiceImpl::getChatServer() {
	std::lock_guard<std::mutex> guard(_server_mtx);
	auto minServer = _servers.begin()->second;
	
	auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, minServer.name);
	if (count_str.empty()) {
		//不存在则默认设置为最大
		minServer.con_count = INT_MAX;
	}
	else {
		minServer.con_count = std::stoi(count_str);
	}


	// 使用范围基于for循环
	for ( auto& server : _servers) {
		
		if (server.second.name == minServer.name) {
			continue;
		}

		auto count_str = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server.second.name);
		if (count_str.empty()) {
			server.second.con_count = INT_MAX;
		}
		else {
			server.second.con_count = std::stoi(count_str);
		}

		if (server.second.con_count < minServer.con_count) {
			minServer = server.second;
		}
	}

	return minServer;
}
```

哪怕我们获取的信息是旧的数据也没关系，负载分配没必要太精确，还有心跳每隔60s会更新依次连接逻辑，所以问题不大。

我们把`ChatServer2`的逻辑也更新成和`ChatServer1`，再次测试分布式情况下踢人+心跳逻辑

![image-20250501180339785](https://cdn.llfc.club/img/image-20250501180339785.png)

可以看到同账号异地登录，旧客户端收到被踢掉消息，旧的客户端关闭连接，所以弹出`心跳超时或异常,该终端下线`

所以我们这期踢人+心跳检测就实现了。

## 源码和视频

源码连接：

https://gitee.com/secondtonone1/llfcchat

视频连接：

https://www.bilibili.com/video/BV1ct5xzcEka/?vd_source=8be9e83424c2ed2c9b2a3ed1d01385e9
