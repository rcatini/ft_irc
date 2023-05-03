#include <stdexcept>
#include <string>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include "ircserv.hpp"
#include "user.hpp"

// Create user object from socket file descriptor
User::User(int socket_fd) : fd(socket_fd), registered(false)
{
}

// Copy user object (cannibalizing other user)
User::User(const User &other) : fd(other.fd), registered(other.registered), incoming_buffer(other.incoming_buffer), outgoing_buffer(other.outgoing_buffer), incoming_messages(other.incoming_messages)
{
    // Cannibalizes other user (steals file descriptor)
    int &other_fd = const_cast<int &>(const_cast<User &>(other).fd);
    other_fd = -1;
}

// Destroy user object closing network connection
User::~User()
{
    // If socket is invalid, do nothing
    if (fd < 0)
        return;

    // Close connection
    if (shutdown(fd, SHUT_RDWR) < 0)
        std::cerr << "could not shutdown user socket: " << std::string(strerror(errno)) << std::endl;

    // Close file descriptor
    if (close(fd) < 0)
        std::cerr << "could not close user socket: " << std::string(strerror(errno)) << std::endl;
}

// Check if user has outgoing messages
bool User::has_outgoing_messages()
{
    return outgoing_buffer.length() > 0;
}

// Extract message from incoming buffer
std::string User::get_message()
{
    if (incoming_messages.empty())
        return "";

    std::string message = incoming_messages.front();
    incoming_messages.pop_front();
    return message;
}

// Append message to outgoing buffer
bool User::put_message(std::string message)
{
    if (message.empty())
        return false;

    // Check message length
    if (message.length() > MAX_MSG_LEN - strlen(LINE_DELIM))
        throw std::length_error("Message too long");

    outgoing_buffer.append(message + LINE_DELIM);

    return true;
}

// Parse incoming buffer and put messages in incoming_messages
void split_buffer(std::string &buffer, std::list<std::string> &messages)
{
    size_t line_delim_pos;

    // While there are line delimiters in the incoming buffer
    while ((line_delim_pos = buffer.find(LINE_DELIM)) != std::string::npos)
    {
        // If line delimiter is at the beginning of the string, ignore it
        if (line_delim_pos == 0)
        {
            buffer.erase(0, strlen(LINE_DELIM));
            continue;
        }

        // Extract message from incoming buffer
        std::string message = buffer.substr(0, line_delim_pos);
        buffer.erase(0, line_delim_pos + strlen(LINE_DELIM));
        messages.push_back(message);
    }

    // Check if the remaining string is too long
    if (buffer.length() > MAX_MSG_LEN)
        throw std::length_error("Message too long");
}

// Consumes data from socket and appends it to incoming buffer
ssize_t User::read()
{
    char buf[MAX_MSG_LEN];
    ssize_t bytes_read;

    // Read data from socket
    while ((bytes_read = recv(fd, buf, MAX_MSG_LEN, MSG_DONTWAIT)) > 0)
    {
        // Append data to incoming buffer
        incoming_buffer.append(buf, (unsigned long)bytes_read);

        // If the string has grown over the maximum message length, parse early
        if (incoming_buffer.length() > MAX_MSG_LEN)
            split_buffer(incoming_buffer, incoming_messages);
    }

    // Return 0 if operation would block (try again later)
    if (bytes_read < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            bytes_read = 0;
        else
            throw std::runtime_error("Error reading message" + std::string(strerror(errno)));
    }

    // Parse the incoming data
    split_buffer(incoming_buffer, incoming_messages);

    return bytes_read;
}

// Writes messages from outgoing buffer to socket, returns written bytes
ssize_t User::write()
{
    ssize_t bytes_sent = 0;

    // Send data from outgoing buffer
    while (bytes_sent >= 0 && outgoing_buffer.length() > 0)
    {
        // Send all data from outgoing buffer
        bytes_sent = send(fd, outgoing_buffer.c_str(), outgoing_buffer.length(), MSG_DONTWAIT);

        // Delete written data from outgoing buffer
        if (bytes_sent > 0)
            outgoing_buffer.erase(0, (unsigned long)bytes_sent);

        // Return 0 if operation would block (try again later)
        else if (errno == EAGAIN || errno == EWOULDBLOCK)
            return 0;
    }

    return bytes_sent;
}