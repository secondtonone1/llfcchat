---
title: 分布式锁设计思路
date: 2025-03-31 19:05:14
tags: [C++聊天项目]
categories: [C++聊天项目]
---

## 1. 引言

在分布式系统中，多个客户端可能同时访问和操作共享资源。为了防止数据竞争和不一致，分布式锁是一个常见的解决方案。Redis 提供了强大的功能来实现高效且可靠的分布式锁。本文将通过 C++ 和 Redis（通过 `hredis` 库）实现一个简单的分布式锁。

![image-20250403133049929](https://cdn.llfc.club/img/image-20250403133049929.png)

## 2. 项目背景

- **分布式锁**：它是一种机制，用于保证在分布式系统中，某一时刻只有一个客户端能够执行某些共享资源的操作。
- **使用 Redis 作为锁存储**：Redis 被用作集中式存储，可以确保锁的状态在所有参与者之间同步。



## 3. 设计思路

分布式锁的核心思想是：

1. **加锁**：客户端通过设置一个 Redis 键来获取锁。通过 Redis 的原子操作，确保只有一个客户端能够成功设置该键。
2. **持有者标识符**：每个客户端在加锁时生成一个唯一的标识符（UUID），该标识符用来标识锁的持有者。
3. **超时机制**：锁会在一定时间后自动释放（过期），防止因程序异常导致的死锁。
4. **解锁**：只有锁的持有者才能释放锁，这通过 Redis Lua 脚本来保证。

------



## 4. 代码实现步骤

### 4.1 生成全局唯一标识符 (UUID)

使用 **Boost UUID** 库生成一个全局唯一的标识符（UUID）。这个标识符会被用作锁的持有者标识符。它确保每个客户端在加锁时拥有唯一的标识，从而能够确保锁的唯一性。

**代码：**

```cpp
std::string generateUUID() {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    return to_string(uuid);
}
```



### 4.2 尝试加锁（`acquireLock` 函数）

客户端通过 Redis 的 `SET` 命令尝试加锁。该命令的参数如下：

- `NX`：确保只有当键不存在时才能成功设置（即，只有一个客户端能够成功设置锁）。
- `EX`：设置一个超时时间，锁会在超时后自动释放，避免死锁。

如果加锁成功，返回一个唯一标识符。如果加锁失败，则会在指定的超时时间内多次尝试。

**代码如下：**

```cpp
// 尝试获取锁，返回锁的唯一标识符（UUID），如果获取失败则返回空字符串
std::string acquireLock(redisContext* context, const std::string& lockName, int lockTimeout, int acquireTimeout) {
    std::string identifier = generateUUID();
    std::string lockKey = "lock:" + lockName;
    auto endTime = std::chrono::steady_clock::now() + std::chrono::seconds(acquireTimeout);

    while (std::chrono::steady_clock::now() < endTime) {
        // 使用 SET 命令尝试加锁：SET lockKey identifier NX EX lockTimeout
        redisReply* reply = (redisReply*)redisCommand(context, "SET %s %s NX EX %d", lockKey.c_str(), identifier.c_str(), lockTimeout);
        if (reply != nullptr) {
            // 判断返回结果是否为 OK
            if (reply->type == REDIS_REPLY_STATUS && std::string(reply->str) == "OK") {
                freeReplyObject(reply);
                return identifier;
            }
            freeReplyObject(reply);
        }
        // 暂停 1 毫秒后重试，防止忙等待
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return "";
}
```

**函数参数说明**

**redisContext\* context**
 这是一个指向 Redis 连接上下文的指针，用于与 Redis 服务器通信。通过这个上下文，你可以发送命令和接收响应。

**const std::string& lockName**
 这是你想要加锁的资源名称。例如，如果你需要对某个资源加锁，可以用 `"my_resource"`，函数内部会把它拼接成 `"lock:my_resource"` 作为 Redis 中的 key。

**int lockTimeout**
 这是锁的有效期，单位是秒。设置这个值的目的是防止因程序异常或崩溃而导致的死锁。当锁达到这个超时时间后，Redis 会自动删除这个 key，从而释放锁。

**int acquireTimeout**
 这是获取锁的最大等待时间，单位也是秒。如果在这个时间内没有成功获取到锁，函数就会停止尝试，并返回空字符串。这样可以避免程序无限等待。



**Redis 命令解释**

在 `acquireLock` 函数中，使用的 Redis 命令格式是：

```
"SET %s %s NX EX %d"
```

这个命令实际上是一个格式化字符串，参数会被填入以下位置：

1. **SET**
    Redis 的基本命令，用于设置一个 key 的值。
2. **%s**（第一个 %s）
    代表锁的 key（例如 `"lock:my_resource"`）。
3. **%s**（第二个 %s）
    代表锁的持有者标识符，也就是通过 `generateUUID()` 生成的 UUID。
4. **NX**
    表示 “Not eXists”，意思是“只有当 key 不存在时才进行设置”。这可以保证如果其他客户端已经设置了这个 key（即已经有锁了），那么当前客户端就不会覆盖原来的锁。
5. **EX %d**
    `EX` 参数用于指定 key 的过期时间，`%d` 表示锁的有效期（lockTimeout），单位为秒。这样即使客户端因某些原因没有正常释放锁，锁也会在指定时间后自动失效。



**构造锁的 Redis 键**

```cpp
std::string lockKey = "lock:" + lockName;
```

- 构造出 Redis 键，格式为 `lock:lockName`，用于存储锁的状态。

**设置获取锁的截止时间**

```cpp
auto endTime = std::chrono::steady_clock::now() + std::chrono::seconds(acquireTimeout);
```

- 设置一个截止时间 `endTime`，表示最多尝试获取锁的时间，单位为秒。当前时间加上 `acquireTimeout` 秒即为截止时间。

**尝试获取锁**

```cpp
while (std::chrono::steady_clock::now() < endTime) {
```

- 通过一个 `while` 循环，不断尝试获取锁，直到超时或成功获取锁。

**使用 Redis 的 SET 命令尝试加锁**

```cpp
redisReply* reply = (redisReply*)redisCommand(context, "SET %s %s NX EX %d", lockKey.c_str(), identifier.c_str(), lockTimeout);
```

- 通过 Redis 的 `SET` 命令来尝试获取锁，命令格式为：
  - `SET lockKey identifier NX EX lockTimeout`：
    - `NX`：只有当 `lockKey` 不存在时才会设置成功（即实现了锁的功能，防止其他客户端重入）。
    - `EX lockTimeout`：设置锁的过期时间为 `lockTimeout` 秒，防止锁永远占用。
  - 如果锁成功获取，Redis 会返回 `OK`。

**检查返回结果**

```cpp
if (reply != nullptr) {
    if (reply->type == REDIS_REPLY_STATUS && std::string(reply->str) == "OK") {
        freeReplyObject(reply);
        return identifier;
    }
    freeReplyObject(reply);
}
```

- 如果 Redis 返回的 `reply` 不为空，检查返回值类型是否为 `REDIS_REPLY_STATUS`，并且返回的字符串是否是 `OK`，表示锁成功获取。
- 如果获取锁成功，释放 `redisReply` 对象，并返回生成的唯一标识符 `identifier`，表示锁已经成功获得。

**暂停并重试**

```
std::this_thread::sleep_for(std::chrono::milliseconds(1));
```

- 如果获取锁失败，则通过 `std::this_thread::sleep_for` 暂停 1 毫秒，避免忙等待，提高 CPU 的利用率。

**超时返回空字符串**

```cpp
return "";
```

- 如果在指定的 `acquireTimeout` 时间内没有成功获取锁，函数返回空字符串，表示获取锁失败。

### **4.3 释放锁（`releaseLock` 函数）**

释放锁的操作使用 Redis Lua 脚本，确保只有持有锁的客户端才能释放锁。脚本通过判断当前锁的持有者是否与传入的标识符一致来决定是否删除锁。

**Lua 脚本：**

```lua
if redis.call('get', KEYS[1]) == ARGV[1] then
    return redis.call('del', KEYS[1])
else
    return 0
end
```

- `KEYS[1]`：锁的 key（例如 `lock:my_resource`）。
- `ARGV[1]`：客户端在加锁时生成的唯一标识符。
- 如果当前锁的值（标识符）与传入的标识符一致，删除该锁。

Lua 脚本的作用是：

1. `redis.call('get', KEYS[1])`：从 Redis 获取 `lockKey` 对应的值。
2. `if redis.call('get', KEYS[1]) == ARGV[1]`：检查获取到的值是否与传入的 `identifier` 相同，只有标识符匹配时才能删除锁。
3. `return redis.call('del', KEYS[1])`：如果匹配，执行删除操作，释放锁。
4. `else return 0`：如果标识符不匹配，返回 `0`，表示没有成功释放锁。

**代码：**

```cpp
// 释放锁，只有锁的持有者才能释放，返回是否成功
bool releaseLock(redisContext* context, const std::string& lockName, const std::string& identifier) {
    std::string lockKey = "lock:" + lockName;
    // Lua 脚本：判断锁标识是否匹配，匹配则删除锁
    const char* luaScript = "if redis.call('get', KEYS[1]) == ARGV[1] then \
                                return redis.call('del', KEYS[1]) \
                             else \
                                return 0 \
                             end";
    // 调用 EVAL 命令执行 Lua 脚本，第一个参数为脚本，后面依次为 key 的数量、key 以及对应的参数
    redisReply* reply = (redisReply*)redisCommand(context, "EVAL %s 1 %s %s", luaScript, lockKey.c_str(), identifier.c_str());
    bool success = false;
    if (reply != nullptr) {
        // 当返回整数值为 1 时，表示成功删除了锁
        if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1) {
            success = true;
        }
        freeReplyObject(reply);
    }
    return success;
}
```



**函数参数说明**

- `redisContext* context`：指向 Redis 连接上下文的指针，用于执行 Redis 命令。
- `const std::string& lockName`：锁的名称，用于生成 Redis 键的名称。
- `const std::string& identifier`：标识符，用于标识哪个客户端持有锁。

函数返回一个布尔值，通常表示释放锁操作是否成功。



**执行 Lua 脚本**

```cpp
redisReply* reply = (redisReply*)redisCommand(context, "EVAL %s 1 %s %s", luaScript, lockKey.c_str(), identifier.c_str());
```

- 使用 `redisCommand` 函数执行 Redis 的 `EVAL` 命令，传入脚本、键的数量（在这里是 1，因为只有一个 `lockKey`），然后依次传入 `lockKey` 和 `identifier`。
- `redisCommand` 会返回一个 `redisReply` 指针，表示命令的返回结果。



**处理返回结果**

```cpp
cpp复制bool success = false;
if (reply != nullptr) {
    // 当返回整数值为 1 时，表示成功删除了锁
    if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1) {
        success = true;
    }
    freeReplyObject(reply);
}
```

- 如果返回的 `redisReply` 不为空，表示 Redis 执行了命令。
- 检查返回值的类型是否是整数（`REDIS_REPLY_INTEGER`），并且它的值是否是 1。如果是 1，表示删除锁成功，将 `success` 设置为 `true`。
- 释放 `redisReply` 对象，防止内存泄漏。



### 4.4 主函数（`main` 函数）

主函数执行以下操作：

1. 创建 Redis 客户端并连接到 Redis 服务器。
2. 尝试加锁，若成功获取锁，则执行临界区代码。
3. 在临界区代码执行完后，释放锁。

**代码：**

```cpp
int main() {
    // 连接到 Redis 服务器（根据实际情况修改主机和端口）
    redisContext* context = redisConnect("127.0.0.1", 6379);
    if (context == nullptr || context->err) {
        if (context) {
            std::cerr << "连接错误: " << context->errstr << std::endl;
            redisFree(context);
        } else {
            std::cerr << "无法分配 redis context" << std::endl;
        }
        return 1;
    }
    
    // 尝试获取锁（锁有效期 10 秒，获取超时时间 5 秒）
    std::string lockId = acquireLock(context, "my_resource", 10, 5);
    if (!lockId.empty()) {
        std::cout << "子进程 " << GetCurrentProcessId() << " 成功获取锁，锁 ID: " << lockId << std::endl;
        // 执行需要保护的临界区代码
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // 释放锁
        if (releaseLock(context, "my_resource", lockId)) {
            std::cout << "成功释放锁" << std::endl;
        } else {
            std::cout << "释放锁失败" << std::endl;
        }
    } else {
        std::cout << "获取锁失败" << std::endl;
    }
    
    // 释放 Redis 连接
    redisFree(context);
    return 0;
}
```

------

## 5. 封装为单例类操作

**类声明如下**

``` cpp
#include <string>
#include <hiredis.h>
class DistLock
{
public:
	static DistLock& Inst();
	~DistLock();
	std::string acquireLock(redisContext* context, const std::string& lockName, 
		int lockTimeout, int acquireTimeout);

	bool releaseLock(redisContext* context, const std::string& lockName, 
		const std::string& identifier);
private:
	DistLock() = default;
};
```

**类定义如下**

``` cpp
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <hiredis.h>

DistLock& DistLock::Inst() {
	static DistLock lock;
	return lock;
}


DistLock::~DistLock() {

}

// 尝试获取锁，返回锁的唯一标识符（UUID），如果获取失败则返回空字符串
std::string DistLock::acquireLock(redisContext* context, const std::string& lockName, 
    int lockTimeout, int acquireTimeout) {
    std::string identifier = generateUUID();
    std::string lockKey = "lock:" + lockName;
    auto endTime = std::chrono::steady_clock::now() + std::chrono::seconds(acquireTimeout);

    while (std::chrono::steady_clock::now() < endTime) {
        // 使用 SET 命令尝试加锁：SET lockKey identifier NX EX lockTimeout
        redisReply* reply = (redisReply*)redisCommand(context, "SET %s %s NX EX %d", 
            lockKey.c_str(), identifier.c_str(), lockTimeout);
        if (reply != nullptr) {
            // 判断返回结果是否为 OK
            if (reply->type == REDIS_REPLY_STATUS && std::string(reply->str) == "OK") {
                freeReplyObject(reply);
                return identifier;
            }
            freeReplyObject(reply);
        }
        // 暂停 1 毫秒后重试，防止忙等待
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return "";
}

// 释放锁，只有锁的持有者才能释放，返回是否成功
bool DistLock::releaseLock(redisContext* context, const std::string& lockName, 
    const std::string& identifier) {
    std::string lockKey = "lock:" + lockName;
    // Lua 脚本：判断锁标识是否匹配，匹配则删除锁
    const char* luaScript = "if redis.call('get', KEYS[1]) == ARGV[1] then \
                                return redis.call('del', KEYS[1]) \
                             else \
                                return 0 \
                             end";
    // 调用 EVAL 命令执行 Lua 脚本，第一个参数为脚本，后面依次为 key 的数量、key 以及对应的参数
    redisReply* reply = (redisReply*)redisCommand(context, "EVAL %s 1 %s %s", 
        luaScript, lockKey.c_str(), identifier.c_str());
    bool success = false;
    if (reply != nullptr) {
        // 当返回整数值为 1 时，表示成功删除了锁
        if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1) {
            success = true;
        }
        freeReplyObject(reply);
    }
    return success;
}
```

**测试用例**

``` cpp
int TestDisLock() {
    // 连接到 Redis 服务器（根据实际情况修改主机和端口）
    redisContext* context = redisConnect("81.68.86.146", 6380);
    if (context == nullptr || context->err) {
        if (context) {
            std::cerr << "连接错误: " << context->errstr << std::endl;
            redisFree(context);
        }
        else {
            std::cerr << "无法分配 redis context" << std::endl;
        }
        return 1;
    }

    std::string redis_password = "123456";
    redisReply* r = (redisReply*)redisCommand(context, "AUTH %s", redis_password.c_str());
    if (r->type == REDIS_REPLY_ERROR) {
        printf("Redis认证失败！\n");
    }
    else {
        printf("Redis认证成功！\n");
    }

    // 尝试获取锁（锁有效期 10 秒，获取超时时间 5 秒）
    std::string lockId = DistLock::Inst().acquireLock(context, "my_resource", 10, 5);
    
    if (!lockId.empty()) {
        std::cout << "子进程 " << GetCurrentProcessId() << " 成功获取锁，锁 ID: " << lockId << std::endl;
        // 执行需要保护的临界区代码
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // 释放锁
        if (DistLock::Inst().releaseLock(context, "my_resource", lockId)) {
            std::cout << "子进程 " << GetCurrentProcessId() << " 成功释放锁" << std::endl;
        }
        else {
            std::cout << "子进程 " << GetCurrentProcessId() << " 释放锁失败" << std::endl;
        }
    }
    else {
        std::cout << "子进程 " << GetCurrentProcessId() << " 获取锁失败" << std::endl;
    }

    // 释放 Redis 连接
    redisFree(context);
}
```

## 6. 多进程测试

我们可以创建另一个项目，调用之前生成好的`distribute.exe`

``` cpp
#include <windows.h>
#include <iostream>
#include <vector>

int main() {
    const int numProcesses = 5; // 需要启动 5 个子进程
    std::vector<PROCESS_INFORMATION> procInfos;

    for (int i = 0; i < numProcesses; i++) {
        STARTUPINFO si = { 0 };
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi = { 0 };

        // 这里假设 ChildTest.exe 在当前目录下
        if (CreateProcess(TEXT("DistributeLock.exe"),   // 应用程序名
            NULL,                    // 命令行参数
            NULL,                    // 进程句柄不可继承
            NULL,                    // 线程句柄不可继承
            FALSE,                   // 不继承句柄
            0,                       // 没有特殊创建标志
            NULL,                    // 使用父进程的环境
            NULL,                    // 使用父进程的当前目录
            &si,                     // 指向 STARTUPINFO 结构体的指针
            &pi))                    // 指向 PROCESS_INFORMATION 结构体的指针
        {
            std::cout << "成功创建子进程, PID: " << pi.dwProcessId << std::endl;
            procInfos.push_back(pi);
        }
        else {
            std::cerr << "创建子进程失败: " << GetLastError() << std::endl;
        }
    }

    // 等待所有子进程结束
    for (auto& pi : procInfos) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    std::cout << "所有子进程已结束" << std::endl;
    system("pause");
    return 0;
}

```

测试效果

![image-20250331215038920](https://cdn.llfc.club/img/image-20250331215038920.png)

## 7. 总结

- **分布式锁**：使用 Redis 和 Boost UUID 实现了一个简单的分布式锁，确保多个客户端可以同步地访问共享资源。
- **加锁**：通过 Redis 的 `SET` 命令，使用 `NX` 和 `EX` 参数确保只有一个客户端可以成功加锁。
- **解锁**：通过 Lua 脚本确保只有锁的持有者能够释放锁，避免其他客户端误释放锁。
- **持有者标识符**：每个客户端在加锁时生成一个唯一的标识符（UUID），它作为锁的持有者标识。
