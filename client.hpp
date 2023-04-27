#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>
#include <sys/socket.h>
#include "user.hpp"

class Client
{
	std::string buffer;
	std::vector<std::string> outgoing_messages;

public:
	int read(int fd);
	int write(int fd);
	std::vector<std::string> parse_buffer();
	void connect(int *fd);
	void quit();
};

#endif