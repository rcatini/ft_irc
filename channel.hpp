#ifndef CHANNEL_HPP
#define CHANNEL_HPP
#include <string>

class Channel
{
	std::string name;
	std::string topic;

public:
	Channel(std::string name, std::string topic = "");
	std::string getName();
	std::string getTopic();
	void setTopic(std::string &topic);
	~Channel();
};

#endif