#include "channel.hpp"

Channel::Channel(const std::string &n)
	: name(n), user_limit(0), invite_only(false), protected_topic(false)
{
}

void Channel::set_user_limit(long limit)
{
	user_limit = limit;
}

void Channel::set_invite_only(bool enable)
{
	invite_only = enable;
}

void Channel::set_protected_topic(bool enable)
{
	protected_topic = enable;
}
