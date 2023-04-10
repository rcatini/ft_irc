#include <stdexcept>
#include <string>
#include <cerrno>
#include <cstring>
#include <iostream>
#include "client.hpp"

Client::Client(int connection_descriptor) : connection_descriptor(connection_descriptor)
{
	std::cout << "Client constructed" << std::endl;
}

Client::~Client()
{
	std::cout << "Client destroyed" << std::endl;
}