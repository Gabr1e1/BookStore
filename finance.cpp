#include "finance.h"

FinancialEvent::FinancialEvent(int q, double p, bool r) : quantity(q), price(p), isRevenue(r)
{
	/* Empty */
}

FinancialEvent::FinancialEvent(const std::string &str)
{
	char *t = (char*)str.c_str();
	quantity = *(reinterpret_cast<int*>(t));
	price = *(reinterpret_cast<double*>(t + sizeof(int)));
	isRevenue = (bool)(str[str.length() - 1] - '0');
}

std::string FinancialEvent::printToString()
{
	std::string ret = "";
	for (size_t i = 0; i < sizeof(quantity); i++)
		ret += *(reinterpret_cast<char*>(&quantity) + i);
	for (size_t i = 0; i < sizeof(price); i++)
		ret += *(reinterpret_cast<char*>(&price) + i);
	return ret;
}

void FinancialEvent::print()
{
	std::cout << (isRevenue) ? "+ " : "- ";
	std::cout << price << "\n";
}

FinanceSystem::FinanceSystem(const std::string &file) : dataSystem(file)
{
	/* Empty */
}

FinanceSystem::~FinanceSystem()
{
	dataIO.seekg(std::ios::beg);
	dataIO.write(reinterpret_cast<char*>(&size), sizeof(size));
}

void FinanceSystem::addEvent(int quantity, double price, bool isRevenue)
{
	printToBack(FinancialEvent(quantity, price, isRevenue).printToString());
	size++;
}

void FinanceSystem::addEvent(FinancialEvent &event)
{
	printToBack(event.printToString);
	size++;
}

void FinanceSystem::printEvent(int time)
{
	int curAddress = sizeof(int) + (size - time) * FinancialEventLen;
	for (int i = size - time + 1; i <= size; i++, curAddress += FinancialEventLen)
	{
		FinancialEvent(read(curAddress, FinancialEventLen)).print();
	}
}

void FinanceSystem::printTotal()
{
	int quantity = 0, price = 0;
	for (int i = 1, curAddress = sizeof(int); i <= size; i++, curAddress += FinancialEventLen)
	{
		FinancialEvent t = FinancialEvent(read(curAddress, FinancialEventLen));
		price += t.price;
		quantity += t.quantity;
	}
	FinancialEvent(quantity, price, quantity >= 0).print();
}
