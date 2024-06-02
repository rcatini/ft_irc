#pragma once

#define EOL "\r\n"
#include <string>
#include <queue>
#include <sstream>

class User
{
	std::string incoming_buffer;
	std::queue<std::string> incoming_messages;

public:
	void receive_data(std::string const &data);
};
