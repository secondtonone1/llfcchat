
const config_module = require('./config')
const Redis = require("ioredis");

// 创建Redis客户端实例
const RedisCli = new Redis({
  host: config_module.redis_host,       // Redis服务器主机名
  port: config_module.redis_port,        // Redis服务器端口号
  password: config_module.redis_passwd, // Redis密码

  enableOfflineQueue: false, // 禁用离线队列
  enableReadyCheck: true,    // 启用连接就绪检查
});


// 监听连接错误事件
RedisCli.on("error", function (err) {
  console.error("Redis connection error:", err);
  // 尝试重新连接
  RedisCli.connect();
});

// 监听连接断开事件
RedisCli.on("end", function () {
  console.log("Redis connection closed");
  // 尝试重新连接
  RedisCli.connect();
});

// 心跳机制：定时发送心跳消息
setInterval(() => {
  // 发送心跳消息，比如向一个特定的 key 写入当前时间戳
  RedisCli.set("heartbeat", Date.now());
}, 60000); // 每 5 秒发送一次心跳消息

/**
 * 根据key获取value
 * @param {*} key 
 * @returns 
 */
async function GetRedis(key) {
    
    try{
        const result = await RedisCli.get(key)
        if(result === null){
          console.log('result:','<'+result+'>', 'This key cannot be find...')
          return null
        }
        console.log('Result:','<'+result+'>','Get key success!...');
        return result
    }catch(error){
        console.log('GetRedis error is', error);
        return null
    }

  }

/**
 * 根据key查询redis中是否存在key
 * @param {*} key 
 * @returns 
 */
async function QueryRedis(key) {
    try{
        const result = await RedisCli.exists(key)
        //  判断该值是否为空 如果为空返回null
        if (result === 0) {
          console.log('result:<','<'+result+'>','This key is null...');
          return null
        }
        console.log('Result:','<'+result+'>','With this value!...');
        return result
    }catch(error){
        console.log('QueryRedis error is', error);
        return null
    }

  }

/**
 * 设置key和value，并过期时间
 * @param {*} key 
 * @param {*} value 
 * @param {*} exptime 
 * @returns 
 */
async function SetRedisExpire(key,value, exptime){
    try{
        // 设置键和值
        await RedisCli.set(key,value)
        // 设置过期时间（以秒为单位）
        await RedisCli.expire(key, exptime);
        return true;
    }catch(error){
        console.log('SetRedisExpire error is', error);
        return false;
    }
}


module.exports = {GetRedis, QueryRedis,SetRedisExpire,}