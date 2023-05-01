#ifndef USER_HPP
#define USER_HPP

#include <string>
#include <list>
#include <sys/socket.h>
#include "user.hpp"

class User
{
    int fd;                                   // File descriptor for socket
    std::string incoming_buffer;              // Buffer for incoming messages
    std::string outgoing_buffer;              // Buffer for outgoing messages
    std::list<std::string> incoming_messages; // List of incoming messages

public:
    User(int fd);                          // Constructor
    ~User();                               // Destructor
    User(const User &other);               // Copy constructor
    ssize_t read();                        // Read data from socket
    ssize_t write();                       // Write data to socket
    bool put_message(std::string message); // Put message in outgoing buffer
    std::string get_message();             // Extract message from incoming buffer
    bool has_incoming_messages();          // Check if there are messages in incoming buffer
    bool has_outgoing_messages();          // Check if there are messages in outgoing buffer
};

#endif