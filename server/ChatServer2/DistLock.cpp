#include "DistLock.h"
#include <thread>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <hiredis.h>


//定义单例模式
DistLock& DistLock::Inst() {
	static DistLock lock;
	return lock;
}

// 使用 Boost UUID 生成全局唯一标识符（UUID）
static std::string generateUUID() {
	boost::uuids::uuid uuid = boost::uuids::random_generator()();
	return to_string(uuid);
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