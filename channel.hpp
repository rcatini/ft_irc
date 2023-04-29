#ifndef CHANNEL_HPP
#define CHANNEL_HPP
#include <string>

class Channel
{
	std::string name;  // Channel name
	std::string topic; // Channel topic

public:
	Channel(std::string name, std::string topic = ""); // Constructor
	~Channel();										   // Destructor
	std::string getName();							   // Get channel name
	std::string getTopic();							   // Get channel topic
	void setTopic(std::string &topic);				   // Set channel topic
};

#endif