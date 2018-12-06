#include "database.h"

static const int NumLen = 9;
static const int FinancialEventLen = NumLen * 2 + 1;

DataType::DataType(std::string str)
{
	char *t = (char*)str.c_str();
	ISBN = str.substr(0, ISBNLen);
	name = str.substr(ISBNLen, StringLen);
	author = str.substr(ISBNLen + StringLen, StringLen);
	keyword = str.substr(ISBNLen + StringLen * 2, StringLen);
	price = *(reinterpret_cast<double*>(t + ISBNLen + 3 * StringLen));
	quantity = *(reinterpret_cast<int*>(t + ISBNLen + 3 * StringLen + sizeof(price)));
	deleted = str[str.length() - 1] - '0';
}

DataType::DataType(std::string _ISBN, std::string _name, std::string _author,
	std::string _keyword, double _price, int _quantity)
	: ISBN(_ISBN), name(_name), author(_author), keyword(_keyword), price(_price), quantity(_quantity)
{
	/* Empty */
}

std::string DataType::printToString()
{
	std::ostringstream output;
	output << std::setw(ISBNLen) << ISBN;
	output << std::setw(StringLen) << name;
	output << std::setw(StringLen) << author;
	output << std::setw(StringLen) << keyword;

	std::string ret = output.str();
	for (size_t i = 0; i < sizeof(quantity); i++)
		ret += *(reinterpret_cast<char*>(&quantity) + i);
	for (size_t i = 0; i < sizeof(price); i++)
		ret += *(reinterpret_cast<char*>(&price) + i);
	ret += (char)(deleted + '0');
	return ret;
}

Database::Database(const std::string &file, int _offset, int len) : offset(_offset), readLen(len)
{
	std::ofstream tmp(file, std::ios::app); tmp.close();
	dataIO.open(file, std::ios::binary | std::ios::app | std::ios::out);
	dataIO.seekg(0);
	dataIO.read(reinterpret_cast<char*>(&startAddress), sizeof(int));
}

Database::~Database()
{
	dataIO.close();
}

std::string Database::readBlock(int address, int len)
{ 
	char *t = new char[len];
	dataIO.seekg(address);
	dataIO.read(t, len);
	std::string ret = t;
	delete[] t;
	return ret;
}

std::string Database::readWholeBlock(int address)
{
	char *t = new char[DataType::DataType::DataTypeLen];
	dataIO.seekg(address);
	dataIO.read(t, DataType::DataType::DataTypeLen);
	std::string ret = t;
	delete[] t;
	return ret;
}

void Database::writeWholeBlock(int address, std::string str)
{
	char *s = (char*)str.c_str();
	dataIO.seekg(address);
	dataIO.write(s, str.length());
}

bool Database::inCurBlock(std::string &key, int curSize)
{
	int curAddress = (int)dataIO.tellg();
	std::string strBegin = readBlock(curAddress + offset, readLen);
	std::string strEnd = readBlock(curAddress + curSize * DataType::DataType::DataTypeLen + offset, readLen);
	return (key >= strBegin) && (key <= strEnd);
}

std::vector<DataType> Database::readInsideBlock(std::string key, int address, int size,std::string uniqueKey)
{
	std::vector<DataType> ret;
	for (int i = 1; i <= size; i++)
	{
		std::string cur = readBlock(address, readLen);
		if (cur == key && (cur.substr(0, DataType::StringLen) == uniqueKey || uniqueKey == "")) ret.push_back(DataType(readWholeBlock(address)));
		address += DataType::DataType::DataTypeLen;
	}
	return ret;
}

DataType Database::read(std::string key, std::string uniqueKey)
{
	int curAddress = startAddress;
	while (true)
	{
		dataIO.seekg(curAddress);
		int curSize;
		dataIO.read(reinterpret_cast<char*>(&curSize), sizeof(curSize));

		if (inCurBlock(key, curSize))
		{
			std::vector<DataType> r = readInsideBlock(key, (int)dataIO.tellg(), curSize, uniqueKey);
			if (r.size() == 0) return DataType();
			else return r[0];
		}
		else
		{
			curAddress += BlockLen - sizeof(int);
			dataIO.seekg(curAddress);
			int next;
			dataIO.read(reinterpret_cast<char*>(&next), sizeof(next));
			if (next == -1) break;
			curAddress = next;
		}
	}
	return DataType();
}

