#ifndef LOG_H
#define LOG_H

#include <sstream>

#include "dataSystem.h"
#include "finance.h"
#include "index.h"

class Log
{
public:
	static const int StringLen = 300;
	static const int LogLen = FinancialEvent::FinancialEventLen + StringLen * 2;
public:
	FinancialEvent finance;
	std::string userId;
	std::string action;
public:
	Log(const FinancialEvent &f, const std::string &user, const std::string &act);
	Log(const std::string &str);
	~Log() = default;

public:
	std::string printToString();
};

class LogSystem : public dataSystem
{
public:
	LogSystem(const std::string &file);
	~LogSystem();

public:
	void addEvent(const FinancialEvent &f, const std::string &user, const std::string &act);
	void printFinance();
	void printEmployee(const std::string &name = "");
	void printLog();

};

#endif