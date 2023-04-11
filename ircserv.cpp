#include <iostream>
#include <cstdlib>
#include <csignal>
#include "server.hpp"

void empty_handler(int) {}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cout << "Usage: " << argv[0] << "./ircserv <port> <password>" << std::endl;
		return 1;
	}

	char *endpointer;
	int port = std::strtol(argv[1], &endpointer, 10);
	if (*endpointer != '\0' || port <= 0 || port > 65535)
	{
		std::cout << "Invalid port number" << std::endl;
		return 1;
	}

	std::string password = argv[2];
	std::signal(SIGINT, empty_handler);
	try
	{
		Server(port, password).run();
	}
	catch (std::runtime_error &e)
	{
		std::cout << e.what() << std::endl;
	}
	std::cout << "Server stopped" << std::endl;
}
