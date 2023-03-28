#include <iostream>
#include <cstdlib>

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cout << "Usage: " << argv[0] << "./ircserv <port> <password>" << std::endl;
		return 1;
	}

	char *endpointer;
	int port = std::strtol(argv[1], &endpointer, 10);
	if (*endpointer != '\0' || port < 0 || port > 65535)
	{
		std::cout << "Invalid port number" << std::endl;
		return 1;
	}

	std::string password = argv[2];
}
