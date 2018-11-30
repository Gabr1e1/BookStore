#ifndef Database_H
#define Database_H

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

static const int ISBNLen = 20;
static const int StringLen = 40;
static const int NumLen = 5;
static const int DataTypeLen = ISBNLen + StringLen * 2 + NumLen * 2;

static const int BlockSize = 100;
static const int BlockLen = sizeof(int) * 2 + BlockSize * DataTypeLen;

int stringToInteger(const std::string &str);

class DataType
{
private:
	std::string ISBN; //aligned to 20 characters
	std::string name, author; //aligned to 40 characters(20 chinese characters)
	int price, quantity; //aligned to 5 characters

public:
	DataType() = default;
	DataType(std::string str);

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
	bool inCurBlock(std::string &key, int curSize);
	
	DataType readInsideBlock(std::string &key, int address, int size);
	void writeInsideBlock(std::string &key, int address, int size);
	void split(int address);
	void createNewNode(int pre, int next, std::string &key);
	void cleanup();

public:
	DataType read(std::string &key);
	void write(std::string &key, DataType &data);
	void erase(int address);

#endif Database_H

//TODO: Add const specifier