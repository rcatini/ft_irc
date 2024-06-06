#pragma once

#include <vector>
#include "user.hpp"

enum cmd_type
{
	UNKNOWN = 0,
	NICK,
	USER,
	PASS,
	PRIVMSG,
};

class Command
{
	User *sender;
	enum cmd_type type;
	std::vector<std::string> args;

public:
	void execute(User *sender);
};
