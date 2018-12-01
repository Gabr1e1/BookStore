#ifndef Database_H
#define Database_H

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

static const int ISBNLen = 20;
static const int StringLen = 40;
static const int NumLen = 5;
static const int DataTypeLen = ISBNLen + StringLen * 3 + NumLen * 2 + 1;

static const int BlockSize = 100;
static const int BlockLen = sizeof(int) * 2 + BlockSize * DataTypeLen;

int stringToInteger(const std::string &str);

class DataType
{
public:
	std::string ISBN; //aligned to 20 characters
	std::string name, author, keyword; //aligned to 40 characters(20 chinese characters)
	int price, quantity; //aligned to 5 characters
public:
	bool deleted = false;

public:
	DataType() = default;
	DataType(std::string str);
	DataType(std::string _ISBN, std::string _name, std::string _author,
		std::string _keyword, int _price, int _quantity);

public:
	std::string printToString();
};

class Database
{
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
	void writeWholeBlock(int address, std::string str);
	bool inCurBlock(std::string &key, int curSize);

	DataType readInsideBlock(std::string &key, int address, int size);
	void writeInsideBlock(std::string &key, int address, int size, std::string value = "");
	int split(int start, int beginEle, int size);
	int createNewBlock(int size = 0, int next = -1); //create a new block without filling the elements
	void cleanup();

public:
	DataType read(std::string &key);
	void write(std::string key, DataType &data);
	void erase(std::string key);
};

#endif Database_H

//TODO: Add const specifier