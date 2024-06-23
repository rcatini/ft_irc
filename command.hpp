#pragma once

#include <vector>
#include "user.hpp"

enum cmd_type
{
	NOT_PARSED = -1,
	UNKNOWN = 0,
	PASS,
	NICK,
	USER,
	PRIVMSG,
};

class Command
{
	Server &server;
	std::string raw_message;
	std::string prefix;
	enum cmd_type type;
	std::vector<std::string> args;

public:
	Command(Server &s, std::string &input);
	void execute(User *sender);
	void print();
};
