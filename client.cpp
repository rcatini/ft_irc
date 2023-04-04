#include <stdexcept>
#include <string>
#include <cerrno>
#include <cstring>
#include "client.hpp"

Client::Client(int connection_descriptor) : connection_descriptor(connection_descriptor)
{
}