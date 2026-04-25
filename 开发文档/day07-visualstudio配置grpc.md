##属性管理器

推荐一种可复制配置的方式，视图里选择其他窗口，再选择属性管理器

![https://cdn.llfc.club/1710292918199.jpg](https://cdn.llfc.club/1710292918199.jpg)

我们选择要配置的类型，我选择Debug 64位的配置，添加新项目属性表

![https://cdn.llfc.club/2789d4d0598e69bff5f0452159d3c14.png](https://cdn.llfc.club/2789d4d0598e69bff5f0452159d3c14.png)

选择创建属性的名字

![https://cdn.llfc.club/7675ab8ac46308693eec2ea4ec0f708.png](https://cdn.llfc.club/7675ab8ac46308693eec2ea4ec0f708.png)

接下来双击我们创建好的属性文件，将之前配置的boost和jsoncpp库属性移动到这里，把之前在项目中配置的删除。

包含目录

![https://cdn.llfc.club/3e98a4ba407416e8a433a7b6254c3a6.png](https://cdn.llfc.club/3e98a4ba407416e8a433a7b6254c3a6.png)

库目录

![https://cdn.llfc.club/56a894eca5a6b3888ba07f29678b291.png](https://cdn.llfc.club/56a894eca5a6b3888ba07f29678b291.png)

链接库

![https://cdn.llfc.club/43aba5606318b56dc56ba1a884c18b3.png](https://cdn.llfc.club/43aba5606318b56dc56ba1a884c18b3.png)

接下来配置grpc头文件包含目录，C++ 常规-> 附加包含目录添加如下

``` bash
D:\cppsoft\grpc\third_party\re2
D:\cppsoft\grpc\third_party\address_sorting\include
D:\cppsoft\grpc\third_party\abseil-cpp
D:\cppsoft\grpc\third_party\protobuf\src
D:\cppsoft\grpc\include
```

![https://cdn.llfc.club/375f8c4b21f643408b73a19e415fcd5.png](https://cdn.llfc.club/375f8c4b21f643408b73a19e415fcd5.png)

接下来配置库路径, 在链接器常规选项下，点击附加库目录，添加我们需要的库目录
``` bash
D:\cppsoft\grpc\visualpro\third_party\re2\Debug
D:\cppsoft\grpc\visualpro\third_party\abseil-cpp\absl\types\Debug
D:\cppsoft\grpc\visualpro\third_party\abseil-cpp\absl\synchronization\Debug
D:\cppsoft\grpc\visualpro\third_party\abseil-cpp\absl\status\Debug
D:\cppsoft\grpc\visualpro\third_party\abseil-cpp\absl\random\Debug
D:\cppsoft\grpc\visualpro\third_party\abseil-cpp\absl\flags\Debug
D:\cppsoft\grpc\visualpro\third_party\abseil-cpp\absl\debugging\Debug
D:\cppsoft\grpc\visualpro\third_party\abseil-cpp\absl\container\Debug
D:\cppsoft\grpc\visualpro\third_party\abseil-cpp\absl\hash\Debug
D:\cppsoft\grpc\visualpro\third_party\boringssl-with-bazel\Debug
D:\cppsoft\grpc\visualpro\third_party\abseil-cpp\absl\numeric\Debug
D:\cppsoft\grpc\visualpro\third_party\abseil-cpp\absl\time\Debug
D:\cppsoft\grpc\visualpro\third_party\abseil-cpp\absl\base\Debug
D:\cppsoft\grpc\visualpro\third_party\abseil-cpp\absl\strings\Debug
D:\cppsoft\grpc\visualpro\third_party\protobuf\Debug
D:\cppsoft\grpc\visualpro\third_party\zlib\Debug
D:\cppsoft\grpc\visualpro\Debug
D:\cppsoft\grpc\visualpro\third_party\cares\cares\lib\Debug
```

![https://cdn.llfc.club/89fcb7a4afef6721c893187fffcfecf.png](https://cdn.llfc.club/89fcb7a4afef6721c893187fffcfecf.png)

在链接器->输入->附加依赖项中添加

``` bash
libprotobufd.lib
gpr.lib
grpc.lib
grpc++.lib
grpc++_reflection.lib
address_sorting.lib
ws2_32.lib
cares.lib
zlibstaticd.lib
upb.lib
ssl.lib
crypto.lib
absl_bad_any_cast_impl.lib
absl_bad_optional_access.lib
absl_bad_variant_access.lib
absl_base.lib
absl_city.lib
absl_civil_time.lib
absl_cord.lib
absl_debugging_internal.lib
absl_demangle_internal.lib
absl_examine_stack.lib
absl_exponential_biased.lib
absl_failure_signal_handler.lib
absl_flags.lib
absl_flags_config.lib
absl_flags_internal.lib
absl_flags_marshalling.lib
absl_flags_parse.lib
absl_flags_program_name.lib
absl_flags_usage.lib
absl_flags_usage_internal.lib
absl_graphcycles_internal.lib
absl_hash.lib
absl_hashtablez_sampler.lib
absl_int128.lib
absl_leak_check.lib
absl_leak_check_disable.lib
absl_log_severity.lib
absl_malloc_internal.lib
absl_periodic_sampler.lib
absl_random_distributions.lib
absl_random_internal_distribution_test_util.lib
absl_random_internal_pool_urbg.lib
absl_random_internal_randen.lib
absl_random_internal_randen_hwaes.lib
absl_random_internal_randen_hwaes_impl.lib
absl_random_internal_randen_slow.lib
absl_random_internal_seed_material.lib
absl_random_seed_gen_exception.lib
absl_random_seed_sequences.lib
absl_raw_hash_set.lib
absl_raw_logging_internal.lib
absl_scoped_set_env.lib
absl_spinlock_wait.lib
absl_stacktrace.lib
absl_status.lib
absl_strings.lib
absl_strings_internal.lib
absl_str_format_internal.lib
absl_symbolize.lib
absl_synchronization.lib
absl_throw_delegate.lib
absl_time.lib
absl_time_zone.lib
absl_statusor.lib
re2.lib
```

![https://cdn.llfc.club/1710301100655.jpg](https://cdn.llfc.club/1710301100655.jpg)

之后点击保存会看到项目目录下生成了PropertySheet.props文件

## proto文件编写

在项目的根目录下创建一个proto名字为message.proto
``` bash
syntax = "proto3";

package message;

service VarifyService {
  rpc GetVarifyCode (GetVarifyReq) returns (GetVarifyRsp) {}
}

message GetVarifyReq {
  string email = 1;
}

message GetVarifyRsp {
  int32 error = 1;
  string email = 2;
  string code = 3;
}
```

接下来我们利用grpc编译后生成的proc.exe生成proto的grpc的头文件和源文件

``` bash
D:\cppsoft\grpc\visualpro\third_party\protobuf\Debug\protoc.exe  -I="." --grpc_out="." --plugin=protoc-gen-grpc="D:\cppsoft\grpc\visualpro\Debug\grpc_cpp_plugin.exe" "message.proto"
```
上述命令会生成message.grpc.pb.h和message.grpc.pb.cc文件。

接下来我们生成用于序列化和反序列化的pb文件

``` bash
D:\cppsoft\grpc\visualpro\third_party\protobuf\Debug\protoc.exe --cpp_out=. "message.proto"
```
上述命令会生成message.pb.h和message.pb.cc文件

接下来我们将这些pb.h和pb.cc放入项目中

我们新建一个VarifyGrpcClient类，vs帮我们自动生成头文件和源文件，我们在头文件添加Grpc客户端类
``` cpp
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "const.h"
#include "Singleton.h"
using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;

class VerifyGrpcClient:public Singleton<VerifyGrpcClient>
{
	friend class Singleton<VerifyGrpcClient>;
public:

	GetVarifyRsp GetVarifyCode(std::string email) {
		ClientContext context;
		GetVarifyRsp reply;
		GetVarifyReq request;
		request.set_email(email);

		Status status = stub_->GetVarifyCode(&context, request, &reply);

		if (status.ok()) {
			
			return reply;
		}
		else {
			reply.set_error(ErrorCodes::RPCFailed);
			return reply;
		}
	}

private:
	VerifyGrpcClient() {
		std::shared_ptr<Channel> channel = grpc::CreateChannel("127.0.0.1:50051", grpc::InsecureChannelCredentials());
		stub_ = VarifyService::NewStub(channel);
	}

	std::unique_ptr<VarifyService::Stub> stub_;
};
```

我们在之前收到post请求获取验证码的逻辑里添加处理
``` cpp
RegPost("/get_varifycode", [](std::shared_ptr<HttpConnection> connection) {
	auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
	std::cout << "receive body is " << body_str << std::endl;
	connection->_response.set(http::field::content_type, "text/json");
	Json::Value root;
	Json::Reader reader;
	Json::Value src_root;
	bool parse_success = reader.parse(body_str, src_root);
	if (!parse_success) {
		std::cout << "Failed to parse JSON data!" << std::endl;
		root["error"] = ErrorCodes::Error_Json;
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->_response.body()) << jsonstr;
		return true;
	}

	auto email = src_root["email"].asString();
	GetVarifyRsp rsp = VerifyGrpcClient::GetInstance()->GetVarifyCode(email);
	cout << "email is " << email << endl;
	root["error"] = rsp.error();
	root["email"] = src_root["email"];
	std::string jsonstr = root.toStyledString();
	beast::ostream(connection->_response.body()) << jsonstr;
	return true;
});
```

## 服务器读取配置

我们很多参数都是写死的，现通过配置文件读取以方便以后修改
在项目中添加config.ini文件

``` bash
[GateServer]
Port = 8080
[VarifyServer]
Port = 50051
```

添加ConfigMgr类用来读取和管理配置, 定义一个SectionInfo类管理key和value

``` cpp
struct SectionInfo {
	SectionInfo(){}
	~SectionInfo(){
		_section_datas.clear();
	}
	
	SectionInfo(const SectionInfo& src) {
		_section_datas = src._section_datas;
	}
	
	SectionInfo& operator = (const SectionInfo& src) {
		if (&src == this) {
			return *this;
		}

		this->_section_datas = src._section_datas;
	}

	std::map<std::string, std::string> _section_datas;
	std::string  operator[](const std::string  &key) {
		if (_section_datas.find(key) == _section_datas.end()) {
			return "";
		}
		// 这里可以添加一些边界检查  
		return _section_datas[key];
	}
};
```

定义ComigMgr管理section和其包含的key与value

``` cpp
class ConfigMgr
{
public:
	~ConfigMgr() {
		_config_map.clear();
	}
	SectionInfo operator[](const std::string& section) {
		if (_config_map.find(section) == _config_map.end()) {
			return SectionInfo();
		}
		return _config_map[section];
	}


	ConfigMgr& operator=(const ConfigMgr& src) {
		if (&src == this) {
			return *this;
		}

		this->_config_map = src._config_map;
	};

	ConfigMgr(const ConfigMgr& src) {
		this->_config_map = src._config_map;
	}

	ConfigMgr();
private:
	
	// 存储section和key-value对的map  
	std::map<std::string, SectionInfo> _config_map;
};
```

构造函数里实现config读取

``` cpp
ConfigMgr::ConfigMgr(){
	// 获取当前工作目录  
	boost::filesystem::path current_path = boost::filesystem::current_path();
	// 构建config.ini文件的完整路径  
	boost::filesystem::path config_path = current_path / "config.ini";
	std::cout << "Config path: " << config_path << std::endl;

	// 使用Boost.PropertyTree来读取INI文件  
	boost::property_tree::ptree pt;
	boost::property_tree::read_ini(config_path.string(), pt);


	// 遍历INI文件中的所有section  
	for (const auto& section_pair : pt) {
		const std::string& section_name = section_pair.first;
		const boost::property_tree::ptree& section_tree = section_pair.second;

		// 对于每个section，遍历其所有的key-value对  
		std::map<std::string, std::string> section_config;
		for (const auto& key_value_pair : section_tree) {
			const std::string& key = key_value_pair.first;
			const std::string& value = key_value_pair.second.get_value<std::string>();
			section_config[key] = value;
		}
		SectionInfo sectionInfo;
		sectionInfo._section_datas = section_config;
		// 将section的key-value对保存到config_map中  
		_config_map[section_name] = sectionInfo;
	}

	// 输出所有的section和key-value对  
	for (const auto& section_entry : _config_map) {
		const std::string& section_name = section_entry.first;
		SectionInfo section_config = section_entry.second;
		std::cout << "[" << section_name << "]" << std::endl;
		for (const auto& key_value_pair : section_config._section_datas) {
			std::cout << key_value_pair.first << "=" << key_value_pair.second << std::endl;
		}
	}

}
```

在const.h里声明一个全局变量

``` cpp
class ConfigMgr;
extern ConfigMgr gCfgMgr;
```

接下来在main函数中将8080端口改为从配置读取

``` cpp
ConfigMgr gCfgMgr;
std::string gate_port_str = gCfgMgr["GateServer"]["Port"];
unsigned short gate_port = atoi(gate_port_str.c_str());
```

其他地方想要获取配置信息就不需要定义了，直接包含const.h并且使用gCfgMgr即可。

## 总结

本节基于visual studio配置grpc，并实现了grpc客户端发送请求的逻辑。下一节实现 grpc server



