// StatusServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "const.h"
#include "ConfigMgr.h"
#include "hiredis.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"
#include "AsioIOServicePool.h"
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <boost/asio.hpp>
#include "StatusServiceImpl.h"
void RunServer() {
	auto & cfg = ConfigMgr::Inst();
	
	std::string server_address(cfg["StatusServer"]["Host"]+":"+ cfg["StatusServer"]["Port"]);
	StatusServiceImpl service;

	grpc::ServerBuilder builder;
	// 监听端口和添加服务
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);

	// 构建并启动gRPC服务器
	std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;

	// 创建Boost.Asio的io_context
	boost::asio::io_context io_context;
	// 创建signal_set用于捕获SIGINT
	boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);

	// 设置异步等待SIGINT信号
	signals.async_wait([&server, &io_context](const boost::system::error_code& error, int signal_number) {
		if (!error) {
			std::cout << "Shutting down server..." << std::endl;
			server->Shutdown(); // 优雅地关闭服务器
			io_context.stop(); // 停止io_context
		}
		});

	// 在单独的线程中运行io_context
	std::thread([&io_context]() { io_context.run(); }).detach();

	// 等待服务器关闭
	server->Wait();

}

int main(int argc, char** argv) {
	try {
		RunServer();
		RedisMgr::GetInstance()->Close();
	}
	catch (std::exception const& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		RedisMgr::GetInstance()->Close();
		return EXIT_FAILURE;
	}

	return 0;
}


