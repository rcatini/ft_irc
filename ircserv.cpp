#include <iostream>
#include <cstdio>
#include <climits>
#include "server.hpp"

volatile sig_atomic_t signal_status;

void signal_handler(int sig)
{
	signal_status = sig;
}

unsigned short check_args_and_extract_port(int argc, char *argv[])
{
	if (argc != 3)
		throw std::runtime_error("Usage: " + std::string(argv[0]) + " <port> <password>");

	std::istringstream iss(argv[1]);
	int port;
	if (!(iss >> port) || (port <= 0) || (port > USHRT_MAX) || iss.peek() != EOF)
		throw std::runtime_error("Invalid port: " + std::string(argv[1]));

	return (unsigned short)port;
}

int main(int argc, char *argv[])
{
	signal(SIGINT, signal_handler);
	try
	{
		unsigned short port = check_args_and_extract_port(argc, argv);
		Server server((unsigned short)port, argv[2], signal_status);
		server.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
}
