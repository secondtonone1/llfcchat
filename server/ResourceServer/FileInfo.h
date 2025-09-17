#pragma once
#include <string>

class FileInfo {
public:
	FileInfo(int seq = 0, std::string name = "", int total_size = 0,
		int trans_size = 0, std::string file_path_str = "")
		:_seq(seq), _name(name), _total_size(total_size),
		_trans_size(trans_size), _file_path_str(file_path_str) {}
	int _seq;
	std::string _name;
	int _total_size;
	int _trans_size;
	std::string _file_path_str;
};
