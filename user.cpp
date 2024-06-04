#include "user.hpp"
#include <string>
#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <sys/epoll.h>
#include <iostream>

User::User(struct epoll_event initial_event, int main_epoll_fd) : current_event(initial_event), epoll_fd(main_epoll_fd)
{
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, current_event.data.fd, &current_event) == -1)
		throw std::runtime_error("Could not add user socket to epoll: " + std::string(strerror(errno)));
}

ssize_t User::receive_data()
{
	std::string data(MAX_MESSAGE_SIZE, 0);
	ssize_t bytes_read;
	if ((bytes_read = recv(current_event.data.fd, &data[0], data.size(), MSG_DONTWAIT)) == -1)
		throw std::runtime_error("Could not read from user socket: " + std::string(strerror(errno)));
	else if (bytes_read == 0)
		return 0;
	else
		data.resize(bytes_read);

	incoming_buffer += data;
	size_t eol_pos;

	while ((eol_pos = incoming_buffer.find(EOL)) != std::string::npos)
	{
		if (eol_pos != 0)
			incoming_messages.push(incoming_buffer.substr(0, eol_pos));
		incoming_buffer = incoming_buffer.substr(eol_pos + 2);
		std::cout << "Incoming message from fd#" << current_event.data.fd << ": <<<" << incoming_messages.back() << ">>>" << std::endl;
	}
	return bytes_read;
}

ssize_t User::send_data()
{
	if (outgoing_buffer.empty())
	{
		// remove EPOLLOUT from events
		current_event.events &= ~EPOLLOUT;
		if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, current_event.data.fd, &current_event) == -1)
			throw std::runtime_error("Could not modify user socket events: " + std::string(strerror(errno)));
		return -1;
	}
	ssize_t bytes_sent;
	if ((bytes_sent = send(current_event.data.fd, outgoing_buffer.c_str(), outgoing_buffer.size(), MSG_DONTWAIT)) == -1)
		throw std::runtime_error("Could not write to user socket: " + std::string(strerror(errno)));
	else
		outgoing_buffer.erase(0, bytes_sent);
	return bytes_sent;
}

void User::queue_message(const std::string &message)
{
	std::cout << "Outgoing message for fd#" << current_event.data.fd << ": <<<" << message << ">>>" << std::endl;
	outgoing_buffer += message + EOL;
	current_event.events |= EPOLLOUT;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, current_event.data.fd, &current_event) == -1)
		throw std::runtime_error("Could not modify user socket events: " + std::string(strerror(errno)));
}