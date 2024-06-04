#pragma once

#define EOL "\r\n"
#define MAX_MESSAGE_SIZE 512

#include <string>
#include <queue>
#include <sstream>
#include <sys/epoll.h>

class User
{
	int fd;
	int epoll_fd;
	std::string incoming_buffer;
	std::string outgoing_buffer;
	std::queue<std::string> incoming_messages;

public:
	User(int user_fd, int epoll_fd);
	ssize_t receive_data();
	ssize_t send_data();
	void queue_message(const std::string &message);
};
