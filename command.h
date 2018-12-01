#ifndef Command_H
#define Command_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "account.h"
#include "database.h"

enum ResultType { Executed, Exit };

class CommandSystem
{
private:
	std::fstream dataIO;
	std::vector<DataType> curSelected;

private:
	AccountSystem *Account;
	Database *ISBNDatabase;
	Database *nameDatabase;
	Database *authorDatabase;
	Database *keywordDatabase;

public:
	CommandSystem(const std::string &file);
	~CommandSystem();

private:
	std::vector<std::string> parse(std::string str);
	void modify(DataType &data);

public:
	ResultType runCommand(const std::string &str);
	ResultType runLoadCommand(const std::string &file);
};

#endif Command_H