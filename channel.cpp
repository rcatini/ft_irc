#include "channel.hpp"
#include <iostream>
Channel::Channel(std::string channel_name, std::string channel_topic) : name(channel_name), topic(channel_topic)
{
}

std::string Channel::getName()
{
	return name;
}

std::string Channel::getTopic()
{
	return topic;
}

void Channel::setTopic(std::string &new_topic)
{
	this->topic = new_topic;
}

Channel::~Channel()
{
}