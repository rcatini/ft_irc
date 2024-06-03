#include "user.hpp"
#include <string>
#include <iostream>

void User::receive_data(std::string const &data)
{
	incoming_buffer += data;
	size_t eol_pos;

	while ((eol_pos = incoming_buffer.find(EOL)) != std::string::npos)
	{
		incoming_messages.push(incoming_buffer.substr(0, eol_pos));
		incoming_buffer = incoming_buffer.substr(eol_pos + 2);
		std::cout << "Received message: " << incoming_messages.back() << std::endl;
	}
}
