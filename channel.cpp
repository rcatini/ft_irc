#include "channel.hpp"
#include <iostream>

// Constructor
Channel::Channel(std::string channel_name, std::string channel_topic) : name(channel_name), topic(channel_topic)
{
}

// Destructor
Channel::~Channel()
{
}

// Get channel name
std::string Channel::getName()
{
    return name;
}

// Get channel topic
std::string Channel::getTopic()
{
    return topic;
}

// Set channel topic
void Channel::setTopic(std::string &new_topic)
{
    this->topic = new_topic;
}