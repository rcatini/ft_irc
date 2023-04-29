#ifndef USER_HPP
#define USER_HPP

#include <string>
#include <vector>
#include <sys/socket.h>
#include "user.hpp"

class User
{
	int fd;										// File descriptor for socket
	std::string incoming_buffer;				// Buffer for incoming data
	std::vector<std::string> outgoing_messages; // Queue of outgoing messages

public:
	User(int fd);									  // Constructor
	int read();										  // Read data from socket
	int write();									  // Write data to socket
	std::vector<std::string> parse_incoming_buffer(); // Parse incoming buffer into messages
	void connect(int *fd);							  // Connect to socket
	void quit();									  // Quit connection
};

#endif