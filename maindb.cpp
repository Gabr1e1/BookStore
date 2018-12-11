#include "maindb.h"

DataType::DataType(const std::string &str)
{
	const char *t = str.c_str();
	ISBN = formatSubstr(str, 0, ISBNLen);
	name = formatSubstr(str, ISBNLen, StringLen);
	author = formatSubstr(str, ISBNLen + StringLen, StringLen);
	keyword = formatSubstr(str, ISBNLen + StringLen * 2, StringLen);
	quantity = *(reinterpret_cast<const int*>(t + ISBNLen + 3 * StringLen));
	price = *(reinterpret_cast<const double*>(t + ISBNLen + 3 * StringLen + sizeof(quantity)));
}

DataType::DataType(const std::string &_ISBN, const std::string &_name, const std::string &_author,
	const std::string &_keyword, double _price, int _quantity = 0)
	: ISBN(_ISBN), name(_name), author(_author), keyword(_keyword), price(_price), quantity(_quantity)
{
	/* Empty */
}

std::string DataType::printToString()
{
	std::ostringstream output;
	output.setf(std::ios::left);
	output << std::setw(ISBNLen) << ISBN;
	output << std::setw(StringLen) << name;
	output << std::setw(StringLen) << author;
	output << std::setw(StringLen) << keyword;

	std::string ret = output.str();
	for (size_t i = 0; i < sizeof(quantity); i++)
		ret += *(reinterpret_cast<char*>(&quantity) + i);
	for (size_t i = 0; i < sizeof(price); i++)
		ret += *(reinterpret_cast<char*>(&price) + i);
	return ret;
}

MainDatabase::MainDatabase(const std::string &file) : dataSystem(file)
{
	/* Empty */
}

MainDatabase::~MainDatabase()
{
	dataIO.seekg(0, std::ios::beg);
	dataIO.write(reinterpret_cast<char*>(&size), sizeof(size));
}

DataType MainDatabase::read(int address, int len)
{
	char *t = new char[len + 1];
	for (int i = 0; i < len + 1; i++) t[i] = 0;
	dataIO.seekg(address);
	dataIO.read(t, len);
	std::string ret = "";
	for (int i = 0; i < len; i++) ret += t[i];
	delete[] t;
	return DataType(ret);
}

