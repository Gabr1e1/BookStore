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
#include "maindb.h"

typedef unsigned long long ull;

const ull B = 9;

std::string formatSubstr(const std::string &str, int start, int len);
ull getHash(const std::string &str);

class IndexType
{
public:
	static const int StringLen = sizeof(ull);
	static const int ISBNLen = sizeof(ull);
	static const int NumLen = sizeof(int);
	static const int IndexTypeLen = StringLen + ISBNLen + NumLen;

public:
	ull key;
	ull ISBN;
	int corresAddress;

public:
	IndexType() = default;
	IndexType(const std::string &str);
	IndexType(const std::string &key, const std::string &_ISBN,  int address);

public:
	std::string printToString();
};

class MainDatabase;

class IndexDatabase
{
public:
	static const int BlockSize = 500;
	static const int BlockLen = sizeof(int) * 2 + BlockSize * IndexType::IndexTypeLen;

private:
	std::fstream dataIO;
	int startAddress;
	std::queue<int> que;
	MainDatabase *maindb;

public:
	IndexDatabase(const std::string &file, MainDatabase *_db);
	~IndexDatabase();

private:
	std::string readWholeBlock(int address, bool reSeek = true);
	template<typename T>
	T readNum(int address, bool reSeek = true);

	int readAddress(int address, bool reSeek = true);
	ull readISBN(int address, bool reSeek = true);
	ull readKey(int address, bool reSeek = true);

	void writeBlock(int address, const std::string &str, bool reSeek = true);

	bool inCurBlock(ull key, ull uniqueKey, int curSize);
	std::vector<int> readInsideBlock(ull key, int address, int size, ull uniqueKey = 0, const std::string &uniqueStr = "");
	
	void eraseInsideBlock(ull key, int address, int size, ull uniqueKey, const std::string &uniqueStr);
	void writeInsideBlock(ull key, int address, int size, ull uniqueKey, const std::string &value = "", const std::string &uniqueStr = "");
	int split(int start, int beginEle, int size);
	int createNewBlock(int size = 0, int next = -1); //create a new block without filling the elements

public:
	int read(const std::string &_key, const std::string &_uniqueKey);
	std::vector<int> readAll(const std::string &_key);
	void write(const std::string &_key, const std::string &_uniqueKey, const std::string &data);
	void erase(const std::string &_key, const std::string &_uniqueKey);
	void cleanup();
};

#endif