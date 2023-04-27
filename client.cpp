#include <stdexcept>
#include <string>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include "ircserv.hpp"
#include "client.hpp"

std::vector<std::string> Client::parse_buffer()
{
	std::vector<std::string> messages;
	while (buffer.length() > 0)
	{
		std::size_t line_end = buffer.find(LINE_DELIM);
		if (line_end == std::string::npos)
			break;
		if (line_end == 0)
		{
			buffer.erase(0, strlen(LINE_DELIM));
			continue;
		}
		std::string message = buffer.substr(0, line_end);
		buffer.erase(0, line_end + strlen(LINE_DELIM));
		if (message.length() > MAX_MSG_LEN - strlen(LINE_DELIM))
			throw std::runtime_error("Message too long");
		messages.push_back(message);
	}
	return messages;
}

int Client::read(int fd)
{
	char buf[MAX_MSG_LEN];
	int bytes_read;
	while ((bytes_read = recv(fd, buf, MAX_MSG_LEN, MSG_DONTWAIT)) > 0)
		buffer.append(buf, bytes_read);
	if (bytes_read < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
		return 0;
	return bytes_read;
}

int Client::write(int fd)
{
	int messages_sent = 0;
	while (outgoing_messages.size() > 0)
	{
		std::string message = outgoing_messages.front();
		message.append(LINE_DELIM);
		int bytes_sent = send(fd, message.c_str(), message.length(), MSG_DONTWAIT);
		if (bytes_sent < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			else
				throw std::runtime_error("Error sending message");
		}
		outgoing_messages.erase(outgoing_messages.begin());
		messages_sent++;
	}
	return messages_sent;
}