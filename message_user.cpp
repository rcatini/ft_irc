#include "message_user.hpp"

UserMessage::UserMessage(User &user, std::string source, std::string command, std::vector<std::string> params) : Message(source, command, params), user(user)
{
}

UserMessage::UserMessage(User &user, std::string raw_message) : Message(raw_message), user(user)
{
}
