#ifndef Command_H
#define Command_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <algorithm>

#include "dataSystem.h"
#include "account.h"
#include "index.h"
#include "maindb.h"
#include "finance.h"
#include "log.h"

enum ResultType { Executed, Exit };

class CommandSystem : public dataSystem
{
public:
	static const int maxCommandLen = 200;

private:
	std::fstream dataIO;
	std::vector<DataType> curSelected;

public:
	AccountSystem *Account;
	FinanceSystem *Finance;
	LogSystem *log;
	MainDatabase *mainDatabase;
	IndexDatabase *ISBNDatabase;
	IndexDatabase *nameDatabase;
	IndexDatabase *authorDatabase;
	IndexDatabase *keywordDatabase;

public:
	CommandSystem(const std::string &file);
	~CommandSystem();

private:
	std::vector<std::string> parse(const std::string &str);
	void erase(DataType data);
	bool check(const DataType &d, const DataType &req);
	void modify(DataType old, DataType data);
	void cleanup();

private:
	ResultType userCommand(std::vector<std::string> &token);
	ResultType dataCommand(std::vector<std::string> &token);
	ResultType logCommand(std::vector<std::string> &token);
	void printSelected(const DataType &req);

public:
	ResultType runCommand(const std::string &str);
	ResultType runLoadCommand(const std::string &file);
};

#endif