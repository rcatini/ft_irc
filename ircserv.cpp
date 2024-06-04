#include <iostream>
#include <cstdio>
#include <climits>
#include "server.hpp"

// global variable to store the signal status (automatically initialized to 0)
volatile sig_atomic_t signal_status;

// signal handler: store the signal status in the global variable
void signal_handler(int sig)
{
	signal_status = sig;
}

// check the program arguments and extract the port
unsigned short check_args_and_extract_port(int argc, char *argv[])
{
	// check if the two arguments are present
	if (argc != 3)
		throw std::runtime_error("Usage: " + std::string(argv[0]) + " <port> <password>");

	// check if the first argument is a valid TCP port number
	std::istringstream iss(argv[1]);
	int port;
	if (!(iss >> port) || (port <= 0) || (port > USHRT_MAX) || iss.peek() != EOF)
		throw std::runtime_error("Invalid port: " + std::string(argv[1]));

	return (unsigned short)port;
}

int main(int argc, char *argv[])
{
	// register the signal handler for SIGINT
	signal(SIGINT, signal_handler);

	try
	{
		unsigned short port = check_args_and_extract_port(argc, argv);
		Server server(port, argv[2], signal_status);
		server.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Exception caught in main: " << e.what() << std::endl;
		return 1;
	}
}
