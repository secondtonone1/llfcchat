## 邮箱验证服务联调

我们启动GateServer和VarifyServer

我们启动客户端，点击注册按钮进入注册界面，输入邮箱并且点击获取验证码

![https://cdn.llfc.club/1710646053282.jpg](https://cdn.llfc.club/1710646053282.jpg)

GateServer收到Client发送的请求后，会调用grpc 服务 访问VarifyServer，VarifyServer会随机生成验证码，并且调用邮箱模块发送邮件给指定邮箱。而且把发送的结果给GateServer，GateServer再将消息回传给客户端。

## 设置验证码过期

我们的验证码是要设置过期的，可以用redis管理过期的验证码自动删除，key为邮箱，value为验证码，过期时间为3min。

## windows 安装redis服务

windows 版本下载地址:

[https://github.com/tporadowski/redis/releases](https://github.com/tporadowski/redis/releases)

下载速度慢可以去我的网盘

链接: https://pan.baidu.com/s/1v_foHZLvBeJQMePSGnp4Ow?pwd=yid3 提取码: yid3 

下载完成后解压

![https://cdn.llfc.club/1710649614458.jpg](https://cdn.llfc.club/1710649614458.jpg)

修改redis.windows.conf, 并且修改端口
``` cpp
port 6380
```
找到requirepass foobared，下面添加requirepass
``` cpp
# requirepass foobared
requirepass 123456
```
启动redis 服务器` .\redis-server.exe .\redis.windows.conf`

![https://cdn.llfc.club/1710649945760.jpg](https://cdn.llfc.club/1710649945760.jpg)

启动客户端 `.\redis-cli.exe -p 6380`, 输入密码登录成功

![https://cdn.llfc.club/1710650063208.jpg](https://cdn.llfc.club/1710650063208.jpg)

## Linux 安装redis服务

Linux安装容器后，直接用容器启动redis
``` bash
docker run -d --name llfc-redis -p 6380:6379 redis  --requirepass "123456"
```
为了方便测试能否链接以及以后查看数据,大家可以下载redis desktop manager

官网链接
[redisdesktop.com/](redisdesktop.com/)

下载速度慢可以去我的网盘

链接: https://pan.baidu.com/s/1v_foHZLvBeJQMePSGnp4Ow?pwd=yid3 提取码: yid3 

下载后安装

设置好ip和密码，点击测试连接连通就成功了

![https://cdn.llfc.club/1710657223612.jpg](https://cdn.llfc.club/1710657223612.jpg)

## widows编译和配置redis

Linux的redis库直接编译安装即可，windows反而麻烦一些，我们先阐述windows环境如何配置redis库， C++ 的redis库有很多种，最常用的有hredis和redis-plus-plus. 我们用redis-plus-plus. 这里介绍一种简单的安装方式---vcpkg

先安装vcpkg, 源码地址

[https://github.com/microsoft/vcpkg/releases](https://github.com/microsoft/vcpkg/releases)

下载源码后

windows版本redis下载地址

[https://github.com/microsoftarchive/redis](https://github.com/microsoftarchive/redis)

因为是源码，所以进入msvc目录

![https://cdn.llfc.club/1710725726234.jpg](https://cdn.llfc.club/1710725726234.jpg)

用visual studio打开sln文件，弹出升级窗口, 我的是vs2019所以升级到142

![https://cdn.llfc.club/1710725937787.jpg](https://cdn.llfc.club/1710725937787.jpg)

只需要生成hiredis工程和Win32_Interop工程即可，分别点击生成,生成hiredis.lib和Win32_Interop.lib即可

右键两个工程的属性，代码生成里选择运行时库加载模式为MDD(Debug模式动态运行加载)，为了兼容我们其他的库，其他的库也是MDD模式

![https://cdn.llfc.club/1710726777016.jpg](https://cdn.llfc.club/1710726777016.jpg)

编译Win32_Interop.lib时报错， system_error不是std成员，

![https://cdn.llfc.club/1710727129177.jpg](https://cdn.llfc.club/1710727129177.jpg)

解决办法为在Win32_variadicFunctor.cpp和Win32_FDAPI.cpp添加
`#include <system_error>`,再右键生成成功

![https://cdn.llfc.club/1710729372811.jpg](https://cdn.llfc.club/1710729372811.jpg)

将hiredis.lib和Win32_Interop.lib拷贝到`D:\cppsoft\reids\lib`

将`redis-3.0\deps`和`redis-3.0\src`文件夹拷贝到`D:\cppsoft\reids`

然后我们在visual studio中配置VC++ 包含目录

![https://cdn.llfc.club/1710811823982.jpg](https://cdn.llfc.club/1710811823982.jpg)

配置VC++库目录

![https://cdn.llfc.club/1710811986563.jpg](https://cdn.llfc.club/1710811986563.jpg)

然后在链接器->输入->附加依赖项中添加

![https://cdn.llfc.club/1710812099185.jpg](https://cdn.llfc.club/1710812099185.jpg)

## 代码测试

我们需要写代码测试库配置的情况

``` cpp
void TestRedis() {
	//连接redis 需要启动才可以进行连接
//redis默认监听端口为6387 可以再配置文件中修改
	redisContext* c = redisConnect("127.0.0.1", 6380);
	if (c->err)
	{
		printf("Connect to redisServer faile:%s\n", c->errstr);
		redisFree(c);        return;
	}
	printf("Connect to redisServer Success\n");

	std::string redis_password = "123456";
	redisReply* r = (redisReply*)redisCommand(c, "AUTH %s", redis_password);
	 if (r->type == REDIS_REPLY_ERROR) {
		 printf("Redis认证失败！\n");
	}else {
		printf("Redis认证成功！\n");
		 }

	//为redis设置key
	const char* command1 = "set stest1 value1";

	//执行redis命令行
    r = (redisReply*)redisCommand(c, command1);

	//如果返回NULL则说明执行失败
	if (NULL == r)
	{
		printf("Execut command1 failure\n");
		redisFree(c);        return;
	}

	//如果执行失败则释放连接
	if (!(r->type == REDIS_REPLY_STATUS && (strcmp(r->str, "OK") == 0 || strcmp(r->str, "ok") == 0)))
	{
		printf("Failed to execute command[%s]\n", command1);
		freeReplyObject(r);
		redisFree(c);        return;
	}

	//执行成功 释放redisCommand执行后返回的redisReply所占用的内存
	freeReplyObject(r);
	printf("Succeed to execute command[%s]\n", command1);

	const char* command2 = "strlen stest1";
	r = (redisReply*)redisCommand(c, command2);

	//如果返回类型不是整形 则释放连接
	if (r->type != REDIS_REPLY_INTEGER)
	{
		printf("Failed to execute command[%s]\n", command2);
		freeReplyObject(r);
		redisFree(c);        return;
	}

	//获取字符串长度
	int length = r->integer;
	freeReplyObject(r);
	printf("The length of 'stest1' is %d.\n", length);
	printf("Succeed to execute command[%s]\n", command2);

	//获取redis键值对信息
	const char* command3 = "get stest1";
	r = (redisReply*)redisCommand(c, command3);
	if (r->type != REDIS_REPLY_STRING)
	{
		printf("Failed to execute command[%s]\n", command3);
		freeReplyObject(r);
		redisFree(c);        return;
	}
	printf("The value of 'stest1' is %s\n", r->str);
	freeReplyObject(r);
	printf("Succeed to execute command[%s]\n", command3);

	const char* command4 = "get stest2";
	r = (redisReply*)redisCommand(c, command4);
	if (r->type != REDIS_REPLY_NIL)
	{
		printf("Failed to execute command[%s]\n", command4);
		freeReplyObject(r);
		redisFree(c);        return;
	}
	freeReplyObject(r);
	printf("Succeed to execute command[%s]\n", command4);

	//释放连接资源
	redisFree(c);

}
```
在主函数中调用TestRedis，编译项目时发现编译失败，提示

![https://cdn.llfc.club/1710812579501.jpg](https://cdn.llfc.club/1710812579501.jpg)

在同时使用Redis连接和socket连接时，遇到了Win32_Interop.lib和WS2_32.lib冲突的问题, 因为我们底层用了socket作为网络通信，也用redis，导致两个库冲突。

引起原因主要是Redis库Win32_FDAPI.cpp有重新定义了socket的一些方法引起来冲突

``` cpp
extern "C" {
// Unix compatible FD based routines
fdapi_accept accept = NULL;
fdapi_access access = NULL;
fdapi_bind bind = NULL;
fdapi_connect connect = NULL;
fdapi_fcntl fcntl = NULL;
fdapi_fstat fdapi_fstat64 = NULL;
fdapi_fsync fsync = NULL;
fdapi_ftruncate ftruncate = NULL;
fdapi_freeaddrinfo freeaddrinfo = NULL;
fdapi_getaddrinfo getaddrinfo = NULL;
fdapi_getpeername getpeername = NULL;
fdapi_getsockname getsockname = NULL;
fdapi_getsockopt getsockopt = NULL;
fdapi_htonl htonl = NULL;
fdapi_htons htons = NULL;
fdapi_isatty isatty = NULL;
fdapi_inet_ntop inet_ntop = NULL;
fdapi_inet_pton inet_pton = NULL;
fdapi_listen listen = NULL;
fdapi_lseek64 lseek64 = NULL;
fdapi_ntohl ntohl = NULL;
fdapi_ntohs ntohs = NULL;
fdapi_open open = NULL;
fdapi_pipe pipe = NULL;
fdapi_poll poll = NULL;
fdapi_read read = NULL;
fdapi_select select = NULL;
fdapi_setsockopt setsockopt = NULL;
fdapi_socket socket = NULL;
fdapi_write write = NULL;
}
auto f_WSACleanup = dllfunctor_stdcall<int>("ws2_32.dll", "WSACleanup");
auto f_WSAFDIsSet = dllfunctor_stdcall<int, SOCKET, fd_set*>("ws2_32.dll", "__WSAFDIsSet");
auto f_WSAGetLastError = dllfunctor_stdcall<int>("ws2_32.dll", "WSAGetLastError");
auto f_WSAGetOverlappedResult = dllfunctor_stdcall<BOOL, SOCKET, LPWSAOVERLAPPED, LPDWORD, BOOL, LPDWORD>("ws2_32.dll", "WSAGetOverlappedResult");
auto f_WSADuplicateSocket = dllfunctor_stdcall<int, SOCKET, DWORD, LPWSAPROTOCOL_INFO>("ws2_32.dll", "WSADuplicateSocketW");
auto f_WSAIoctl = dllfunctor_stdcall<int, SOCKET, DWORD, LPVOID, DWORD, LPVOID, DWORD, LPVOID, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE>("ws2_32.dll", "WSAIoctl");
auto f_WSARecv = dllfunctor_stdcall<int, SOCKET, LPWSABUF, DWORD, LPDWORD, LPDWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE>("ws2_32.dll", "WSARecv");
auto f_WSASocket = dllfunctor_stdcall<SOCKET, int, int, int, LPWSAPROTOCOL_INFO, GROUP, DWORD>("ws2_32.dll", "WSASocketW");
auto f_WSASend = dllfunctor_stdcall<int, SOCKET, LPWSABUF, DWORD, LPDWORD, DWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE>("ws2_32.dll", "WSASend");
auto f_WSAStartup = dllfunctor_stdcall<int, WORD, LPWSADATA>("ws2_32.dll", "WSAStartup");
auto f_ioctlsocket = dllfunctor_stdcall<int, SOCKET, long, u_long*>("ws2_32.dll", "ioctlsocket");

auto f_accept = dllfunctor_stdcall<SOCKET, SOCKET, struct sockaddr*, int*>("ws2_32.dll", "accept");
auto f_bind = dllfunctor_stdcall<int, SOCKET, const struct sockaddr*, int>("ws2_32.dll", "bind");
auto f_closesocket = dllfunctor_stdcall<int, SOCKET>("ws2_32.dll", "closesocket");
auto f_connect = dllfunctor_stdcall<int, SOCKET, const struct sockaddr*, int>("ws2_32.dll", "connect");
auto f_freeaddrinfo = dllfunctor_stdcall<void, addrinfo*>("ws2_32.dll", "freeaddrinfo");
auto f_getaddrinfo = dllfunctor_stdcall<int, PCSTR, PCSTR, const ADDRINFOA*, ADDRINFOA**>("ws2_32.dll", "getaddrinfo");
auto f_gethostbyname = dllfunctor_stdcall<struct hostent*, const char*>("ws2_32.dll", "gethostbyname");
auto f_getpeername = dllfunctor_stdcall<int, SOCKET, struct sockaddr*, int*>("ws2_32.dll", "getpeername");
auto f_getsockname = dllfunctor_stdcall<int, SOCKET, struct sockaddr*, int*>("ws2_32.dll", "getsockname");
auto f_getsockopt = dllfunctor_stdcall<int, SOCKET, int, int, char*, int*>("ws2_32.dll", "getsockopt");
auto f_htonl = dllfunctor_stdcall<u_long, u_long>("ws2_32.dll", "htonl");
auto f_htons = dllfunctor_stdcall<u_short, u_short>("ws2_32.dll", "htons");
auto f_listen = dllfunctor_stdcall<int, SOCKET, int>("ws2_32.dll", "listen");
auto f_ntohs = dllfunctor_stdcall<u_short, u_short>("ws2_32.dll", "ntohs");
auto f_ntohl = dllfunctor_stdcall<u_long, u_long>("ws2_32.dll", "ntohl");
auto f_recv = dllfunctor_stdcall<int, SOCKET, char*, int, int>("ws2_32.dll", "recv");
auto f_select = dllfunctor_stdcall<int, int, fd_set*, fd_set*, fd_set*, const struct timeval*>("ws2_32.dll", "select");
auto f_send = dllfunctor_stdcall<int, SOCKET, const char*, int, int>("ws2_32.dll", "send");
auto f_setsockopt = dllfunctor_stdcall<int, SOCKET, int, int, const char*, int>("ws2_32.dll", "setsockopt");
auto f_socket = dllfunctor_stdcall<SOCKET, int, int, int>("ws2_32.dll", "socket");

```
去掉Redis库里面的socket的函数的重定义，把所有使用这些方法的地方都改为下面对应的函数

``` cpp
int FDAPI_accept(int rfd, struct sockaddr *addr, socklen_t *addrlen);
int FDAPI_access(const char *pathname, int mode);
int FDAPI_bind(int rfd, const struct sockaddr *addr, socklen_t addrlen);
int FDAPI_connect(int rfd, const struct sockaddr *addr, size_t addrlen);
int FDAPI_fcntl(int rfd, int cmd, int flags);
int FDAPI_fstat64(int rfd, struct __stat64 *buffer);
void FDAPI_freeaddrinfo(struct addrinfo *ai);
int FDAPI_fsync(int rfd);
int FDAPI_ftruncate(int rfd, PORT_LONGLONG length);
int FDAPI_getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
int FDAPI_getsockopt(int rfd, int level, int optname, void *optval, socklen_t *optlen);
int FDAPI_getpeername(int rfd, struct sockaddr *addr, socklen_t * addrlen);
int FDAPI_getsockname(int rfd, struct sockaddr* addrsock, int* addrlen);
u_long FDAPI_htonl(u_long hostlong);
u_short FDAPI_htons(u_short hostshort);
u_int FDAPI_ntohl(u_int netlong);
u_short FDAPI_ntohs(u_short netshort);
int FDAPI_open(const char * _Filename, int _OpenFlag, int flags);
int FDAPI_pipe(int *pfds);
int FDAPI_poll(struct pollfd *fds, nfds_t nfds, int timeout);
int FDAPI_listen(int rfd, int backlog);
int FDAPI_socket(int af, int type, int protocol);
int FDAPI_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
int FDAPI_setsockopt(int rfd, int level, int optname, const void *optval, socklen_t optlen);
ssize_t FDAPI_read(int rfd, void *buf, size_t count);
ssize_t FDAPI_write(int rfd, const void *buf, size_t count);
```
考虑大家修改起来很麻烦，可以下载我的代码

[https://gitee.com/secondtonone1/windows-redis](https://gitee.com/secondtonone1/windows-redis)

再次编译生成hredis和Win32_Interop的lib库，重新配置下，项目再次编译就通过了。

## 封装redis操作类

因为hredis提供的操作太别扭了，我们手动封装redis操作类，简化调用流程。

封装的类叫RedisMgr，它是个单例类并且可接受回调，按照我们之前的风格
``` cpp
class RedisMgr: public Singleton<RedisMgr>, 
	public std::enable_shared_from_this<RedisMgr>
{
	friend class Singleton<RedisMgr>;
public:
	~RedisMgr();
	bool Connect(const std::string& host, int port);
	bool Get(const std::string &key, std::string& value);
	bool Set(const std::string &key, const std::string &value);
	bool Auth(const std::string &password);
	bool LPush(const std::string &key, const std::string &value);
	bool LPop(const std::string &key, std::string& value);
	bool RPush(const std::string& key, const std::string& value);
	bool RPop(const std::string& key, std::string& value);
	bool HSet(const std::string &key, const std::string  &hkey, const std::string &value);
	bool HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen);
	std::string HGet(const std::string &key, const std::string &hkey);
	bool Del(const std::string &key);
	bool ExistsKey(const std::string &key);
	void Close();
private:
	RedisMgr();

	redisContext* _connect;
	redisReply* _reply;
};
```

连接操作

``` cpp
bool RedisMgr::Connect(const std::string &host, int port)
{
	this->_connect = redisConnect(host.c_str(), port);
	if (this->_connect != NULL && this->_connect->err)
	{
		std::cout << "connect error " << this->_connect->errstr << std::endl;
		return false;
	}
	return true;
}
```

获取key对应的value

``` cpp
bool RedisMgr::Get(const std::string &key, std::string& value)
{
	 this->_reply = (redisReply*)redisCommand(this->_connect, "GET %s", key.c_str());
	 if (this->_reply == NULL) {
		 std::cout << "[ GET  " << key << " ] failed" << std::endl;
		 freeReplyObject(this->_reply);
		  return false;
	}

	 if (this->_reply->type != REDIS_REPLY_STRING) {
		 std::cout << "[ GET  " << key << " ] failed" << std::endl;
		 freeReplyObject(this->_reply);
		 return false;
	}

	 value = this->_reply->str;
	 freeReplyObject(this->_reply);

	 std::cout << "Succeed to execute command [ GET " << key << "  ]" << std::endl;
	 return true;
}
```

设置key和value

``` cpp
bool RedisMgr::Set(const std::string &key, const std::string &value){
	//执行redis命令行

	this->_reply = (redisReply*)redisCommand(this->_connect, "SET %s %s", key.c_str(), value.c_str());

	//如果返回NULL则说明执行失败
	if (NULL == this->_reply)
	{
		std::cout << "Execut command [ SET " << key << "  "<< value << " ] failure ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}

	//如果执行失败则释放连接
	if (!(this->_reply->type == REDIS_REPLY_STATUS && (strcmp(this->_reply->str, "OK") == 0 || strcmp(this->_reply->str, "ok") == 0)))
	{
		std::cout << "Execut command [ SET " << key << "  " << value << " ] failure ! " << std::endl;
		freeReplyObject(this->_reply);     
		return false;
	}

	//执行成功 释放redisCommand执行后返回的redisReply所占用的内存
	freeReplyObject(this->_reply);
	std::cout << "Execut command [ SET " << key << "  " << value << " ] success ! " << std::endl;
	return true;
}
```

密码认证

``` cpp
bool RedisMgr::Auth(const std::string &password)
{
	this->_reply = (redisReply*)redisCommand(this->_connect, "AUTH %s", password.c_str());
	if (this->_reply->type == REDIS_REPLY_ERROR) {
		std::cout << "认证失败" << std::endl;
		//执行成功 释放redisCommand执行后返回的redisReply所占用的内存
		freeReplyObject(this->_reply);
		return false;
	}
	else {
		//执行成功 释放redisCommand执行后返回的redisReply所占用的内存
		freeReplyObject(this->_reply);
		std::cout << "认证成功" << std::endl;
		return true;
	}
}
```

左侧push

``` cpp
bool RedisMgr::LPush(const std::string &key, const std::string &value)
{
	this->_reply = (redisReply*)redisCommand(this->_connect, "LPUSH %s %s", key.c_str(), value.c_str());
	if (NULL == this->_reply)
	{
		std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] failure ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}

	if (this->_reply->type != REDIS_REPLY_INTEGER || this->_reply->integer <= 0) {
		std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] failure ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}

	std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] success ! " << std::endl;
	freeReplyObject(this->_reply);
	return true;
}
```

左侧pop

``` cpp
bool RedisMgr::LPop(const std::string &key, std::string& value){
	this->_reply = (redisReply*)redisCommand(this->_connect, "LPOP %s ", key.c_str());
	if (_reply == nullptr || _reply->type == REDIS_REPLY_NIL) {
		std::cout << "Execut command [ LPOP " << key<<  " ] failure ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}
	value = _reply->str;
	std::cout << "Execut command [ LPOP " << key <<  " ] success ! " << std::endl;
	freeReplyObject(this->_reply);
	return true;
}
```

右侧push

``` cpp
bool RedisMgr::RPush(const std::string& key, const std::string& value) {
	this->_reply = (redisReply*)redisCommand(this->_connect, "RPUSH %s %s", key.c_str(), value.c_str());
	if (NULL == this->_reply)
	{
		std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] failure ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}

	if (this->_reply->type != REDIS_REPLY_INTEGER || this->_reply->integer <= 0) {
		std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] failure ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}

	std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] success ! " << std::endl;
	freeReplyObject(this->_reply);
	return true;
}
```

右侧pop

``` cpp
bool RedisMgr::RPop(const std::string& key, std::string& value) {
	this->_reply = (redisReply*)redisCommand(this->_connect, "RPOP %s ", key.c_str());
	if (_reply == nullptr || _reply->type == REDIS_REPLY_NIL) {
		std::cout << "Execut command [ RPOP " << key << " ] failure ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}
	value = _reply->str;
	std::cout << "Execut command [ RPOP " << key << " ] success ! " << std::endl;
	freeReplyObject(this->_reply);
	return true;
}
```

HSet操作

``` cpp
bool RedisMgr::HSet(const std::string &key, const std::string &hkey, const std::string &value) {
	this->_reply = (redisReply*)redisCommand(this->_connect, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
	if (_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER ) {
		std::cout << "Execut command [ HSet " << key << "  " << hkey <<"  " << value << " ] failure ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}
	std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << value << " ] success ! " << std::endl;
	freeReplyObject(this->_reply);
	return true;
}


bool RedisMgr::HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen)
{
	 const char* argv[4];
	 size_t argvlen[4];
	 argv[0] = "HSET";
	argvlen[0] = 4;
	argv[1] = key;
	argvlen[1] = strlen(key);
	argv[2] = hkey;
	argvlen[2] = strlen(hkey);
	argv[3] = hvalue;
	argvlen[3] = hvaluelen;
	this->_reply = (redisReply*)redisCommandArgv(this->_connect, 4, argv, argvlen);
	if (_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER) {
		std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue << " ] failure ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}
	std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue << " ] success ! " << std::endl;
	freeReplyObject(this->_reply);
	return true;
}
```

HGet操作

``` cpp
std::string RedisMgr::HGet(const std::string &key, const std::string &hkey)
{
	const char* argv[3];
	size_t argvlen[3];
	argv[0] = "HGET";
	argvlen[0] = 4;
	argv[1] = key.c_str();
	argvlen[1] = key.length();
	argv[2] = hkey.c_str();
	argvlen[2] = hkey.length();
	this->_reply = (redisReply*)redisCommandArgv(this->_connect, 3, argv, argvlen);
	if (this->_reply == nullptr || this->_reply->type == REDIS_REPLY_NIL) {
		freeReplyObject(this->_reply);
		std::cout << "Execut command [ HGet " << key << " "<< hkey <<"  ] failure ! " << std::endl;
		return "";
	}

	std::string value = this->_reply->str;
	freeReplyObject(this->_reply);
	std::cout << "Execut command [ HGet " << key << " " << hkey << " ] success ! " << std::endl;
	return value;
}
```

Del 操作

``` cpp
bool RedisMgr::Del(const std::string &key)
{
	this->_reply = (redisReply*)redisCommand(this->_connect, "DEL %s", key.c_str());
	if (this->_reply == nullptr || this->_reply->type != REDIS_REPLY_INTEGER) {
		std::cout << "Execut command [ Del " << key <<  " ] failure ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}
	std::cout << "Execut command [ Del " << key << " ] success ! " << std::endl;
	 freeReplyObject(this->_reply);
	 return true;
}
```

判断键值是否存在

``` cpp
bool RedisMgr::ExistsKey(const std::string &key)
{
	this->_reply = (redisReply*)redisCommand(this->_connect, "exists %s", key.c_str());
	if (this->_reply == nullptr || this->_reply->type != REDIS_REPLY_INTEGER || this->_reply->integer == 0) {
		std::cout << "Not Found [ Key " << key << " ]  ! " << std::endl;
		freeReplyObject(this->_reply);
		return false;
	}
	std::cout << " Found [ Key " << key << " ] exists ! " << std::endl;
	freeReplyObject(this->_reply);
	return true;
}
```

关闭

``` cpp
void RedisMgr::Close()
{
	redisFree(_connect);
}
```

测试用例

``` cpp
void TestRedisMgr() {
	assert(RedisMgr::GetInstance()->Connect("127.0.0.1", 6380));
	assert(RedisMgr::GetInstance()->Auth("123456"));
	assert(RedisMgr::GetInstance()->Set("blogwebsite","llfc.club"));
	std::string value="";
	assert(RedisMgr::GetInstance()->Get("blogwebsite", value) );
	assert(RedisMgr::GetInstance()->Get("nonekey", value) == false);
	assert(RedisMgr::GetInstance()->HSet("bloginfo","blogwebsite", "llfc.club"));
	assert(RedisMgr::GetInstance()->HGet("bloginfo","blogwebsite") != "");
	assert(RedisMgr::GetInstance()->ExistsKey("bloginfo"));
	assert(RedisMgr::GetInstance()->Del("bloginfo"));
	assert(RedisMgr::GetInstance()->Del("bloginfo"));
	assert(RedisMgr::GetInstance()->ExistsKey("bloginfo") == false);
	assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue1"));
	assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue2"));
	assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue3"));
	assert(RedisMgr::GetInstance()->RPop("lpushkey1", value));
	assert(RedisMgr::GetInstance()->RPop("lpushkey1", value));
	assert(RedisMgr::GetInstance()->LPop("lpushkey1", value));
	assert(RedisMgr::GetInstance()->LPop("lpushkey2", value)==false);
	RedisMgr::GetInstance()->Close();
}
```
## 封装redis连接池
``` cpp
class RedisConPool {
public:
	RedisConPool(size_t poolSize, const char* host, int port, const char* pwd)
		: poolSize_(poolSize), host_(host), port_(port), b_stop_(false){
		for (size_t i = 0; i < poolSize_; ++i) {
			auto* context = redisConnect(host, port);
			if (context == nullptr || context->err != 0) {
				if (context != nullptr) {
					redisFree(context);
				}
				continue;
			}

			auto reply = (redisReply*)redisCommand(context, "AUTH %s", pwd);
			if (reply->type == REDIS_REPLY_ERROR) {
				std::cout << "认证失败" << std::endl;
				//执行成功 释放redisCommand执行后返回的redisReply所占用的内存
				freeReplyObject(reply);
				continue;
			}

			//执行成功 释放redisCommand执行后返回的redisReply所占用的内存
			freeReplyObject(reply);
			std::cout << "认证成功" << std::endl;
			connections_.push(context);
		}

	}

	~RedisConPool() {
		std::lock_guard<std::mutex> lock(mutex_);
		while (!connections_.empty()) {
			connections_.pop();
		}
	}

	redisContext* getConnection() {
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
		auto* context = connections_.front();
		connections_.pop();
		return context;
	}

	void returnConnection(redisContext* context) {
		std::lock_guard<std::mutex> lock(mutex_);
		if (b_stop_) {
			return;
		}
		connections_.push(context);
		cond_.notify_one();
	}

	void Close() {
		b_stop_ = true;
		cond_.notify_all();
	}

private:
	atomic<bool> b_stop_;
	size_t poolSize_;
	const char* host_;
	int port_;
	std::queue<redisContext*> connections_;
	std::mutex mutex_;
	std::condition_variable cond_;
};
```

RedisMgr构造函数中初始化pool连接池
``` cpp
RedisMgr::RedisMgr() {
	auto& gCfgMgr = ConfigMgr::Inst();
	auto host = gCfgMgr["Redis"]["Host"];
	auto port = gCfgMgr["Redis"]["Port"];
	auto pwd = gCfgMgr["Redis"]["Passwd"];
	_con_pool.reset(new RedisConPool(5, host.c_str(), atoi(port.c_str()), pwd.c_str()));
}
```
在析构函数中回收资源
``` cpp
RedisMgr::~RedisMgr() {
	Close();
}

void RedisMgr::Close() {
	_con_pool->Close();
}
```

在使用的时候改为从Pool中获取链接
``` cpp
bool RedisMgr::Get(const std::string& key, std::string& value)
{
	auto connect = _con_pool->getConnection();
	if (connect == nullptr) {
		return false;
	}
	 auto reply = (redisReply*)redisCommand(connect, "GET %s", key.c_str());
	 if (reply == NULL) {
		 std::cout << "[ GET  " << key << " ] failed" << std::endl;
		 freeReplyObject(reply);
		 _con_pool->returnConnection(connect);
		  return false;
	}

	 if (reply->type != REDIS_REPLY_STRING) {
		 std::cout << "[ GET  " << key << " ] failed" << std::endl;
		 freeReplyObject(reply);
		 _con_pool->returnConnection(connect);
		 return false;
	}

	 value = reply->str;
	 freeReplyObject(reply);

	 std::cout << "Succeed to execute command [ GET " << key << "  ]" << std::endl;
	 _con_pool->returnConnection(connect);
	 return true;
}
```
## 总结

本节告诉大家如何搭建redis服务，linux和windows环境的，并且编译了windows版本的hredis库，解决了链接错误，而且封装了RedisMgr管理类。
并实现了测试用例，大家感兴趣可以测试一下。下一节实现VarifyServer访问的redis功能。