## 认证服务
我们的认证服务要给邮箱发送验证码，所以用nodejs较为合适，nodejs是一门IO效率很高而且生态完善的语言，用到发送邮件的库也方便。

nodejs可以去官网下载[https://nodejs.org/en](https://nodejs.org/en)，一路安装就可以了

我们新建VarifyServer文件夹，在文件夹内部初始化server要用到的nodejs库的配置文件

``` bash
npm init
```
根据提示同意会创建一个package.json文件

![https://cdn.llfc.club/1710380349325.jpg](https://cdn.llfc.club/1710380349325.jpg)

接下来安装grpc-js包，也可以安装grpc，grpc是C++版本，grpc-js是js版本，C++版本停止维护了。所以用grpc-js版本。

安装过程出现了错误，因为淘宝镜像地址过期了

![https://cdn.llfc.club/1710381278285.jpg](https://cdn.llfc.club/1710381278285.jpg)

清除之前npm镜像地址

``` bash
npm cache clean --force
```

重新设置新的淘宝镜像

``` bash
npm config set registry https://registry.npmmirror.com
```

接着下载grpc-js就成功了

![https://cdn.llfc.club/1710381922730.jpg](https://cdn.llfc.club/1710381922730.jpg)

接着安装proto-loader用来动态解析proto文件

``` bash
npm install @grpc/proto-loader
```

![https://cdn.llfc.club/1710382537391.jpg](https://cdn.llfc.club/1710382537391.jpg)

我们再安装email处理的库

``` js
npm install nodemailer
```

我们将proto文件放入VarifyServer文件夹，并且新建一个proto.js用来解析proto文件

``` js
const path = require('path')
const grpc = require('@grpc/grpc-js')
const protoLoader = require('@grpc/proto-loader')

const PROTO_PATH = path.join(__dirname, 'message.proto')
const packageDefinition = protoLoader.loadSync(PROTO_PATH, { keepCase: true, longs: String, enums: String, defaults: true, oneofs: true })
const protoDescriptor = grpc.loadPackageDefinition(packageDefinition)

const message_proto = protoDescriptor.message

module.exports = message_proto
```

keepCase: 如果为 true，则保留字段名的原始大小写。如果为 false，则将所有字段名转换为驼峰命名法。

longs: 控制如何表示 Protocol Buffers 中的 long 类型。如果设置为 String，则长整数会被转换为字符串，以避免 JavaScript 中的整数溢出问题。

enums: 控制如何表示 Protocol Buffers 中的枚举类型。如果设置为 String，则枚举值会被转换为字符串。

defaults: 如果为 true，则为未明确设置的字段提供默认值。

oneofs: 如果为 true，则支持 Protocol Buffers 中的 oneof 特性。

在写代码发送邮件之前，我们先去邮箱开启smtp服务。我用的163邮箱，在邮箱设置中查找smtp服务器地址，需要开启smtp服务。这个是固定的，不需要修改。

网易163邮箱的 SMTP 服务器地址为: smtp.163.com

发送邮件，建议使用授权码(有的邮箱叫 独立密码)，确保邮箱密码的安全性。授权码在邮箱设置中进行设置。如果开启了授权码，发送邮件的时候，必须使用授权码。

这里设置开启smtp服务和授权码。我这里已经是设置好的。

![https://cdn.llfc.club/20210625165014232.png](https://cdn.llfc.club/20210625165014232.png)

新增一个授权码用于发邮件

![https://cdn.llfc.club/20210625165014232%20%282%29.png](https://cdn.llfc.club/20210625165014232%20%282%29.png)

## 读取配置

因为我们要实现参数可配置，所以要读取配置，先在文件夹内创建一个config.json文件
``` js
{
    "email": {
      "user": "secondtonone1@163.com",
      "pass": "CRWTAZOSNCWDDQQTllfc"
    },
}
```
user是我们得邮箱地址，pass是邮箱得授权码，只有有了授权码才能用代码发邮件。大家记得把授权码改为你们自己的，否则用我的无法发送成功。

另外我们也要用到一些常量和全局得变量，所以我们定义一个const.js

``` js
let code_prefix = "code_";

const Errors = {
    Success : 0,
    RedisErr : 1,
    Exception : 2,
};


module.exports = {code_prefix,Errors}
```
新建config.js用来读取配置
``` js
const fs = require('fs');

let config = JSON.parse(fs.readFileSync('config.json', 'utf8'));
let email_user = config.email.user;
let email_pass = config.email.pass;
let mysql_host = config.mysql.host;
let mysql_port = config.mysql.port;
let redis_host = config.redis.host;
let redis_port = config.redis.port;
let redis_passwd = config.redis.passwd;
let code_prefix = "code_";


module.exports = {email_pass, email_user, mysql_host, mysql_port,redis_host, redis_port, redis_passwd, code_prefix}
```

接下来封装发邮件的模块，新建一个email.js文件
``` js
const nodemailer = require('nodemailer');
const config_module = require("./config")

/**
 * 创建发送邮件的代理
 */
let transport = nodemailer.createTransport({
    host: 'smtp.163.com',
    port: 465,
    secure: true,
    auth: {
        user: config_module.email_user, // 发送方邮箱地址
        pass: config_module.email_pass // 邮箱授权码或者密码
    }
});
```
接下来实现发邮件函数
``` js
/**
 * 发送邮件的函数
 * @param {*} mailOptions_ 发送邮件的参数
 * @returns 
 */
function SendMail(mailOptions_){
    return new Promise(function(resolve, reject){
        transport.sendMail(mailOptions_, function(error, info){
            if (error) {
                console.log(error);
                reject(error);
            } else {
                console.log('邮件已成功发送：' + info.response);
                resolve(info.response)
            }
        });
    })
   
}

module.exports.SendMail = SendMail
```
因为transport.SendMail相当于一个异步函数，调用该函数后发送的结果是通过回调函数通知的，所以我们没办法同步使用，需要用Promise封装这个调用，抛出Promise给外部，那么外部就可以通过await或者then catch的方式处理了。

我们新建server.js，用来启动grpc server
``` cpp
async function GetVarifyCode(call, callback) {
    console.log("email is ", call.request.email)
    try{
        uniqueId = uuidv4();
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

function main() {
    var server = new grpc.Server()
    server.addService(message_proto.VarifyService.service, { GetVarifyCode: GetVarifyCode })
    server.bindAsync('0.0.0.0:50051', grpc.ServerCredentials.createInsecure(), () => {
        server.start()
        console.log('grpc server started')        
    })
}

main()
```
GetVarifyCode声明为async是为了能在内部调用await。

## 提升GateServer并发
添加ASIO IOContext Pool 结构，让多个iocontext跑在不同的线程中
``` cpp
#include <vector>
#include <boost/asio.hpp>
#include "Singleton.h"
class AsioIOServicePool:public Singleton<AsioIOServicePool>
{
	friend Singleton<AsioIOServicePool>;
public:
	using IOService = boost::asio::io_context;
	using Work = boost::asio::io_context::work;
	using WorkPtr = std::unique_ptr<Work>;
	~AsioIOServicePool();
	AsioIOServicePool(const AsioIOServicePool&) = delete;
	AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;
	// 使用 round-robin 的方式返回一个 io_service
	boost::asio::io_context& GetIOService();
	void Stop();
private:
	AsioIOServicePool(std::size_t size = 2/*std::thread::hardware_concurrency()*/);
	std::vector<IOService> _ioServices;
	std::vector<WorkPtr> _works;
	std::vector<std::thread> _threads;
	std::size_t                        _nextIOService;
};
```
实现
``` cpp
#include "AsioIOServicePool.h"
#include <iostream>
using namespace std;
AsioIOServicePool::AsioIOServicePool(std::size_t size):_ioServices(size),
_works(size), _nextIOService(0){
	for (std::size_t i = 0; i < size; ++i) {
		_works[i] = std::unique_ptr<Work>(new Work(_ioServices[i]));
	}

	//遍历多个ioservice，创建多个线程，每个线程内部启动ioservice
	for (std::size_t i = 0; i < _ioServices.size(); ++i) {
		_threads.emplace_back([this, i]() {
			_ioServices[i].run();
			});
	}
}

AsioIOServicePool::~AsioIOServicePool() {
	Stop();
	std::cout << "AsioIOServicePool destruct" << endl;
}

boost::asio::io_context& AsioIOServicePool::GetIOService() {
	auto& service = _ioServices[_nextIOService++];
	if (_nextIOService == _ioServices.size()) {
		_nextIOService = 0;
	}
	return service;
}

void AsioIOServicePool::Stop(){
	//因为仅仅执行work.reset并不能让iocontext从run的状态中退出
	//当iocontext已经绑定了读或写的监听事件后，还需要手动stop该服务。
	for (auto& work : _works) {
		//把服务先停止
		work->get_io_context().stop();
		work.reset();
	}

	for (auto& t : _threads) {
		t.join();
	}
}
```

修改CServer处Start逻辑, 改为每次从IOServicePool连接池中获取连接
``` cpp
void CServer::Start()
{	
	auto self = shared_from_this();
	auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();
	std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(io_context);
	_acceptor.async_accept(new_con->GetSocket(), [self, new_con](beast::error_code ec) {
		try {
			//出错则放弃这个连接，继续监听新链接
			if (ec) {
				self->Start();
				return;
			}

			//处理新链接，创建HpptConnection类管理新连接
			new_con->Start();
			//继续监听
			self->Start();
		}
		catch (std::exception& exp) {
			std::cout << "exception is " << exp.what() << std::endl;
			self->Start();
		}
	});
}
```

为了方便读取配置文件，将ConfigMgr改为单例, 将构造函数变成私有，添加Inst函数
``` cpp
static ConfigMgr& Inst() {
    static ConfigMgr cfg_mgr;
    return cfg_mgr;
}
```

VerifyGrpcClient.cpp中添加
``` cpp
class RPConPool {
public:
	RPConPool(size_t poolSize, std::string host, std::string port)
		: poolSize_(poolSize), host_(host), port_(port), b_stop_(false) {
		for (size_t i = 0; i < poolSize_; ++i) {
			
			std::shared_ptr<Channel> channel = grpc::CreateChannel(host+":"+port,
				grpc::InsecureChannelCredentials());

			connections_.push(VarifyService::NewStub(channel));
		}
	}

	~RPConPool() {
		std::lock_guard<std::mutex> lock(mutex_);
		Close();
		while (!connections_.empty()) {
			connections_.pop();
		}
	}

	std::unique_ptr<VarifyService::Stub> getConnection() {
		std::unique_lock<std::mutex> lock(mutex_);
		cond_.wait(lock, [this] {
			if (b_stop_) {
				return true;
			}
			return !connections_.empty();
			});
		//如果停止则直接返回空指针
		if (b_stop_) {
			return  nullptr;
		}
		auto context = std::move(connections_.front());
		connections_.pop();
		return context;
	}

	void returnConnection(std::unique_ptr<VarifyService::Stub> context) {
		std::lock_guard<std::mutex> lock(mutex_);
		if (b_stop_) {
			return;
		}
		connections_.push(std::move(context));
		cond_.notify_one();
	}

	void Close() {
		b_stop_ = true;
		cond_.notify_all();
	}

private:
	atomic<bool> b_stop_;
	size_t poolSize_;
	std::string host_;
	std::string port_;
	std::queue<std::unique_ptr<VarifyService::Stub>> connections_;
	std::mutex mutex_;
	std::condition_variable cond_;
};
```
我们在VerifyGrpcClient类中添加成员
``` cpp
std::unique_ptr<RPConPool> pool_;
```
修改构造函数
``` cpp
VerifyGrpcClient::VerifyGrpcClient() {
	auto& gCfgMgr = ConfigMgr::Inst();
	std::string host = gCfgMgr["VarifyServer"]["Host"];
	std::string port = gCfgMgr["VarifyServer"]["Port"];
	pool_.reset(new RPConPool(5, host, port));
}
```

当我们想连接grpc server端时，可以通过池子获取连接，用完之后再返回连接给池子
``` cpp
GetVarifyRsp GetVarifyCode(std::string email) {
    ClientContext context;
    GetVarifyRsp reply;
    GetVarifyReq request;
    request.set_email(email);
    auto stub = pool_->getConnection();
    Status status = stub->GetVarifyCode(&context, request, &reply);

    if (status.ok()) {
        pool_->returnConnection(std::move(stub));
        return reply;
    }
    else {
        pool_->returnConnection(std::move(stub));
        reply.set_error(ErrorCodes::RPCFailed);
        return reply;
    }
}
```

## 总结
到本节为止我们完成nodejs搭建的grpc server， 修改package.json中的脚本

``` cpp
  "scripts": {
    "serve": "node server.js"
  },
```
接着命令行执行 npm run serve即可启动grpc 服务。