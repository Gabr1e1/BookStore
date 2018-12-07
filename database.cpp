#include "database.h"

static const int NumLen = 9;
static const int FinancialEventLen = NumLen * 2 + 1;

std::string formatSubstr(const std::string &str, int start, int len)
{
	std::string ret = str.substr(start, len);
	while (ret.length() != 0 && (ret[ret.length() - 1] == ' ' || ret[ret.length()] == 0))
		ret = ret.substr(0, ret.length() - 1);
	return ret;
}

DataType::DataType(std::string str)
{
	const char *t = str.c_str();
	ISBN = formatSubstr(str, 0, ISBNLen);
	name = formatSubstr(str, ISBNLen, StringLen);
	author = formatSubstr(str, ISBNLen + StringLen, StringLen);
	keyword = formatSubstr(str, ISBNLen + StringLen * 2, StringLen);
	price = *(reinterpret_cast<const double*>(t + ISBNLen + 3 * StringLen));
	quantity = *(reinterpret_cast<const int*>(t + ISBNLen + 3 * StringLen + sizeof(price)));
}

DataType::DataType(std::string _ISBN, std::string _name, std::string _author,
	std::string _keyword, double _price, int _quantity = 0)
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
	//std::cout << ret.length() << " " << DataTypeLen << std::endl;
	return ret;
}

Database::Database(const std::string &file, int _offset, int len) : offset(_offset), readLen(len)
{
	std::ofstream tmp(file/*, std::ios::app*/); tmp.close();
	dataIO.open(file, std::ios::binary | std::ios::in | std::ios::out);
	dataIO.seekg(0, std::ios::end);
	if (!dataIO.gcount())
	{
		startAddress = sizeof(int);
		dataIO.write(reinterpret_cast<char*>(&startAddress), sizeof(startAddress));
		int size = 0, next = -1;
		dataIO.write(reinterpret_cast<const char*>(&size), sizeof(size));
		dataIO.seekg(startAddress + BlockLen - sizeof(int));
		dataIO.write(reinterpret_cast<const char*>(&next), sizeof(next));
		if (!dataIO) throw std::exception("Create file error");
	}
	else
	{
		dataIO.seekg(0, std::ios::beg);
		dataIO.read(reinterpret_cast<char*>(&startAddress), sizeof(int));
	}
}

Database::~Database()
{
	dataIO.close();
}

std::string Database::readBlock(int address, int len)
{
	char *t = new char[len + 1];
	for (int i = 0; i < len + 1; i++) t[i] = 0;
	dataIO.seekg(address);
	dataIO.read(t, len);
	std::string ret = t;
	while (ret.length() != 0 && (ret[ret.length() - 1] == ' ' || ret[ret.length() - 1] == 0))
		ret = ret.substr(0, ret.length() - 1);
	delete[] t;
	return ret;
}

std::string Database::readWholeBlock(int address)
{
	char *t = new char[DataType::DataTypeLen + 1];
	for (int i = 0; i < DataType::DataTypeLen + 1; i++) t[i] = 0;
	dataIO.seekg(address);
	dataIO.read(t, DataType::DataTypeLen);
	std::string ret = t;
	while (ret.size() < DataType::DataTypeLen) ret += " ";
	delete[] t;
	return ret;
}

void Database::writeWholeBlock(int address, const std::string &str)
{
	dataIO.seekg(address);
	dataIO << str;
}

bool Database::inCurBlock(std::string key, std::string uniqueKey, int curSize)
{
	if (curSize == 0) return false;
	int curAddress = (int)dataIO.tellg();
	std::string strBegin = readBlock(curAddress + offset, readLen);
	std::string strBegin2 = readBlock(curAddress, DataType::StringLen);
	std::string strEnd = readBlock(curAddress + (curSize - 1) * DataType::DataType::DataTypeLen + offset, readLen);
	std::string strEnd2 = readBlock(curAddress + (curSize - 1) * DataType::DataType::DataTypeLen, DataType::StringLen);
	if (uniqueKey == "") strBegin2 = strEnd2 = "";
	return (make_pair(key,uniqueKey) >= make_pair(strBegin, strBegin2)) 
		&& (make_pair(key, uniqueKey) <= make_pair(strEnd, strEnd2));
}

std::vector<DataType> Database::readInsideBlock(std::string key, int address, int size, std::string uniqueKey)
{
	std::vector<DataType> ret;
	for (int i = 1; i <= size; i++)
	{
		std::string cur = readBlock(address + offset, readLen);
		std::string curKey = readBlock(0, DataType::StringLen);
		if (cur == key && (curKey == uniqueKey || uniqueKey == "")) ret.push_back(DataType(readWholeBlock(address)));
		address += DataType::DataType::DataTypeLen;
	}
	return ret;
}

