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
	bool authenticated; // user has sent the PASS command
	// bool registered;	// user is registered with a nickname
	std::string nickname, username;
	std::string incoming_buffer, outgoing_buffer;
	std::queue<std::string> incoming_messages;

public:
	User(Server &s, int user_fd, struct sockaddr_in user_address, int epoll_fd);
	ssize_t receive_data();
	ssize_t send_data();
	void queue_message(const std::string &message);
	bool authenticate(const std::string &password);
	std::string const &get_nickname() const;
	bool set_nickname(const std::string &nick);
	std::string const get_address();
};
