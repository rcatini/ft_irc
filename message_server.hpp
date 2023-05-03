#ifndef MESSAGE_SERVER_HPP
#define MESSAGE_SERVER_HPP

#include "message.hpp"
#include "user.hpp"

class ServerMessage : public Message
{
    User *user;
};

#endif