DataType Database::read(std::string key, std::string uniqueKey)
{
	int curAddress = startAddress;
	while (true)
	{
		int curSize;
		dataIO.seekg(curAddress);
		dataIO.read(reinterpret_cast<char*>(&curSize), sizeof(curSize));

		if (inCurBlock(key, uniqueKey, curSize))
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

		if (inCurBlock(key, "", curSize))
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
	dataIO.seekg(0, std::ios::end);
	int ret = dataIO.tellg();
	dataIO.write(reinterpret_cast<char*>(&size), sizeof(size));
	dataIO.seekg(BlockLen - 2 * sizeof(int), std::ios::cur);
	dataIO.write(reinterpret_cast<char*>(&next), sizeof(next));
	return ret;
}

int Database::split(int start, int beginEle, int size)
{
	int oriNext;
	dataIO.seekg(start + BlockLen - 2 * sizeof(int));
	dataIO.read(reinterpret_cast<char*>(&oriNext), sizeof(oriNext));

	int t = createNewBlock(size - beginEle + 1, oriNext);
	int t2 = t;

	//link current to new block
	dataIO.seekg(start + BlockLen - 2 * sizeof(int));
	dataIO.write(reinterpret_cast<char*>(&t), sizeof(t));

	//resize the current block
	dataIO.seekg(start - sizeof(int));
	int curSize = beginEle - 1;
	dataIO.write(reinterpret_cast<char*>(&curSize), sizeof(curSize));

	//write elements into new block
	start = start + DataType::DataTypeLen * curSize;
	t += sizeof(int);
	for (int i = beginEle; i <= size; i++, t += DataType::DataTypeLen, start += DataType::DataTypeLen)
	{
		std::string cur = readWholeBlock(start);
		writeWholeBlock(t, cur);
	}

	return t2;
}

void Database::writeInsideBlock(std::string &key, int address, int size,
								std::string uniqueKey, std::string value)
{
	int pre = 0;
	int start = address;
	if (size == 0)
	{
		size = 1;
		dataIO.seekg(address - sizeof(int));
		dataIO.write(reinterpret_cast<char*>(&size), sizeof(size));
		dataIO << value;
		return;
	}

	for (int i = 1; i <= size; i++)
	{
		std::string cur = readBlock(address, DataType::StringLen);
		std::string curKey = readBlock(address + offset, readLen);
		if (cur == uniqueKey && curKey == key)
		{
			dataIO.seekg(address);
			dataIO << value;
			return;
		}
		if (curKey < key) pre = i;
		else break; //break if the current key > key
		address += DataType::DataTypeLen;
	}

	int t = split(start, pre + 1, size);
	int newBlock = createNewBlock(1, t);
	//link the first block to the new block
	dataIO.seekg(start + BlockLen - 2 * sizeof(int));
	dataIO.write(reinterpret_cast<char*>(&newBlock), sizeof(newBlock));
	//write data to the new block
	writeWholeBlock(newBlock + sizeof(int), value);
	return;
}

void Database::write(std::string key, DataType &data, std::string uniqueKey)
{
	int curAddress = startAddress;
	while (true)
	{
		dataIO.seekg(curAddress);
		int curSize;
		dataIO.read(reinterpret_cast<char*>(&curSize), sizeof(curSize));

		if (inCurBlock(key, uniqueKey, curSize))
		{
			writeInsideBlock(key, curAddress + sizeof(int), curSize, uniqueKey, data.printToString());
			return;
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
	//create new block at the end
	int t = createNewBlock(1);
	dataIO.seekg(curAddress);
	dataIO.write(reinterpret_cast<char*>(&t), sizeof(t));
	//write data to the new block
	writeWholeBlock(t + sizeof(int), data.printToString());
}

void Database::eraseInsideBlock(std::string &key, int address, int size,
	std::string uniqueKey)
{
	int pre = 0;
	int start = address;
	for (int i = 1; i <= size; i++)
	{
		std::string cur = readBlock(address, DataType::StringLen);
		std::string curKey = readBlock(address + offset, readLen);
		if (cur == uniqueKey && curKey == key)
		{
			int t = split(start, i + 1, size);
			dataIO.seekg(start - sizeof(int));
			dataIO.write(reinterpret_cast<const char*>(i - 1), sizeof(int));
			return;
		}
		address += DataType::DataTypeLen;
	}
}

void Database::erase(std::string key, std::string uniqueKey)
{
	int curAddress = startAddress;
	while (true)
	{
		dataIO.seekg(curAddress);
		int curSize;
		dataIO.read(reinterpret_cast<char*>(&curSize), sizeof(curSize));

		if (inCurBlock(key, uniqueKey, curSize))
		{
			eraseInsideBlock(key, curAddress + sizeof(int), curSize, uniqueKey);
			return;
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
}

void Database::cleanup()
{
	//TODO: Write cleanup()
}