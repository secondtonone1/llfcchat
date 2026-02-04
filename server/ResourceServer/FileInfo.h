#pragma once
#include <string>

class FileInfo {
public:
	FileInfo(int seq = 0, std::string name = "", int64_t total_size = 0,
		int64_t trans_size = 0, std::string file_path_str = "")
		:_seq(seq), _name(name), _total_size(total_size),
		_trans_size(trans_size), _file_path_str(file_path_str) {}
	int _seq;
	std::string _name;
	int64_t _total_size;
	int64_t _trans_size;
	std::string _file_path_str;
};

class ChatImgInfo {
public:
	ChatImgInfo(int sender_id,  int receiver_id, int message_id, std::string img_name)
	:_sender_id(sender_id),_receiver_id(receiver_id),_message_id(message_id),_img_name(img_name){}
	int _sender_id;
	int _receiver_id;
	int _message_id;
	std::string _img_name;
};