std::vector<DataType> Database::readAll(std::string key)
{
	int curAddress = startAddress;
	while (true)
	{
		dataIO.seekg(curAddress);
		int curSize;
		dataIO.read(reinterpret_cast<char*>(&curSize), sizeof(curSize));

		if (inCurBlock(key, curSize))
		{
			return readInsideBlock(key, (int)dataIO.tellg(), curSize);
		}
		else
		{
			curAddress += BlockLen - sizeof(int);
			dataIO.seekg(curAddress);
			int next;
			dataIO.read(reinterpret_cast<char*>(&next), sizeof(next));
			if (next == -1) break;
			curAddress = next;
		}
	}
	return {};
}

int Database::createNewBlock(int size, int next)
{
	int ret = (int)dataIO.tellg();
	dataIO.seekg(0, std::ios::end);
	dataIO.write(reinterpret_cast<char*>(&size), sizeof(int));
	dataIO.seekg(BlockLen - sizeof(int), std::ios::cur);
	dataIO.write(reinterpret_cast<char*>(&next), sizeof(int));
	return ret;
}

int Database::split(int start, int beginEle, int size)
{
	int t = createNewBlock(size - beginEle + 1);
	int t2 = t;

	//link current to new block
	dataIO.seekg(start + BlockLen);
	dataIO.write(reinterpret_cast<char*>(&t), sizeof(t));

	//resize the current block
	dataIO.seekg(start);
	int curSize = beginEle - 1;
	dataIO.write(reinterpret_cast<char*>(&curSize), sizeof(curSize));

	//write elements into new block
	start = start + sizeof(int) + DataType::DataTypeLen * (size - 1);
	t += sizeof(int);
	for (int i = beginEle; i <= size; i++, t += DataType::DataTypeLen)
	{
		std::string cur = readWholeBlock(start);
		writeWholeBlock(t, cur);
	}

	return t2;
}

void Database::cleanup()
{
	//TODO: Write cleanup()
}

void Database::writeInsideBlock(std::string &key, int address, int size, 
	std::string uniqueKey, std::string value)
{
	int pre = 0;
	int start = address;
	for (int i = 1; i <= size; i++)
	{
		std::string cur = readBlock(address, readLen);
		if (cur == key && cur.substr(0,DataType::StringLen) == uniqueKey)
		{
			char *t = new char[DataType::DataTypeLen];
			for (int i = 0; i < DataType::DataTypeLen; i++) t[i] = cur[i];
			for (int i = offset; i < offset + readLen; i++) t[i] = key[i - offset];
			dataIO.write(t, DataType::DataTypeLen);
			return;
		}
		if (cur < key) pre = address;
		else
		{
			int t = split(start, address, size - i + 1);
			int newBlock = createNewBlock(1, t);
			//link the first block to the new block
			dataIO.seekg(start + BlockLen - sizeof(int));
			dataIO.write(reinterpret_cast<char*>(&newBlock), sizeof(int));
			//write data to the new block
			writeWholeBlock(newBlock, value);
		}
		address += DataType::DataTypeLen;
	}
}

void Database::write(std::string key, DataType &data, std::string uniqueKey)
{
	int curAddress = startAddress;
	while (true)
	{
		dataIO.seekg(curAddress);
		int curSize;
		dataIO.read(reinterpret_cast<char*>(&curSize), sizeof(curSize));

		if (inCurBlock(key, curSize)) writeInsideBlock(key, (int)dataIO.tellg(), curSize, uniqueKey, data.printToString());
		else
		{
			curAddress += BlockLen - sizeof(int);
			dataIO.seekg(curAddress);
			int next;
			dataIO.read(reinterpret_cast<char*>(&next), sizeof(next));
			if (next == -1) break;
			curAddress = next;
		}
	}
	//craete new block at the end
	int t = createNewBlock(1, -1);
	dataIO.seekg(curAddress + DataType::DataTypeLen - sizeof(int));
	dataIO.write(reinterpret_cast<char*>(&t), sizeof(t));
	//write data to the new block
	writeWholeBlock(t + sizeof(int), data.printToString());
}

void Database::erase(std::string key, std::string uniqueKey)
{
	auto data = read(key,uniqueKey);
	data.deleted = true;
	write(key, data,uniqueKey);
}
