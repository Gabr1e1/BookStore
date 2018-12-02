#ifndef Finance_H
#define Finance_H

#include <iostream>
#include <fstream>
#include <string>
#include "dataSystem.h"

static const int NumLen = 9;
static const int FinancialEventLen = NumLen * 2 + 1;

class FinancialEvent
{
public:
	int quantity;
	int price; //total price
	//both aligned to 9 characters 
	bool isRevenue;

public:
	FinancialEvent(int q, int p, bool r);
	FinancialEvent(const std::string &str);

public:
	std::string printToString();
	void print();
};

class FinanceSystem : public dataSystem
{
private:
	int size;

public:
	FinanceSystem(const std::string &file);
	~FinanceSystem();

public:
	void addEvent(int quantity, int price, bool isRevenue);
	void addEvent(FinancialEvent &event);
	void printEvent(int time);
	void printTotal();
};

#endif Finance_H
