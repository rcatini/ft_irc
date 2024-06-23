#include <iostream>
#include "command.hpp"

Command::Command(Server &s, std::string &input) : server(s), raw_message(input), type(NOT_PARSED)
{
	std::istringstream iss(input);
	std::string token;

	if (iss.peek() == '@')
	{
		std::string tags;
		std::getline(iss, tags, ' ');
	}

	if (iss.peek() == ':')
	{
		std::getline(iss, token, ' ');
		prefix = token.substr(1);
	}

	std::getline(iss >> std::ws, token, ' ');
	if (token == "NICK")
		type = NICK;
	else if (token == "USER")
		type = USER;
	else if (token == "PASS")
		type = PASS;
	else if (token == "PRIVMSG")
		type = PRIVMSG;
	else
		type = UNKNOWN;

	while (iss && !iss.eof() && iss.peek() != ':')
	{
		std::getline(iss >> std::ws, token, ' ');
		args.push_back(token);
	}

	if (iss && iss.peek() == ':')
	{
		iss.get();
		std::getline(iss, token);
		args.push_back(token);
	}

	if (!iss.eof())
		throw std::runtime_error("Command::Command: input not completely consumed");
}

void Command::print()
{
	std::cout << "raw_message: " << raw_message << std::endl;
	std::cout << "prefix: " << prefix << std::endl;
	std::cout << "type: " << type << std::endl;
	std::cout << "args: ";
	for (std::vector<std::string>::iterator it = args.begin(); it != args.end(); ++it)
		std::cout << "'" << *it << "' ";
	std::cout << std::endl;
}

void Command::execute(User *sender)
{
	(void)server;
	switch (type)
	{
	case UNKNOWN:
		break;
	case PASS:
		if (sender->authenticate(args[0]))
			std::cout << "User " << sender->get_nickname() << " authenticated" << std::endl;
		else
			throw std::runtime_error("Authentication failed");
		break;
	case NICK:
		if (server.nick_exists(args[0]))
			throw std::runtime_error("Nickname already in use");
		else
			sender->set_nickname(args[0]);
	case USER:
		sender->queue_message("001 " + sender->get_nickname() + " :Welcome to the Internet Relay Network " + sender->get_nickname() + "!" + sender->get_nickname() + "@localhost");
		sender->queue_message("002 " + sender->get_nickname() + " :Your host is 42, running version 0.0.1");
		sender->queue_message("003 " + sender->get_nickname() + " :This server was created today");
		sender->queue_message("004 " + sender->get_nickname() + " localhost 1.0.0 aoOirw abiklmnoprst");
		sender->queue_message("005 " + sender->get_nickname() + " CHANTYPES=#& PREFIX=(ov)@+ NETWORK=IRCnet CASEMAPPING=ascii");
		break;
	case PRIVMSG:
	case NOT_PARSED:
		break;
	}
}
