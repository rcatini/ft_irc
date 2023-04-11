#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <sys/socket.h>
#include "user.hpp"

class Client : public User
{
public:
	Client();
	~Client();
};

#endif