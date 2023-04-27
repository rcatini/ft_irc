#ifndef SERVER_HPP
#define SERVER_HPP
#include <string>
#include <netinet/in.h>
#include <map>
#include "client.hpp"
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
	std::map<int, Client> clients;
	std::map<std::string, Channel> channels;
	std::map<std::string, User> users;
	int handle_server_event(struct epoll_event event);
	void handle_client_event(struct epoll_event event);
	void disconnect_client(int client_descriptor);
	void signal_handler(int signal);

public:
	Server(bool &shutdown_ref, int port = 6667, std::string password = "");
	~Server();
	void run();
};

#endif