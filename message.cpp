#include "message.hpp"

Message::Message(std::string src, std::string cmd, std::vector<std::string> prms) : source(src), command(cmd), params(prms)
{
}

Message::Message(std::string raw_message)
{
    *this = raw_message;

    // Parse source
    if (raw_message[0] == ':')
    {
        std::string::size_type end = raw_message.find(' ');
        this->source = raw_message.substr(1, end - 1);
        raw_message = raw_message.substr(end + 1);
    }

    // Parse command
    {
        std::string::size_type end = raw_message.find(' ');
        this->command = raw_message.substr(0, end);
        if (end == std::string::npos)
            return;
        raw_message = raw_message.substr(end + 1);
    }

    // Parse params
    while (raw_message.length() > 0)
    {
        if (raw_message[0] == ':')
        {
            this->params.push_back(raw_message.substr(1));
            break;
        }
        else
        {
            std::string::size_type end = raw_message.find(' ');
            this->params.push_back(raw_message.substr(0, end));
            raw_message = raw_message.substr(end + 1);
        }
    }
}