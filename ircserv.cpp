#include <iostream>
#include <cstdlib>
#include <csignal>
#include <cerrno>
#include <cstring>
#include <stdint.h>
#include "server.hpp"

static bool teardown = false;

// Signal handler for SIGINT
void signal_handler(int signal)
{
    // Set teardown flag
    if (signal == SIGINT)
        teardown = true;
    std::cerr << "Caught SIG" << sigabbrev_np(signal) << std::endl;
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
    if (std::signal(SIGINT, signal_handler) == SIG_ERR)
    {
        std::cerr << "Could not set up signal handler: " << std::string(strerror(errno)) << std::endl;
        return 3;
    }

    // Run server
    try
    {
        Server server(teardown, uint16_t(port), argv[2]);
        server.run();
    }
    catch (std::runtime_error &e)
    {
        std::cerr << "Fatal error: " << e.what() << std::endl;
    }
}
