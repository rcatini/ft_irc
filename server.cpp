#include "server.hpp"
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <csignal>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <map>
#include <vector>

Server::Server(unsigned short p, const std::string &pass, volatile sig_atomic_t &sigstatus)
	: port(p), password(pass), signal(sigstatus)
{
	if ((this->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		throw std::runtime_error("Could not create socket: " + std::string(strerror(errno)));

	this->address = (struct sockaddr_in){AF_INET, htons(port), {INADDR_ANY}, {}};

	if (bind(this->fd, (struct sockaddr *)&this->address, sizeof(this->address)) == -1)
		throw std::runtime_error("Could not bind socket: " + std::string(strerror(errno)));

	if (listen(this->fd, CONNECTION_QUEUE_SIZE) == -1)
		throw std::runtime_error("Could not listen on socket: " + std::string(strerror(errno)));
}

Server::~Server()
{
	if (close(this->fd) == -1)
		throw std::runtime_error("Could not close server socket: " + std::string(strerror(errno)));
	for (std::map<int, User>::iterator it = users.begin(); it != users.end(); ++it)
		if (close(it->first) == -1)
			throw std::runtime_error("Could not close user socket: " + std::string(strerror(errno)));
}

void Server::run()
{
	int epoll_fd;

	if ((epoll_fd = epoll_create(1)) == -1)
		throw std::runtime_error("Could not create epoll instance: " + std::string(strerror(errno)));

	struct epoll_event server_event = {.events = EPOLLIN, .data = {.fd = this->fd}};
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, this->fd, &server_event) == -1)
		throw std::runtime_error("Could not add server socket to epoll: " + std::string(strerror(errno)));

	struct epoll_event event;

	while (!signal)
	{
		int event_count = epoll_wait(epoll_fd, &event, 1, -1);
		if (event_count <= 0)
			continue;
		else if (event.data.fd == this->fd)
		{
			int user_fd = accept_connection();
			struct epoll_event user_event = {.events = EPOLLIN, .data = {.fd = user_fd}};
			if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, user_fd, &user_event) == -1)
				throw std::runtime_error("Could not add user socket to epoll: " + std::string(strerror(errno)));
		}
		else if (event.events & EPOLLIN)
		{
			char buffer[MAX_MESSAGE_SIZE];
			ssize_t bytes_read;
			if ((bytes_read = recv(event.data.fd, buffer, sizeof(buffer), MSG_DONTWAIT)) == -1)
				throw std::runtime_error("Could not read from user socket: " + std::string(strerror(errno)));
			else if (bytes_read == 0)
			{
				if (close(event.data.fd) == -1)
					throw std::runtime_error("Could not close user socket: " + std::string(strerror(errno)));
				users.erase(event.data.fd);
			}
			else
			{
				std::string data(buffer, bytes_read);
				users[event.data.fd].receive_data(data);
			}
		}
		else
			throw std::runtime_error("Unexpected event on user socket");
	}
	if (close(epoll_fd) == -1)
		throw std::runtime_error("Could not close epoll instance: " + std::string(strerror(errno)));
}

int Server::accept_connection()
{
	struct sockaddr_in accept_address;
	int accept_fd;
	socklen_t address_size = sizeof(accept_address);

	if ((accept_fd = accept(this->fd, (struct sockaddr *)&accept_address, &address_size)) == -1)
		throw std::runtime_error("Could not accept connection: " + std::string(strerror(errno)));
	users[accept_fd] = User();
	return accept_fd;
}
