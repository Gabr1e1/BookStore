#include "finance.h"
#include "account.h"
#include "command.h"

#include <iostream>
#include <string>
#include <stdexcept>

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
			if (command->Account->curLevel) std::cerr << command->Account->curUserId << "@";
			else std::cerr << "Guest User@";
			
			std::string str = "";
			getline(std::cin, str);
			auto t = command->runCommand(str);
			if (t == Exit) break;
		}
		catch (std::logic_error &error)
		{
			std::cout << error.what() << std::endl;
		}
	}
	delete command;
	return 0;
}
