## VerifyServer增加redis

我们为了让验证码有一个过期时间，可以利用redis实现，在Verify文件夹用npm安装redis服务。
``` bash
npm install ioredis
```
完善config.json
``` json
{
    "email": {
      "user": "secondtonone1@163.com",
      "pass": "CRWTAZOSNCWDDQQTllfc"
    },

    "mysql": {
      "host": "81.68.86.146",
      "port": 3308,
      "passwd": "123456"
    },

    "redis":{
      "host": "81.68.86.146",
      "port": 6380,
      "passwd": "123456"
    }
}
```

服务里添加redis模块，封装redis操作在redis.js中
``` js

const config_module = require('./config')
const Redis = require("ioredis");

// 创建Redis客户端实例
const RedisCli = new Redis({
  host: config_module.redis_host,       // Redis服务器主机名
  port: config_module.redis_port,        // Redis服务器端口号
  password: config_module.redis_passwd, // Redis密码
});


/**
 * 监听错误信息
 */
RedisCli.on("error", function (err) {
  console.log("RedisCli connect error");
  RedisCli.quit();
});

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

/**
 * 退出函数
 */
function Quit(){
    RedisCli.quit();
}

module.exports = {GetRedis, QueryRedis, Quit, SetRedisExpire,}
```
server.js中包含redis.js
``` js
const redis_module = require('./redis')
```
获取验证码之前可以先查询redis，如果没查到就生成uid并且写入redis
``` js
async function GetVarifyCode(call, callback) {
    console.log("email is ", call.request.email)
    try{
        let query_res = await redis_module.GetRedis(const_module.code_prefix+call.request.email);
        console.log("query_res is ", query_res)
        if(query_res == null){

        }
        let uniqueId = query_res;
        if(query_res ==null){
            uniqueId = uuidv4();
            if (uniqueId.length > 4) {
                uniqueId = uniqueId.substring(0, 4);
            } 
            let bres = await redis_module.SetRedisExpire(const_module.code_prefix+call.request.email, uniqueId,600)
            if(!bres){
                callback(null, { email:  call.request.email,
                    error:const_module.Errors.RedisErr
                });
                return;
            }
        }

        console.log("uniqueId is ", uniqueId)
        let text_str =  '您的验证码为'+ uniqueId +'请三分钟内完成注册'
        //发送邮件
        let mailOptions = {
            from: 'secondtonone1@163.com',
            to: call.request.email,
            subject: '验证码',
            text: text_str,
        };
    
        let send_res = await emailModule.SendMail(mailOptions);
        console.log("send res is ", send_res)

        callback(null, { email:  call.request.email,
            error:const_module.Errors.Success
        }); 
        
 
    }catch(error){
        console.log("catch error is ", error)

        callback(null, { email:  call.request.email,
            error:const_module.Errors.Exception
        }); 
    }
     
}
```

## 验证服务联调

开启VerifyServer和GateServer，再启动客户端，点击获取验证码，客户端就会发送请求给GateServer，GateServer再调用内部服务VerifyServer。最后将请求返回客户端，完成了验证码发送的流程。

如果10分钟之内多次请求，因为验证码被缓存在redis中，所以会被复用返回给客户端。

![https://cdn.llfc.club/1711079058273.jpg](https://cdn.llfc.club/1711079058273.jpg)

看起来客户端收到服务器的回复了，我们去邮箱看看是否收到验证码

![https://cdn.llfc.club/1711080431619.jpg](https://cdn.llfc.club/1711080431619.jpg)

确实收到了验证码。好的多服务调用实现了，大家可以把这个功能理解下，接下来去实现注册逻辑。