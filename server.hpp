#ifndef SERVER_HPP
#define SERVER_HPP
#include <string>
#include <netinet/in.h>
#include <map>
#include "user.hpp"
#include "channel.hpp"

class Server
{
    uint16_t port;                                                   // Server port
    std::string password;                                            // Server password
    int fd;                                                          // Server file descriptor for socket
    bool &teardown;                                                  // Reference to teardown flag (set by signal handler)
    struct sockaddr address;                                         // Server address
    socklen_t address_length;                                        // Server address length
    std::map<int, User> users;                                       // Map of user file descriptors to user objects
    std::map<std::string, Channel> channels;                         // Map of channel names to channel objects
    int handle_server_event(struct epoll_event event);               // Handle server event
    bool handle_user_event(struct epoll_event event);                // Handle user event
    void broadcast(std::string message, int sender_descriptor = -1); // Broadcast a message to all users, except the sender

public:
    Server(bool &shutdown_ref, uint16_t port = 6667, std::string password = ""); // Constructor (starts listening on port)
    ~Server();                                                                   // Destructor (closes socket)
    void run();                                                                  // Main server loop
};

#endif