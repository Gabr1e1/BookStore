#ifndef Index_H
#define Index_H

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <iomanip>
#include <cassert>
#include <queue>

#include "dataSystem.h"

std::string formatSubstr(const std::string &str, int start, int len);

class IndexType
{
public:
	static const int StringLen = 40;
	static const int ISBNLen = 20;
	static const int NumLen = sizeof(int);
	static const int IndexTypeLen = StringLen + ISBNLen + NumLen;

public:
	std::string key, ISBN; //aligned to 40 characters
	int corresAddress;

public:
	IndexType() = default;
	IndexType(const std::string &str);
	IndexType(const std::string &key, const std::string &_ISBN,  int address);

public:
	std::string printToString();
};

class IndexDatabase
{
public:
	static const int BlockSize = 300;
	static const int BlockLen = sizeof(int) * 2 + BlockSize * IndexType::IndexTypeLen;

private:
	std::fstream dataIO;
	int startAddress;
	std::queue<int> que;

public:
	IndexDatabase(const std::string &file);
	~IndexDatabase();

private:
	std::string readWholeBlock(int address);
	std::string readString(int address, int len);
	int readAddress(int address);
	void writeBlock(int address, const std::string &str);

	bool inCurBlock(const std::string &key, const std::string &uniqueKey, int curSize);
	std::vector<int> readInsideBlock(const std::string &key, int address, int size, const std::string &uniqueKey = "");
	
	void eraseInsideBlock(const std::string &key, int address, int size, const std::string &uniqueKey);
	void writeInsideBlock(const std::string &key, int address, int size, const std::string &uniqueKey, const std::string &value = "");
	int split(int start, int beginEle, int size);
	int createNewBlock(int size = 0, int next = -1); //create a new block without filling the elements

public:
	int read(const std::string &key, const std::string &uniqueKey);
	std::vector<int> readAll(const std::string &key);
	void write(const std::string &key, const std::string &uniqueKey, const std::string &data);
	void erase(const std::string &key, const std::string &uniqueKey);
	void cleanup();
};

#endif