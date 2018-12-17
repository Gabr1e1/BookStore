#ifndef Finance_H
#define Finance_H

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <iomanip>

#include "dataSystem.h"

class FinancialEvent
{
public:
	static const int FinancialEventLen = sizeof(int) + sizeof(double) + 1;

public:
	int quantity;
	double price; //total price
	bool isRevenue;

public:
	FinancialEvent(int q = 0, double p = 0, bool r = 0);
	FinancialEvent(const std::string &str);

public:
	std::string printToString();
};

class FinanceSystem : public dataSystem
{
private:
	int size;

public:
	FinanceSystem(const std::string &file);
	~FinanceSystem();

protected:
	std::string read(int address, int len);

public:
	void addEvent(int quantity, double price, bool isRevenue);
	void addEvent(FinancialEvent &event);
	void printEvent(int time = 0);
};

#endif
