#include "user.hpp"
#include <string>
#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <sys/epoll.h>
#include <iostream>

User::User(int user_fd, int main_epoll_fd) : fd(user_fd), epoll_fd(main_epoll_fd)
{
	struct epoll_event event = {.events = EPOLLIN | EPOLLOUT, .data = {.fd = user_fd}};

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1)
		throw std::runtime_error("Could not add user socket to epoll: " + std::string(strerror(errno)));
}

ssize_t User::receive_data()
{
	// read the data
	std::string data(MAX_MESSAGE_SIZE, 0);
	ssize_t bytes_read;
	if ((bytes_read = recv(fd, &data[0], data.size(), MSG_DONTWAIT)) == -1)
		throw std::runtime_error("Could not read from user socket: " + std::string(strerror(errno)));
	else if (bytes_read == 0)
		return 0;
	else
		data.resize(bytes_read);

	incoming_buffer += data;
	size_t eol_pos;

	while ((eol_pos = incoming_buffer.find(EOL)) != std::string::npos)
	{
		// extract the message
		std::string message = incoming_buffer.substr(0, eol_pos);

		// truncate the message if it is too long
		if (message.size() > MAX_MESSAGE_SIZE - 2)
			message.resize(MAX_MESSAGE_SIZE - 2);

		// push the message to the queue
		if (eol_pos != 0)
			incoming_messages.push(message);
		incoming_buffer = incoming_buffer.substr(eol_pos + 2);

		std::cout << "Incoming message from fd#" << fd << ": <<<" << message << ">>>" << std::endl;
	}

	// if the buffer is full, the message is too long
	if (incoming_buffer.size() >= MAX_MESSAGE_SIZE)
	{
		incoming_buffer.clear();
		throw std::runtime_error("Message too long");
	}

	return bytes_read;
}

ssize_t User::send_data()
{
	// if there is no data to send, remove EPOLLOUT from events
	if (outgoing_buffer.empty())
	{
		struct epoll_event event = {.events = EPOLLIN, .data = {.fd = fd}};
		if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event) == -1)
			throw std::runtime_error("Could not modify user socket events: " + std::string(strerror(errno)));
		return -1;
	}

	// try to send the data
	ssize_t bytes_sent;
	if ((bytes_sent = send(fd, outgoing_buffer.c_str(), outgoing_buffer.size(), MSG_DONTWAIT)) == -1)
		throw std::runtime_error("Could not write to user socket: " + std::string(strerror(errno)));
	else
		outgoing_buffer.erase(0, bytes_sent);

	return bytes_sent;
}

void User::queue_message(const std::string &message)
{
	std::cout << "Outgoing message for fd#" << fd << ": <<<" << message << ">>>" << std::endl;
	outgoing_buffer += message + EOL;

	// add EPOLLOUT to events
	struct epoll_event current_event = {.events = EPOLLIN | EPOLLOUT, .data = {.fd = fd}};
	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, current_event.data.fd, &current_event) == -1)
		throw std::runtime_error("Could not modify user socket events: " + std::string(strerror(errno)));
}
