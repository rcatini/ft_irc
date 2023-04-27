#include <iostream>
#include <cstdlib>
#include <csignal>
#include "server.hpp"

static bool teardown = false;

void signal_handler(int signal)
{
	if (signal == SIGINT)
		teardown = true;
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << "./ircserv <port> <password>" << std::endl;
		return 1;
	}

	char *endpointer;
	int port = std::strtol(argv[1], &endpointer, 10);
	if (*endpointer != '\0' || port <= 0 || port > 65535)
	{
		std::cerr << "Invalid port number" << std::endl;
		return 2;
	}

	std::signal(SIGINT, signal_handler);
	std::string password = argv[2];
	try
	{
		Server server(teardown, port, password);
		server.run();
	}
	catch (std::runtime_error &e)
	{
		std::cout << "Unhandled exception: " << e.what() << std::endl;
	}
	std::cout << "Server stopped" << std::endl;
}
