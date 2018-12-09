#ifndef Database_H
#define Database_H

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <iomanip>

#include "dataSystem.h"
#include <cassert>

std::string formatSubstr(const std::string &str, int start, int len);

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

class Database
{
public:
	static const int BlockSize = 20;
	static const int BlockLen = sizeof(int) * 2 + BlockSize * DataType::DataTypeLen;

private:
	std::fstream dataIO;
	int offset, readLen; //offset used when reading a DataType
	int startAddress;

public:
	Database(const std::string &file, int _offset, int len);
	~Database();

private:
	std::string readBlock(int offset, int len = BlockLen);
	std::string readWholeBlock(int address);
	void writeWholeBlock(int address, const std::string &str);
	bool inCurBlock(const std::string &key, const std::string &uniqueKey, int curSize);
	std::vector<DataType> readInsideBlock(const std::string &key, int address, int size, const std::string &uniqueKey = "");
	void eraseInsideBlock(const std::string &key, int address, int size, const std::string &uniqueKey);
	void writeInsideBlock(const std::string &key, int address, int size, const std::string &uniqueKey, const std::string &value = "");
	int split(int start, int beginEle, int size);
	int createNewBlock(int size = 0, int next = -1); //create a new block without filling the elements

public:
	DataType read(const std::string &key, const std::string &uniqueKey);
	std::vector<DataType> readAll(const std::string &key);
	void write(const std::string &key, DataType &data, const std::string &uniqueKey);
	void erase(const std::string &key, const std::string &uniqueKey);
	void cleanup();
};

#endif