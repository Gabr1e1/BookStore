#ifndef Command_H
#define Command_H

#include <iostream>
#include <string>
#include <fstream>
#include "account.h"
#include "database.h"

enum ResultType { Executed, Exit };

const int userCommandNum = 6;
const std::string userCommand[] = { "su", "logout", "useradd", "register", "delete", "passwd" };

const int dataCommandNum = 5;
const std::string dataCommand[] = { "select", "modify", "import", "show", "buy" };

class Command
{
private:
	std::fstream dataIO;
	std::ifstream input;

private:
	AccountSystem *Account;
	Database *ISBNDatabase;
	Database *nameDatabase;
	Database *authorDatabase;
	Database *keywordDatabase;

public:
	Command(const std::string &file);
	~Command();

public:
	ResultType runLoadCommand();
	ResultType runCommand(const std::string &str);
};

#endif Command_H