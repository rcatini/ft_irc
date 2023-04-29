#ifndef SERVER_HPP
#define SERVER_HPP
#include <string>
#include <netinet/in.h>
#include <map>
#include "user.hpp"
#include "channel.hpp"

class Server
{
	int port;
	std::string password;
	int fd;
	bool &teardown;
	struct sockaddr address;
	socklen_t address_length;
	std::map<int, User> users;
	std::map<std::string, Channel> channels;
	int handle_server_event(struct epoll_event event);
	void handle_user_event(struct epoll_event event);
	void disconnect_user(int user_descriptor);
	void signal_handler(int signal);

public:
	Server(bool &shutdown_ref, int port = 6667, std::string password = "");
	~Server();
	void run();
};

#endif