#include "command.h"

Command::Command(const std::string &file)
{
	dataIO.open(file, std::ios::out | std::ios::app);
	ISBNDatabase = new Database("ISBNDatabase.txt", 0, ISBNLen);
	nameDatabase = new Database("nameDatabase.txt", ISBNLen, StringLen);

}

Command::~Command()
{
	dataIO.close();
}

ResultType Command::runLoadCommand()
{

}

ResultType Command::runCommand(const std::string &str)
{
	if (str == "EXIT") return Exit;
	
	int p = str.find(" ");
	std::string cmd;
	if (p == std::string::npos) cmd = str;
	else cmd = str.substr(0, p);

	if (cmd == "LOAD")
	{
		std::string file = str.substr(5, str.length() - 5);
		input.open(file);
		return runLoadCommand();
	}

	if (cmd == "su") {}
	else throw("Invalid");
}