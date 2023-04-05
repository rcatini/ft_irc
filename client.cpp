#include <stdexcept>
#include <string>
#include <cerrno>
#include <cstring>
#include <iostream>
#include "client.hpp"

Client::Client(int connection_descriptor) : connection_descriptor(connection_descriptor)
{
}

Client::~Client()
{
	std::cout << "Closing connection" << std::endl;
}