#include "log.h"

Log::Log(const FinancialEvent &f, const std::string &user, const std::string &act)
		: finance(f), userId(user), action(act)
{
	/* Empty */
}

Log::Log(const std::string &str) : finance(FinancialEvent(str.substr(0, FinancialEvent::FinancialEventLen)))
{
	userId = formatSubstr(str, FinancialEvent::FinancialEventLen, StringLen);
	action = formatSubstr(str, FinancialEvent::FinancialEventLen + StringLen, StringLen);
}

std::string Log::printToString()
{
	std::ostringstream t;
	t.setf(std::ios::left);
	t << std::setw(StringLen) << userId;
	t << std::setw(StringLen) << action;
	return finance.printToString() + t.str();
}

LogSystem::LogSystem(const std::string &file) : dataSystem(file)
{
	/* Empty */
}

LogSystem::~LogSystem()
{
	dataIO.seekg(std::ios::beg);
	dataIO.write(reinterpret_cast<char*>(&size), sizeof(size));
}

void LogSystem::addEvent(const FinancialEvent &f, const std::string &user, const std::string &act)
{
	printToBack(Log(f, user, act).printToString());
	size++;
}

void LogSystem::printFinance()
{
	int address = sizeof(size);
	for (int i = 1; i <= size; i++, address += Log::LogLen)
	{
		auto t = Log(readAll(address, Log::LogLen));
		if (t.finance.quantity == 0) continue;
		std::cout.setf(std::ios::fixed);
		std::cout << t.action << " " << (t.finance.isRevenue ? "+" : "-") << " ";
		std::cout << std::setprecision(2) << t.finance.price << std::endl;
	}
}

void LogSystem::printEmployee(const std::string &name)
{
	int address = sizeof(size);
	for (int i = 1; i <= size; i++, address += Log::LogLen)
	{
		auto t = Log(readAll(address, Log::LogLen));
		if (name == "" || t.userId == name)
		{
			std::cout << t.userId << " " << t.action << std::endl;
		}
	}
}

void LogSystem::printLog()
{
	std::cout << "Finance: " << std::endl;
	printFinance();
	std::cout << "Employee: " << std::endl;
	printEmployee();
}
