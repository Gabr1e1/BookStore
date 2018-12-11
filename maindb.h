#ifndef MainDatabase_H
#define MainDatabase_H

#include "dataSystem.h"
#include "index.h"

class DataType
{
public:
	static const int ISBNLen = 20;
	static const int StringLen = 40;
	static const int DataTypeLen = ISBNLen + StringLen * 3 + sizeof(int) + sizeof(double);

public:
	std::string ISBN; //aligned to 20 characters
	std::string name, author, keyword; //aligned to 40 characters(20 chinese characters)
	double price; //current price
	int quantity;

public:
	DataType() = default;
	DataType(const std::string &str);
	DataType(const std::string &_ISBN, const std::string &_name, const std::string &_author,
		const std::string &_keyword, double _price, int _quantity);

public:
	std::string printToString();
};

class MainDatabase : public dataSystem
{
private:
	int size;

public:
	MainDatabase(const std::string &file);
	~MainDatabase();

public:
	DataType read(int address, int len = DataType::DataTypeLen);
};

#endif