#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <sys/socket.h>

class Client
{
	std::string name;
	std::string host;
	int connection_descriptor;
public:
	Client(int connection_descriptor);
	~Client();
};


#endif