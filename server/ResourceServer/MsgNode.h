#pragma once
#include <string>
#include "const.h"
#include <iostream>
#include <boost/asio.hpp>
using namespace std;
using boost::asio::ip::tcp;
class LogicSystem;
class MsgNode
{
public:
	MsgNode(int max_len) :_total_len(max_len), _cur_len(0) {
		_data = new char[_total_len + 1]();
		_data[_total_len] = '\0';
	}

	~MsgNode() {
		std::cout << "destruct MsgNode" << endl;
		delete[] _data;
	}

	void Clear() {
		::memset(_data, 0, _total_len);
		_cur_len = 0;
	}

	int _cur_len;
	int _total_len;
	char* _data;
};

class RecvNode :public MsgNode {
public:
	RecvNode(int max_len, short msg_id);
	short _msg_id;
};

class SendNode:public MsgNode {
public:
	SendNode(const char* msg,int max_len, short msg_id);
	short _msg_id;
};

