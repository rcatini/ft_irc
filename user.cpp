#include <stdexcept>
#include <string>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include "ircserv.hpp"
#include "user.hpp"

// Create user object from socket file descriptor
User::User(int socket_fd) : fd(socket_fd)
{
    std::cerr << "New user connected on socket " << fd << std::endl;
    put_message("Welcome to the IRC server!");
}

// Copy user object (cannibalizing other user)
User::User(const User &other) : fd(other.fd), incoming_buffer(other.incoming_buffer), outgoing_buffer(other.outgoing_buffer)
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

    std::cerr << "User on socket " << fd << " disconnected" << std::endl;
}

// Check if user has outgoing messages
bool User::has_outgoing_messages()
{
    return outgoing_buffer.find(LINE_DELIM) != std::string::npos;
}

// Extract message from incoming buffer
std::string User::get_message()
{
    // Find end of line
    std::size_t line_end = incoming_buffer.find(LINE_DELIM);

    // If no end of line, the buffer does not contain a complete message
    if (line_end == std::string::npos)
        throw std::logic_error("No complete message in buffer");

    // If line is empty, read next message
    if (line_end == 0)
    {
        incoming_buffer.erase(0, strlen(LINE_DELIM));
        return get_message();
    }

    // Extract message from buffer
    std::string message = incoming_buffer.substr(0, line_end);
    incoming_buffer.erase(0, line_end + strlen(LINE_DELIM));

    // Check message length
    if (message.length() > MAX_MSG_LEN - strlen(LINE_DELIM))
        throw std::length_error("Message too long");

    std::cerr << "Message extracted from incoming buffer (client #" << fd << "): " << message << std::endl;

    return message;
}

// Check if user has outgoing messages
bool User::has_incoming_messages()
{

    // Find end of line
    std::size_t line_end = incoming_buffer.find(LINE_DELIM);

    // If no end of line, the buffer does not contain a complete message
    if (line_end == std::string::npos)
        return false;

    // If line is empty, look for next message
    if (line_end == 0)
    {
        incoming_buffer.erase(0, strlen(LINE_DELIM));
        return has_incoming_messages();
    }

    // If line is not empty, there is a complete message in the buffer
    return true;
}

// Append message to outgoing buffer
bool User::put_message(std::string message)
{
    if (message.empty())
        return false;

    // Check message length
    if (message.length() > MAX_MSG_LEN - strlen(LINE_DELIM))
        throw std::length_error("Message too long");

    // Add message to outgoing buffer (with line delimiter)
    outgoing_buffer.append(message + LINE_DELIM);

    std::cerr << "Message added to outgoing buffer (client #" << fd << "): " << message << std::endl;

    return true;
}

// Consumes data from socket and appends it to incoming buffer
ssize_t User::read()
{
    // Read data from socket
    char buf[MAX_MSG_LEN];
    ssize_t bytes_read;
    while ((bytes_read = recv(fd, buf, MAX_MSG_LEN, MSG_DONTWAIT)) > 0)
        incoming_buffer.append(buf, (unsigned long)bytes_read);

    // Return 0 if operation would block (try again later)
    if (bytes_read < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return 0;
        throw std::runtime_error("Error reading message" + std::string(strerror(errno)));
    }

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