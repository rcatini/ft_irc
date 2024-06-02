#include "user.hpp"
#include <string>

void User::receive_data(std::string const &data)
{
	std::string incoming_stuff = incoming_buffer + data;
	size_t eol_pos;

	if ((eol_pos = incoming_stuff.find(EOL)) != std::string::npos)
	{
		incoming_messages.push(incoming_stuff.substr(0, eol_pos));
		incoming_buffer = incoming_stuff.substr(eol_pos + 2);
	}
	else
		incoming_buffer = incoming_stuff;
}
