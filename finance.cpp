#include "finance.h"

FinancialEvent::FinancialEvent(int q, double p, bool r) : quantity(q), price(p), isRevenue(r)
{
	/* Empty */
}

FinancialEvent::FinancialEvent(const std::string &str)
{
	const char *t = str.c_str();
	quantity = *(reinterpret_cast<const int*>(t));
	price = *(reinterpret_cast<const double*>(t + sizeof(int)));
	isRevenue = (bool)(str[str.length() - 1] - '0');
}

std::string FinancialEvent::printToString()
{
	std::string ret = "";
	for (size_t i = 0; i < sizeof(quantity); i++)
		ret += *(reinterpret_cast<char*>(&quantity) + i);
	for (size_t i = 0; i < sizeof(price); i++)
		ret += *(reinterpret_cast<char*>(&price) + i);
	ret += (char)(isRevenue + '0');
	return ret;
}

FinanceSystem::FinanceSystem(const std::string &file) : dataSystem(file)
{
	/* Empty */
}

FinanceSystem::~FinanceSystem()
{
	dataIO.seekg(0, std::ios::beg);
	dataIO.write(reinterpret_cast<char*>(&size), sizeof(size));
}

std::string FinanceSystem::read(int address, int len)
{
	char *t = new char[len + 1];
	for (int i = 0; i < len + 1; i++) t[i] = 0;
	dataIO.seekg(address);
	dataIO.read(t, len);
	std::string ret = "";
	for (int i = 0; i < len; i++) ret += t[i];
	delete[] t;
	return ret;
}

void FinanceSystem::addEvent(int quantity, double price, bool isRevenue)
{
	printToBack(FinancialEvent(quantity, price, isRevenue).printToString());
	size++;
}

void FinanceSystem::addEvent(FinancialEvent &event)
{
	printToBack(event.printToString());
	size++;
}

void FinanceSystem::printEvent(int time)
{
	if (time == 0) time = size;
	time = std::min(time, size);
	double priceRevenue = 0, pricePaid = 0;
	int curAddress = sizeof(int) + (size - time) * FinancialEvent::FinancialEventLen;
	for (int i = size - time + 1; i <= size; i++, curAddress += FinancialEvent::FinancialEventLen)
	{
		auto t = FinancialEvent(read(curAddress, FinancialEvent::FinancialEventLen));
		if (t.isRevenue) priceRevenue += t.price;
		else pricePaid += t.price;
	}
	std::cout.setf(std::ios::fixed);
	std::cout << std::setprecision(2) << "+ " << priceRevenue << " - " << pricePaid << std::endl;
}

