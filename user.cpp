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
}

// Check if user has messages to send
bool User::has_messages()
{
    return outgoing_buffer.length() > 0;
}

// Get messages from the buffer, and returns them as a list of strings
std::list<std::string> User::get_messages()
{
    // Return empty list if there are no messages
    std::list<std::string> messages;

    // Split incoming buffer into messages
    while (incoming_buffer.length() > 0)
    {
        // Find end of line
        std::size_t line_end = incoming_buffer.find(LINE_DELIM);

        // If no end of line, the buffer does not contain a complete message
        if (line_end == std::string::npos)
            break;

        // If line is empty, ignore it
        if (line_end == 0)
        {
            incoming_buffer.erase(0, strlen(LINE_DELIM));
            continue;
        }

        // Extract message from buffer
        std::string message = incoming_buffer.substr(0, line_end);
        incoming_buffer.erase(0, line_end + strlen(LINE_DELIM));

        // Check message length
        if (message.length() > MAX_MSG_LEN - strlen(LINE_DELIM))
            throw std::runtime_error("Message too long");

        // Add message to list
        messages.push_back(message);
    }

    // Check if the unterminated message is too long
    if (incoming_buffer.length() > MAX_MSG_LEN)
        throw std::runtime_error("Message too long");

    return messages;
}

// Put message in outgoing buffer
void User::put_message(std::string message)
{
    // Check message length
    if (message.length() > MAX_MSG_LEN - strlen(LINE_DELIM))
        throw std::runtime_error("Message too long");

    // Add message to outgoing buffer (with line delimiter)
    outgoing_buffer.append(message);
    outgoing_buffer.append(LINE_DELIM);
}

// Consumes data from socket and appends it to incoming buffer
int User::read()
{
    // Read data from socket
    char buf[MAX_MSG_LEN];
    int bytes_read;
    while ((bytes_read = recv(fd, buf, MAX_MSG_LEN, MSG_DONTWAIT)) > 0)
        incoming_buffer.append(buf, bytes_read);

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
int User::write()
{
    int bytes_sent = 0;

    // Send data from outgoing buffer
    while (outgoing_buffer.length() > 0)
    {
        // Send all data from outgoing buffer
        bytes_sent = send(fd, outgoing_buffer.c_str(), outgoing_buffer.length(), MSG_DONTWAIT);

        // Return 0 if operation would block (try again later)
        if (bytes_sent < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return 0;
            throw std::runtime_error("Error sending message" + std::string(strerror(errno)));
        }

        // Remove sent data from outgoing buffer
        outgoing_buffer.erase(0, bytes_sent);
    }

    return bytes_sent;
}