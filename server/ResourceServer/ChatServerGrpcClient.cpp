#include "ChatServerGrpcClient.h"
#include "MysqlMgr.h"

NotifyChatImgRsp  ChatServerGrpcClient::NotifyChatImgMsg(int message_id,std::string chatserver)
{
	ClientContext context;
	NotifyChatImgRsp reply;
	NotifyChatImgReq request;
	request.set_message_id(message_id);
	if (_hash_pools.find(chatserver) == _hash_pools.end()) {
		reply.set_error(ErrorCodes::ServerIpErr);
		return reply;
	}
	auto chat_msg = MysqlMgr::GetInstance()->GetChatMsgById(message_id);
	request.set_file_name(chat_msg->content);
	request.set_from_uid(chat_msg->sender_id);
	request.set_to_uid(chat_msg->recv_id);
	request.set_thread_id(chat_msg->thread_id);
    // 资源文件路径
	auto file_dir = ConfigMgr::Inst().GetFileOutPath();
	//该消息是接收方客户端发送过来的,服务器将资源存储在发送方的文件夹中
	auto uid_str = std::to_string(chat_msg->sender_id);
	auto file_path = (file_dir / uid_str / chat_msg->content);
	boost::uintmax_t file_size = boost::filesystem::file_size(file_path);
	request.set_total_size(file_size);

	auto &pool_ = _hash_pools[chatserver];
	auto stub = pool_->getConnection();
	Status status = stub->NotifyChatImgMsg(&context, request, &reply);
	Defer defer([&stub, &pool_, this]() {
		pool_->returnConnection(std::move(stub));
		});
	if (status.ok()) {
		return reply;
	}
	else {
		reply.set_error(ErrorCodes::RPCFailed);
		return reply;
	}
}

ChatServerGrpcClient::ChatServerGrpcClient()
{
	auto& gCfgMgr = ConfigMgr::Inst();
	std::string host1 = gCfgMgr["chatserver1"]["Host"];
	std::string port1 = gCfgMgr["chatserver1"]["Port"];
	_hash_pools["chatserver1"] = std::make_unique<ChatServerConPool>(5, host1, port1);
	
	std::string host2 = gCfgMgr["chatserver2"]["Host"];
	std::string port2 = gCfgMgr["chatserver2"]["Port"];
	_hash_pools["chatserver2"] = std::make_unique<ChatServerConPool>(5, host2, port2);
}
