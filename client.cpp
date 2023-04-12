#include <stdexcept>
#include <string>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include "client.hpp"

int Client::read(int fd)
{
	std::cout << "Client::read() called" << std::endl;
	char buffer[512];
	int bytes_read;
	while ((bytes_read = recv(fd, buffer, 512, MSG_DONTWAIT)) > 0)
		(std::cout << "Read " << bytes_read << " bytes from socket " << fd << ": ").write(buffer, bytes_read) << std::endl;
	if (bytes_read < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
		return 0;
	return bytes_read;
}

int Client::write(int fd)
{
	std::cout << "Client::write() called" << std::endl;
	int bytes_written = send(fd, "Hello world!", 12, MSG_DONTWAIT);
	if (bytes_written < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
		return 0;
	return bytes_written;
}