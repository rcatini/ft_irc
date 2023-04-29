#ifndef USER_HPP
#define USER_HPP

#include <string>
#include <list>
#include <sys/socket.h>
#include "user.hpp"

class User
{
	int fd;						 // File descriptor for socket
	std::string incoming_buffer; // Buffer for incoming messages
	std::string outgoing_buffer; // Buffer for outgoing messages

public:
	User(int fd);						   // Constructor
	~User();							   // Destructor
	User(const User &other);			   // Copy constructor
	int read();							   // Read data from socket
	int write();						   // Write data to socket
	std::list<std::string> get_messages(); // Parse incoming buffer into messages
	void put_message(std::string message); // Put message in outgoing buffer
	bool has_messages();				   // Check if there are messages in outgoing buffer
	void connect(int *fd);				   // Connect to socket
	void quit();						   // Quit connection
};

#endif