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
    if (message.length() > MAX_MSG_LEN - 2)
        throw std::length_error("Message too long");

    outgoing_buffer.append(message + "\r\n");

    return true;
}

// Gets the first non-empty line from the buffer
std::string getline(std::string &buffer)
{
    size_t delim_len = 0;

    // Find first line delimiter
    size_t pos = buffer.find("\r\n");
    if (pos != std::string::npos)
    {
        delim_len++;
        if (buffer.size() > pos + 1 && buffer[pos + 1] == '\n')
            delim_len++;
    }
    else
    {
        pos = buffer.find("\n");
        if (pos != std::string::npos)
            delim_len++;
    }

    // If no line delimiter was found, return empty string
    if (pos == std::string::npos)
        return "";

    // Extract line from buffer
    std::string line = buffer.substr(0, pos);

    // Remove line from buffer
    buffer.erase(0, pos + delim_len);

    // If the line is empty, return the next one
    if (line.empty())
        return getline(buffer);

    return line;
}

// Parse incoming buffer and put messages in incoming_messages
void split_buffer(std::string &buffer, std::list<std::string> &messages)
{
    // While there is a line in the buffer
    std::string line;
    while ((line = getline(buffer)) != "")
    {
        // If line is too long, throw exception
        if (line.length() > MAX_MSG_LEN - 2)
            throw std::length_error("Message too long");

        // Append line to incoming messages
        messages.push_back(line);
    }

    // Check if the remaining string is too long
    if (buffer.length() > MAX_MSG_LEN - 2)
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