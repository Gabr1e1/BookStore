#include "finance.h"
#include "account.h"
#include "command.h"

#include <iostream>
#include <string>
#include <exception>

int main()
{
	std::cout << "Bookstore System" << std::endl;
	CommandSystem *command = new CommandSystem("CommandSystem.txt");

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
}
