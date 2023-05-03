#ifndef MESSAGE_USER_HPP
#define MESSAGE_USER_HPP
#include "message.hpp"
#include "user.hpp"

class UserMessage : public Message
{
    User &user;
    UserMessage(User &user, std::string source, std::string command, std::vector<std::string> params);
    UserMessage(User &user, std::string raw_message);
};

#endif