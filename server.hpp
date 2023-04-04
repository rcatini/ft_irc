#ifndef SERVER_HPP
#define SERVER_HPP
#include <string>
#include <netinet/in.h>
#include <map>
#include "client.hpp"

class Server 
{
	int port;
	std::string password;
	int socket_descriptor;
	struct sockaddr_in address;
	socklen_t address_length;
	std::map<int, Client> clients;
public:
	Server(int port = 6667, std::string password = "");
	void run();
};

#endif