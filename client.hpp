#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <sys/socket.h>
#include "user.hpp"

class Client
{

public:
	int read(int fd);
	int write(int fd);
	void connect(int *fd);
	void quit();
};

#endif