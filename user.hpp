#pragma once

#define EOL "\r\n"
#define EOL_SIZE 2
#define MAX_MESSAGE_SIZE 512

#include <string>
#include <queue>
#include <sstream>
#include <sys/epoll.h>
#include <netinet/in.h>
#include "server.hpp"

class Server;

class User
{
	Server &server;
	int fd;
	struct sockaddr_in address;
	int epoll_fd;
	bool authenticated;
	std::string nickname, username;
	std::string incoming_buffer, outgoing_buffer;
	std::queue<std::string> incoming_messages;
	bool authenticate(const std::string &password);

public:
	User(Server &s, int user_fd, struct sockaddr_in user_address, int epoll_fd);
	ssize_t receive_data();
	ssize_t send_data();
	void queue_message(const std::string &message);
	std::string const get_address();
};
