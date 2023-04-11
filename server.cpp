#include "server.hpp"
#include <string>
#include <iostream>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>

Server::Server(int port, std::string password) : port(port), password(password)
{
	if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		throw std::runtime_error("Could not create socket: " + std::string(strerror(errno)));
	}

	int opt = 1;
	if (setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	{
		throw std::runtime_error("Could not set socket options: " + std::string(strerror(errno)));
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	address_length = sizeof(address);
	if (bind(socket_descriptor, (struct sockaddr *)&address, address_length) < 0)
	{
		throw std::runtime_error("Could not bind socket: " + std::string(strerror(errno)));
	}
}

void Server::run()
{
	if (listen(socket_descriptor, 3) < 0)
	{
		throw std::runtime_error("Could not listen on socket: " + std::string(strerror(errno)));
	}

	int epoll_fd = epoll_create(1);
	if (epoll_fd == -1)
	{
		throw std::runtime_error("Could not create epoll instance: " + std::string(strerror(errno)));
	}

	struct epoll_event event = {.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET, .data = {.fd = socket_descriptor}};
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_descriptor, &event) == -1)
	{
		throw std::runtime_error("Could not add socket to epoll instance: " + std::string(strerror(errno)));
	}

	while (true)
	{
		std::cout << "Waiting for events..." << std::endl;
		if (epoll_wait(epoll_fd, &event, 1, -1) < 0)
		{
			std::cout << "Stopping server: Error while waiting for events: " << strerror(errno) << std::endl;
			close(epoll_fd);
			break;
		}

		if (event.data.fd == socket_descriptor)
		{
			int connection_descriptor;
			if ((connection_descriptor = accept(socket_descriptor, (struct sockaddr *)&address, &address_length)) < 0)
				throw std::runtime_error("Could not accept connection: " + std::string(strerror(errno)));
			std::pair<std::map<int, Client>::iterator, bool> result = clients.insert(std::make_pair(connection_descriptor, Client()));
			if (!result.second)
			{
				throw std::runtime_error("Could not insert client into map, file descriptor already exists");
			}
			std::cout << "New connection: " << connection_descriptor << std::endl;
			event = (struct epoll_event){.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET, .data = {.fd = connection_descriptor}};
			if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connection_descriptor, &event) == -1)
				throw std::runtime_error("Could not add connection to epoll instance: " + std::string(strerror(errno)));
		}
		if (event.events & EPOLLIN)
		{
			std::cout << "EPOLLIN on file descriptor " << event.data.fd << std::endl;
			// Read the first 512 bytes and dump them to stdout
			char buffer[512];
			int bytes_read = recv(event.data.fd, buffer, 512, 0);
			if (bytes_read < 0)
				throw std::runtime_error("Could not read from socket: " + std::string(strerror(errno)));
			std::cout << "Read " << bytes_read << " bytes from socket " << event.data.fd << std::endl;
			// Escape the non printable characters and print them to stdout
			for (int i = 0; i < bytes_read; i++)
			{
				if (buffer[i] >= 32 && buffer[i] <= 126)
					std::cout << buffer[i];
				else // print the hex value of the character in the format \x00
					std::cout << "\\x" << std::hex << (int)buffer[i] << std::dec;
			}
			std::cout << std::endl;
		}
		if (event.events & EPOLLOUT)
			std::cout << "EPOLLOUT on file descriptor " << event.data.fd << std::endl;
		if (event.events & EPOLLRDHUP)
		{
			std::cout << "EPOLLRDHUP on file descriptor " << event.data.fd << std::endl;
			// Remove the client from the map
			clients.erase(event.data.fd);
			// Finalize the connection
			if (shutdown(event.data.fd, SHUT_RDWR) < 0)
				throw std::runtime_error("Could not shutdown socket: " + std::string(strerror(errno)));
			close(event.data.fd);
			std::cout << "Closed file descriptor: " << event.data.fd << std::endl;
		}
		if (event.events & EPOLLHUP)
			std::cout << "EPOLLHUP on file descriptor " << event.data.fd << std::endl;
		if (event.events & EPOLLERR)
		{
			std::cout << "EPOLLERR on file descriptor " << event.data.fd << "\tError number: " << errno << std::endl;
			perror("Error: ");
		}
	}

	std::cout << "Shutting down server..." << std::endl;
	std::cout << "Closing " << clients.size() << " connections..." << std::endl;
	for (std::map<int, Client>::iterator client = clients.begin(); client != clients.end(); client++)
	{
		std::cout << "Closing connection: " << client->first << std::endl;
		if (shutdown(client->first, SHUT_RDWR) < 0)
			throw std::runtime_error("Could not shutdown socket: " + std::string(strerror(errno)));
		close(client->first);
	}

	// Shutdown the socket
	std::cout << "Closing socket..." << std::endl;
	if (shutdown(socket_descriptor, SHUT_RDWR) < 0)
		throw std::runtime_error("Could not shutdown socket: " + std::string(strerror(errno)));
	close(socket_descriptor);
}