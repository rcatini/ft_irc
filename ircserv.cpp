#include <iostream>
#include <cstdlib>
#include <csignal>
#include <cerrno>
#include <stdint.h>
#include "server.hpp"

static bool teardown = false;

// Signal handler for SIGINT
void signal_handler(int signal)
{
    // Set teardown flag
    if (signal == SIGINT)
        teardown = true;
}

int main(int argc, char *argv[])
{
    // Check number of arguments (port number and password)
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << "./ircserv <port> <password>" << std::endl;
        return 1;
    }

    // Check port number (reject non-numeric or out-of-range ports)
    char *endpointer;
    long port = std::strtol(argv[1], &endpointer, 10);
    if (*endpointer != '\0' || port <= 0 || port > UINT16_MAX)
    {
        std::cerr << "Invalid port number" << std::endl;
        return 2;
    }

    // Set up signal handler
    std::signal(SIGINT, signal_handler);

    // Run server
    try
    {
        Server server(teardown, port, argv[2]);
        server.run();
    }
    catch (std::runtime_error &e)
    {
        std::cout << "Fatal error: " << e.what() << std::endl;
    }
}
