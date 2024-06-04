#pragma once

#define CONNECTION_QUEUE_SIZE 10

#include <string>
#include <map>
#include <csignal>
#include <netinet/in.h>
#include "user.hpp"

class Server
{
	short port;
	std::string password;
	int fd;
	struct sockaddr_in address;
	volatile sig_atomic_t &signal;
	std::map<int, User> users;
	void broadcast(const std::string &message);

public:
	Server(unsigned short port, const std::string &password,
		   volatile sig_atomic_t &signal);
	~Server();
	void run();
};
