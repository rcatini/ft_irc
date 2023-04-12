#include <iostream>
#include <cstdlib>
#include <csignal>
#include "server.hpp"

static bool should_shutdown = false;

void signal_handler(int signal)
{
	if (signal == SIGINT)
		should_shutdown = true;
}

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

	std::signal(SIGINT, signal_handler);
	std::string password = argv[2];
	try
	{
		Server server(should_shutdown, port, password);
		server.run();
	}
	catch (std::runtime_error &e)
	{
		std::cout << "Unhandled exception: " << e.what() << std::endl;
	}
	std::cout << "Server stopped" << std::endl;
}
