#include "server.hpp"
#include <string>
#include <iostream>
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

	struct epoll_event event;
	event.data.fd = socket_descriptor;
	event.events = EPOLLIN;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_descriptor, &event) == -1)
	{
		throw std::runtime_error("Could not add socket to epoll instance: " + std::string(strerror(errno)));
	}

	while (true)
	{
		if (epoll_wait(epoll_fd, &event, 1, -1) < 0)
		{
			throw std::runtime_error("Could not wait for epoll events: " + std::string(strerror(errno)));
		}

		if (event.data.fd == socket_descriptor)
		{
			int connection_descriptor;
			if ((connection_descriptor = accept(socket_descriptor, (struct sockaddr *)&address, &address_length)) < 0)
				throw std::runtime_error("Could not accept connection: " + std::string(strerror(errno)));
			std::pair<int, Client> element(connection_descriptor, Client(connection_descriptor));
			std::pair<std::map<int, Client>::iterator, bool>  result = clients.insert(element);
			if (!result.second)
			{
				std::cout << "Connection already exists: " << connection_descriptor << std::endl;
				result.first->second = element.second;
			}
			std::cout << "New connection: " << connection_descriptor << std::endl;
		}
	}
}