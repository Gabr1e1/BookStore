#include <iostream>
#include <string>
#include <stdexcept>

#include "command.h"
#include "userinterface.hpp"

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

	std::cerr << "Welcome to the Bookstore management system!" << std::endl;
	std::cerr << "Are you an expert? Enter 1 for yes and 0 for no" << std::endl;
	bool expert;
	std::cin >> expert;
	if (!expert) UserInterface::showManual();

	while (true)
	{
		try
		{
			if (command->Account->curLevel) std::cerr << command->Account->curUserId << "@";
			else std::cerr << "Guest User@";

			std::string str = "";
			if (expert) str = UserInterface::read();
			else str = UserInterface::getInput(command->Account->curLevel == 7);

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
