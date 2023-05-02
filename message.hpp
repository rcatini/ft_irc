#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>

class Message : public std::string
{
    std::string source;
    std::string command;
    std::vector<std::string> params;

public:
    Message(std::string source, std::string command, std::vector<std::string> params);
    Message(std::string raw_message);
};

#endif