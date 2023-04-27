#include "server.hpp"
#include <string>
#include <iostream>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>

Server::Server(bool &teardown_ref, int server_port, std::string server_password) : port(server_port), password(server_password), fd(-1), teardown(teardown_ref)
{
	if ((this->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw std::runtime_error("Could not create socket: " + std::string(strerror(errno)));
	struct sockaddr_in address_ipv4 = (struct sockaddr_in){AF_INET, htons(port), {INADDR_ANY}, {}};
	this->address = *reinterpret_cast<struct sockaddr *>(&address_ipv4);
	this->address_length = sizeof(address_ipv4);
	if (bind(this->fd, &this->address, sizeof(address_ipv4)) < 0)
		throw std::runtime_error("Could not bind socket: " + std::string(strerror(errno)));
	if (listen(this->fd, 3) < 0)
		throw std::runtime_error("Could not listen on socket: " + std::string(strerror(errno)));
}

int Server::handle_server_event(struct epoll_event event)
{
	if (event.events & EPOLLERR)
		throw std::runtime_error("error on server socket");
	else if (event.events & EPOLLHUP)
		throw std::runtime_error("server socket hung up");
	else if (event.events & EPOLLRDHUP)
		throw std::runtime_error("socket closed");
	else if (event.events & ~EPOLLIN)
		throw std::runtime_error("unknown event on server socket");
	int connection_descriptor;
	if ((connection_descriptor = accept(this->fd, &this->address, &this->address_length)) < 0)
		throw std::runtime_error("could not accept connection: " + std::string(strerror(errno)));
	std::pair<std::map<int, Client>::iterator, bool> result = clients.insert(std::make_pair(connection_descriptor, Client()));
	if (!result.second)
		throw std::runtime_error("could not insert client into map, file descriptor already exists");
	return connection_descriptor;
}

void Server::handle_client_event(struct epoll_event event)
{
	Client &client = this->clients.at(event.data.fd);
	if (event.events & ~(EPOLLERR | EPOLLHUP | EPOLLRDHUP | EPOLLIN | EPOLLOUT))
		throw std::runtime_error("unknown event on client socket");
	if (event.events & EPOLLIN)
	{
		if (client.read(event.data.fd) < 0)
			throw std::runtime_error("error in reading client socket");
		std::vector<std::string> messages = client.parse_buffer();
		for (std::vector<std::string>::iterator it = messages.begin(); it != messages.end(); ++it)
			std::cout << *it << std::endl;
	}
	if (event.events & EPOLLOUT && client.write(event.data.fd) < 0)
		throw std::runtime_error("error in writing client socket");
	if (event.events & EPOLLOUT)
		client.write(event.data.fd);
	if (event.events & EPOLLERR)
		throw std::runtime_error("error on client socket");
	if (event.events & EPOLLHUP || event.events & EPOLLRDHUP)
	{
		if (!this->clients.erase(event.data.fd))
			throw std::runtime_error("could not erase client from map, file descriptor does not exist");
		std::cerr << "client disconnected" << std::endl;
		disconnect_client(event.data.fd);
	}
}

void Server::run()
{
	int epoll_fd;
	if ((epoll_fd = epoll_create(1)) < 0)
		throw std::runtime_error("could not create epoll instance: " + std::string(strerror(errno)));
	struct epoll_event event = {.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET, .data = {.fd = this->fd}};
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, this->fd, &event) < 0)
		throw std::runtime_error("could not add network socket to epoll instance: " + std::string(strerror(errno)));
	while (!this->teardown)
	{
		if (epoll_wait(epoll_fd, &event, 1, -1) < 0)
			std::cerr << "could not wait for epoll event: " << strerror(errno) << std::endl;
		else if (event.data.fd == this->fd)
		{
			int connection_descriptor = handle_server_event(event);
			event = (struct epoll_event){.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET, .data = {.fd = connection_descriptor}};
			if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connection_descriptor, &event) < 0)
				throw std::runtime_error("could not add connection to epoll instance: " + std::string(strerror(errno)));
		}
		else
			handle_client_event(event);
	}
	if (close(epoll_fd) < 0)
		throw std::runtime_error("could not close epoll instance: " + std::string(strerror(errno)));
}

Server::~Server()
{
	for (std::map<int, Client>::iterator client = this->clients.begin(); client != this->clients.end(); client++)
		disconnect_client(client->first);
	if (this->fd < 0 || shutdown(this->fd, SHUT_RDWR) < 0 || close(this->fd) < 0)
		std::cerr << "could not cleanly destroy server: " << strerror(errno) << std::endl;
	this->fd = -1;
}

void Server::disconnect_client(int client_fd)
{
	std::cout << "Disconnecting client " << client_fd << std::endl;
	if (shutdown(client_fd, SHUT_RDWR) < 0)
		throw std::runtime_error("could not shutdown client socket: " + std::string(strerror(errno)));
	if (close(client_fd) < 0)
		throw std::runtime_error("could not close client socket: " + std::string(strerror(errno)));
}