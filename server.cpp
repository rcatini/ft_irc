#include "server.hpp"
#include <string>
#include <iostream>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <vector>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>

// Create server object, starting listening on port for TCP connections
Server::Server(bool &teardown_ref, int server_port, std::string server_password) : port(server_port), password(server_password), fd(-1), teardown(teardown_ref)
{
	// Create socket
	if ((this->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw std::runtime_error("Could not create socket: " + std::string(strerror(errno)));

	// Construct address structure using IPv4 and the specified port
	struct sockaddr_in address_ipv4 = (struct sockaddr_in){AF_INET, htons(port), {INADDR_ANY}, {}};
	this->address = *reinterpret_cast<struct sockaddr *>(&address_ipv4);
	this->address_length = sizeof(address_ipv4);

	// Bind socket to port
	if (bind(this->fd, &this->address, sizeof(address_ipv4)) < 0)
		throw std::runtime_error("Could not bind socket: " + std::string(strerror(errno)));

	// Start listening on socket
	if (listen(this->fd, 3) < 0)
		throw std::runtime_error("Could not listen on socket: " + std::string(strerror(errno)));
}

// Detect an incoming connection, and handle it (throwing an exception if it fails)
int Server::handle_server_event(struct epoll_event event)
{
	// Check for errors
	if (event.events & EPOLLERR)
		throw std::runtime_error("error on server socket");
	else if (event.events & EPOLLHUP)
		throw std::runtime_error("server socket hung up");
	else if (event.events & EPOLLRDHUP)
		throw std::runtime_error("socket closed");
	else if (event.events & ~EPOLLIN)
		throw std::runtime_error("unknown event on server socket");

	// Accept connection from new user
	int connection_descriptor;
	if ((connection_descriptor = accept(this->fd, &this->address, &this->address_length)) < 0)
		throw std::runtime_error("could not accept connection: " + std::string(strerror(errno)));

	// Add user to map
	std::pair<std::map<int, User>::iterator, bool> result = users.insert(std::make_pair(connection_descriptor, User(connection_descriptor)));
	if (!result.second)
		throw std::runtime_error("could not insert user into map, file descriptor already exists");

	return connection_descriptor;
}

// Broadcast a message to all users, except the sender
void Server::broadcast(std::string message, int sender_descriptor)
{
	// Iterate over all users
	for (std::map<int, User>::iterator it = this->users.begin(); it != this->users.end(); ++it)
	{
		// Skip sender
		if (it->first == sender_descriptor)
			continue;

		// Add message to outgoing queue
		it->second.put_message(message);
	}
}

// Handle user event, returning false if user has disconnected
bool Server::handle_user_event(struct epoll_event event)
{
	// Get user object
	User &user = this->users.at(event.data.fd);

	// Check for unknown events
	if (event.events & ~(EPOLLERR | EPOLLHUP | EPOLLRDHUP | EPOLLIN | EPOLLOUT))
		throw std::runtime_error("unknown event on user socket");

	// Get user input
	if (event.events & EPOLLIN)
	{
		if (user.read() < 0)
			throw std::runtime_error("error in reading user socket");
		std::list<std::string> messages = user.get_messages();
		for (std::list<std::string>::iterator it = messages.begin(); it != messages.end(); ++it)
			broadcast(*it, event.data.fd);
	}

	// Send user output
	if (event.events & EPOLLOUT && user.write() < 0)
		throw std::runtime_error("error in writing user socket");

	// Handle unknown errors
	if (event.events & EPOLLERR)
		throw std::runtime_error("error on user socket");

	// Handle user disconnect
	if (event.events & EPOLLHUP || event.events & EPOLLRDHUP)
	{
		if (!this->users.erase(event.data.fd))
			throw std::runtime_error("could not erase user from map, file descriptor does not exist");
		std::cerr << "user disconnected" << std::endl;
		return false;
	}

	return true;
}

void Server::run()
{
	// Set up epoll
	int epoll_fd;
	if ((epoll_fd = epoll_create(1)) < 0)
		throw std::runtime_error("could not create epoll instance: " + std::string(strerror(errno)));

	// Add server socket to epoll (wait for incoming connections)
	struct epoll_event event = {.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET, .data = {.fd = this->fd}};
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, this->fd, &event) < 0)
		throw std::runtime_error("could not add network socket to epoll instance: " + std::string(strerror(errno)));

	// Prepare return vector for epoll_wait. For now, only one event is expected (server socket)
	int expected_events = 1;
	std::vector<struct epoll_event> events(expected_events);

	// Loop until teardown is requested from signal handler
	while (!this->teardown)
	{
		// Wait for epoll event (can be interrupted by incoming signal)
		int event_count;
		events.resize(expected_events);
		if ((event_count = epoll_wait(epoll_fd, events.data(), events.size(), -1)) < 0)
		{
			// If interrupted by SIGINT, shut down server
			if (errno == EINTR && this->teardown)
			{
				std::cerr << "epoll_wait interrupted by SIGINT, shutting down server" << std::endl;
				break;
			}

			// If interrupted by another signal, continue (ignore the signal)
			else if (errno == EINTR)
				continue;

			// Otherwise, throw an exception
			else
				throw std::runtime_error("could not wait for epoll event: " + std::string(strerror(errno)));
		}

		// Handle all events
		for (int i = 0; i < event_count; ++i)
		{
			// Handle server event (new connection)
			if (events[i].data.fd == this->fd)
			{
				// Try to accept connection
				int connection_descriptor = handle_server_event(events[i]);
				// Add connection to epoll instance
				event = (struct epoll_event){.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET, .data = {.fd = connection_descriptor}};
				if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connection_descriptor, &event) < 0)
					throw std::runtime_error("could not add connection to epoll instance: " + std::string(strerror(errno)));
				// Increase capacity of return vector for epoll_wait
				expected_events++;
			}
			// Handle user events (if user has disconnected, decrease capacity of return vector for epoll_wait)
			else if (!handle_user_event(events[i]))
				expected_events--;
		}

		// Rearm all triggers for users with outgoing messages
		for (std::map<int, User>::iterator it = this->users.begin(); it != this->users.end(); ++it)
		{
			if (it->second.has_messages())
			{
				event = (struct epoll_event){.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET, .data = {.fd = it->first}};
				if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, it->first, &event) < 0)
					throw std::runtime_error("could not rearm user with outgoing messages: " + std::string(strerror(errno)));
			}
		}
	}
	// End of loop, close epoll instance
	if (close(epoll_fd) < 0)
		throw std::runtime_error("could not close epoll instance: " + std::string(strerror(errno)));
}

// Stop the server cleanly closing all connections
Server::~Server()
{
	// Destroy all users, bringing down all client connections
	users.clear();

	// Close server socket
	if (this->fd < 0 || shutdown(this->fd, SHUT_RDWR) < 0 || close(this->fd) < 0)
		std::cerr << "could not cleanly destroy server: " << strerror(errno) << std::endl;
}