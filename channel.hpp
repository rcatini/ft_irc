#pragma once

#include <string>
#include <set>
#include "user.hpp"

class Channel
{
	std::string name;
	long user_limit;
	bool invite_only;
	std::set<User *> invited;
	std::string password;
	bool protected_topic;
	std::string topic;
	std::set<User *> users;
	std::set<User *> operators;

public:
	Channel(const std::string &name);
	void set_user_limit(long limit);
	void set_invite_only(bool enable);
	void set_protected_topic(bool enable);
};
