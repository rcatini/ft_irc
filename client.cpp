#include <stdexcept>
#include <string>
#include <cerrno>
#include <cstring>
#include <iostream>
#include "client.hpp"

Client::Client()
{
	std::cout << "Client constructed" << std::endl;
}

Client::~Client()
{
	std::cout << "Client destroyed" << std::endl;
}