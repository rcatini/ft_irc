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
#include <iostream>

Server::Server(unsigned short p, const std::string &pass, volatile sig_atomic_t &sigstatus)
	: port(p), password(pass), signal(sigstatus)
{
	// socket creation (normally fd==3)
	if ((this->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		throw std::runtime_error("Could not create socket: " + std::string(strerror(errno)));

	// enable port reuse
	int enable = 1;
	if (setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1)
		throw std::runtime_error("Could not set socket options: " + std::string(strerror(errno)));

	// bind socket to port
	this->address = (struct sockaddr_in){AF_INET, htons(port), {INADDR_ANY}, {}};
	if (bind(this->fd, (struct sockaddr *)&this->address, sizeof(this->address)) == -1)
		throw std::runtime_error("Could not bind socket: " + std::string(strerror(errno)));

	// listen on socket
	if (listen(this->fd, CONNECTION_QUEUE_SIZE) == -1)
		throw std::runtime_error("Could not listen on socket: " + std::string(strerror(errno)));
}

Server::~Server()
{
	// close server socket
	if (close(this->fd) == -1)
		throw std::runtime_error("Could not close server socket: " + std::string(strerror(errno)));
}

void Server::run()
{
	// epoll instance creation (normally epoll_fd==4)
	int epoll_fd;
	if ((epoll_fd = epoll_create(1)) == -1)
		throw std::runtime_error("Could not create epoll instance: " + std::string(strerror(errno)));

	// add server socket to epoll (wait for incoming connections)
	struct epoll_event server_event = {.events = EPOLLIN, .data = {.fd = this->fd}};
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, this->fd, &server_event) == -1)
		throw std::runtime_error("Could not add server socket to epoll: " + std::string(strerror(errno)));

	// add server stdin to epoll (wait for user input)
	struct epoll_event stdin_event = {.events = EPOLLIN, .data = {.fd = STDIN_FILENO}};
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &stdin_event) == -1)
		throw std::runtime_error("Could not add stdin to epoll: " + std::string(strerror(errno)));

	// event structure for the epoll results (space for 2 events: server socket and stdin)
	std::vector<struct epoll_event> events(2);

	// wait for incoming connections and user data
	while (!signal)
	{
		int event_count = epoll_wait(epoll_fd, &events.front(), (int)events.size(), -1);

		// if wait was interrupted (e.g. by a signal), retry
		if (event_count <= 0)
			continue;

		// check all events
		for (int i = 0; i < event_count && !signal; ++i)
		{
			struct epoll_event event = events[(unsigned long)i];

			// if server socket has a read event, accept the connection
			if (event.data.fd == this->fd)
			{
				struct sockaddr_in user_address;
				socklen_t user_address_len = sizeof(user_address);
				int user_fd;
				if ((user_fd = accept(this->fd, (struct sockaddr *)&user_address, &user_address_len)) == -1)
					throw std::runtime_error("Could not accept connection: " + std::string(strerror(errno)));
				fd_user.insert(std::make_pair(user_fd, User(*this, user_fd, user_address, epoll_fd)));
				events.resize(events.size() + 1);
			}

			// if stdin has a read event, read the input (server-side command)
			else if (event.data.fd == STDIN_FILENO)
			{
				std::string input;
				std::getline(std::cin, input);
				if (input == "exit" || input == "quit")
					signal = SIGINT;
				else if (input == "list")
					list_fd_user();
				else
					broadcast(input);

				// remove stdin from epoll events
				if (std::cin.eof() && epoll_ctl(epoll_fd, EPOLL_CTL_DEL, STDIN_FILENO, NULL) == -1)
					throw std::runtime_error("Could not remove stdin from epoll: " + std::string(strerror(errno)));
			}

			// find the epoll fd in the users map
			if (fd_user.find(event.data.fd) != fd_user.end())
			{
				// if user socket has a read event, receive data from it
				if (event.events & EPOLLIN)
				{
					if (fd_user.at(event.data.fd).receive_data() == 0)
					{
						if (close(event.data.fd) == -1)
							throw std::runtime_error("Could not close user socket: " + std::string(strerror(errno)));
						fd_user.erase(event.data.fd);
						events.resize(events.size() - 1);
					}
				}

				// if user socket has a write event, send data to it
				if (event.events & EPOLLOUT)
				{
					if (fd_user.at(event.data.fd).send_data() == 0)
					{
						if (close(event.data.fd) == -1)
							throw std::runtime_error("Could not close user socket: " + std::string(strerror(errno)));
						fd_user.erase(event.data.fd);
						events.resize(events.size() - 1);
					}
				}
			}
		}
	}

	// close all user sockets
	for (std::map<int, User>::iterator it = fd_user.begin(); it != fd_user.end(); ++it)
		if (close(it->first) == -1)
			throw std::runtime_error("Could not close user socket: " + std::string(strerror(errno)));

	// exit from the loop, close epoll instance
	if (close(epoll_fd) == -1)
		throw std::runtime_error("Could not close epoll instance: " + std::string(strerror(errno)));
}

void Server::broadcast(const std::string &message)
{
	for (std::map<int, User>::iterator it = fd_user.begin(); it != fd_user.end(); ++it)
		it->second.queue_message(message);
}

void Server::list_fd_user()
{
	for (std::map<int, User>::iterator it = fd_user.begin(); it != fd_user.end(); ++it)
		std::cout << it->first << "\t" << it->second.get_address() << std::endl;
}

bool Server::verify_password(const std::string &pass) const
{
	return pass == password;
}

bool Server::nick_exists(const std::string &nick)
{
	for (std::map<int, User>::const_iterator it = fd_user.begin(); it != fd_user.end(); ++it)
		if (it->second.get_nickname() == nick)
			return true;
	return false;
}
