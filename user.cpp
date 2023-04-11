#include <iostream>
#include "user.hpp"

User::User()
{
	std::cout << "User constructed" << std::endl;
}

User::~User()
{
	std::cout << "User destroyed" << std::endl;
}