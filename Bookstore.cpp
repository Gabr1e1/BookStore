#include "finance.h"
#include "account.h"
#include "command.h"

#include <iostream>
#include <string>
#include <exception>

int main()
{
	CommandSystem *command = new CommandSystem("CommandSystem.txt");

	std::ifstream file("command.txt");
	auto t = command->runCommand("load command.txt");
	if (t == Exit)
	{
		delete command;
		return 0;
	}

	while (true)
	{
		try
		{
			std::string str = "";
			getline(std::cin, str);
			auto t = command->runCommand(str);
			if (t == Exit) break;
		}
		catch (std::exception &error)
		{
			std::cout << error.what() << std::endl;
		}
	}
	delete command;
	return 0;
}
