#include "database.h"

static const int NumLen = 9;
static const int FinancialEventLen = NumLen * 2 + 1;

std::string formatSubstr(const std::string &str, int start, int len)
{
	std::string ret = str.substr(start, len);
	while (ret.length() != 0 && (ret[ret.length() - 1] == ' ' || ret[ret.length() - 1] == 0)) ret.pop_back();
	return ret;
}

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

Database::Database(const std::string &file, int _offset, int len) : offset(_offset), readLen(len)
{
	std::ofstream tmp(file, std::ios::app); tmp.close();
	dataIO.open(file, std::ios::binary | std::ios::in | std::ios::out);
	dataIO.seekg(0, std::ios::end);
	if (!dataIO.tellg())
	{
		startAddress = sizeof(int);
		dataIO.write(reinterpret_cast<char*>(&startAddress), sizeof(startAddress));
		int size = 0, next = -1;
		dataIO.write(reinterpret_cast<const char*>(&size), sizeof(size));
		dataIO.seekg(startAddress + BlockLen - sizeof(int));
		dataIO.write(reinterpret_cast<const char*>(&next), sizeof(next));
		if (!dataIO) throw std::logic_error("Create file error");
	}
	else
	{
		dataIO.seekg(0, std::ios::beg);
		dataIO.read(reinterpret_cast<char*>(&startAddress), sizeof(startAddress));
	}
}

Database::~Database()
{
	dataIO.seekg(0, std::ios::beg);
	dataIO.write(reinterpret_cast<char*>(&startAddress), sizeof(startAddress));
	dataIO.close();
}

std::string Database::readBlock(int address, int len)
{
	char *t = new char[len + 1];
	for (int i = 0; i < len + 1; i++) t[i] = 0;
	dataIO.seekg(address);
	dataIO.read(t, len);
	std::string ret = "";
	for (int i = 0; i < len; i++) ret += t[i];
	while (ret.length() != 0 && (ret[ret.length() - 1] == ' ' || ret[ret.length() - 1] == 0)) ret.pop_back();
	delete[] t;
	return ret;
}

std::string Database::readWholeBlock(int address)
{
	char *t = new char[DataType::DataTypeLen + 1];
	for (int i = 0; i < DataType::DataTypeLen + 1; i++) t[i] = 0;
	dataIO.seekg(address);
	dataIO.read(t, DataType::DataTypeLen);
	std::string ret = "";
	for (int i = 0; i < DataType::DataTypeLen; i++) ret += t[i];
	delete[] t;
	return ret;
}

void Database::writeWholeBlock(int address, const std::string &str)
{
	dataIO.seekg(address);
	dataIO << str;
	//assert((int)dataIO.tellg() - address == str.length());
}

bool Database::inCurBlock(const std::string &key, const std::string &uniqueKey, int curSize)
{
	if (curSize == 0) return false;
	int curAddress = (int)dataIO.tellg();
	std::string &&strBegin = readBlock(curAddress + offset, readLen);
	std::string &&strBegin2 = readBlock(curAddress, DataType::ISBNLen);
	std::string &&strEnd = readBlock(curAddress + (curSize - 1) * DataType::DataTypeLen + offset, readLen);
	std::string &&strEnd2 = readBlock(curAddress + (curSize - 1) * DataType::DataTypeLen, DataType::ISBNLen);
	if (uniqueKey == "") strBegin2 = strEnd2 = "";
	return (make_pair(key, uniqueKey) >= make_pair(strBegin, strBegin2))
		&& (make_pair(key, uniqueKey) <= make_pair(strEnd, strEnd2));
}

std::vector<DataType> Database::readInsideBlock(const std::string &key, int address, int size, const std::string &uniqueKey)
{
	std::vector<DataType> ret;
	for (int i = 1; i <= size; i++)
	{
		std::string &&cur = readBlock(address + offset, readLen);
		std::string &&curKey = readBlock(address, DataType::ISBNLen);
		if ((cur == key || key == "") && (curKey == uniqueKey || uniqueKey == "")) ret.push_back(DataType(readWholeBlock(address)));
		address += DataType::DataTypeLen;
	}
	return ret;
}

DataType Database::read(const std::string &key, const std::string &uniqueKey)
{
	int curAddress = startAddress;
	while (true)
	{
		int curSize;
		dataIO.seekg(curAddress);
		dataIO.read(reinterpret_cast<char*>(&curSize), sizeof(curSize));

		if (inCurBlock(key, uniqueKey, curSize))
		{
			std::vector<DataType> r = readInsideBlock(key, curAddress + sizeof(int), curSize, uniqueKey);
			if (r.size() == 0) return DataType();
			else return r[0];
		}
		else
		{
			dataIO.seekg(curAddress + BlockLen - sizeof(int));
			int next;
			dataIO.read(reinterpret_cast<char*>(&next), sizeof(next));
			if (next == -1) break;
			curAddress = next;
		}
	}
	return DataType();
}

std::vector<DataType> Database::readAll(const std::string &key)
{
	int curAddress = startAddress;
	std::vector<DataType> ret = {};
	bool flg = false;
	while (true)
	{
		dataIO.seekg(curAddress);
		int curSize;
		dataIO.read(reinterpret_cast<char*>(&curSize), sizeof(curSize));

		if (inCurBlock(key, "", curSize) || key == "")
		{
			flg = true;
			auto t = readInsideBlock(key, curAddress + sizeof(int), curSize);
			ret.insert(ret.end(), t.begin(), t.end());
		}
		else
		{
			if (flg && key != "" && curSize != 0) break;
		}
		dataIO.seekg(curAddress + BlockLen - sizeof(int));
		int next;
		dataIO.read(reinterpret_cast<char*>(&next), sizeof(next));
		if (next == -1) break;
		curAddress = next;
	}
	return ret;
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
		std::string &&cur = readWholeBlock(start);
		writeWholeBlock(t, cur);
	}
	return t2;
}

void Database::writeInsideBlock(const std::string &key, int address, int size,
	const std::string &uniqueKey, const std::string &value)
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
		std::string &&cur = readBlock(address, DataType::ISBNLen);
		std::string &&curKey = readBlock(address + offset, readLen);
		if (cur == uniqueKey && curKey == key)
		{
			dataIO.seekg(address);
			dataIO << value;
			return;
		}
		//std::cout << curKey << " " << cur << " " << key << " " << uniqueKey << std::endl;
		if (make_pair(curKey, cur) < make_pair(key, uniqueKey)) pre = i;
		else break; //break if the current key > key
		address += DataType::DataTypeLen;
	}
	if (size + 1 < BlockSize)
	{
		int tAdd = start + DataType::DataTypeLen * (size - 1);
		for (int i = size; i >= pre + 1; i--, tAdd -= DataType::DataTypeLen)
		{
			writeWholeBlock(tAdd + DataType::DataTypeLen, readWholeBlock(tAdd));
		}
		writeWholeBlock(tAdd + DataType::DataTypeLen, value);
		size++;
		dataIO.seekg(start - sizeof(int));
		dataIO.write(reinterpret_cast<char*>(&size), sizeof(size));
		return;
	}

	int t = split(start, pre + 1, size);
	int newBlock = createNewBlock(1, t);
	//link the first block to the new block
	dataIO.seekg(start + BlockLen - 2 * sizeof(int));
	dataIO.write(reinterpret_cast<char*>(&newBlock), sizeof(newBlock));
	//write data to the new block
	writeWholeBlock(newBlock + sizeof(int), value);
}

void Database::write(const std::string &key, DataType &data, const std::string &uniqueKey)
{
	int curAddress = startAddress;
	int pre = -1;

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
			if (curSize >= 1)
			{
				std::string &&strEnd = readBlock(curAddress + sizeof(int) + (curSize - 1) * DataType::DataType::DataTypeLen + offset, readLen);
				std::string &&strEnd2 = readBlock(curAddress + sizeof(int) + (curSize - 1) * DataType::DataType::DataTypeLen, DataType::ISBNLen);
				//std::cerr << key << " " << strEnd << " " << (key > strEnd) << std::endl;
				if (make_pair(strEnd, strEnd2) < make_pair(key, uniqueKey)) pre = curAddress;
			}
			dataIO.seekg(curAddress + BlockLen - sizeof(int));
			int next;
			dataIO.read(reinterpret_cast<char*>(&next), sizeof(next));
			if (next == -1) break;
			curAddress = next;
		}
	}

	int t;
	if (pre == -1) //create block at the front
	{
		t = createNewBlock(1, startAddress);
		startAddress = t;
	}
	else  //create new block at the right place
	{
		int next;
		dataIO.seekg(pre + BlockLen - sizeof(int));
		dataIO.read(reinterpret_cast<char*>(&next), sizeof(next));
		t = createNewBlock(1, next);
		dataIO.seekg(pre + BlockLen - sizeof(int));
		dataIO.write(reinterpret_cast<char*>(&t), sizeof(t));
	}
	writeWholeBlock(t + sizeof(int), data.printToString()); //write data to the new block
}

void Database::eraseInsideBlock(const std::string &key, int address, int size,
	const std::string &uniqueKey)
{
	int pre = 0;
	int start = address;
	for (int i = 1; i <= size; i++)
	{
		std::string &&cur = readBlock(address, DataType::ISBNLen);
		std::string &&curKey = readBlock(address + offset, readLen);
		if (cur == uniqueKey && curKey == key)
		{
			int tAdd = address;
			for (int j = i + 1; j <= size; j++, tAdd += DataType::DataTypeLen)
			{
				writeWholeBlock(tAdd, readWholeBlock(tAdd + DataType::DataTypeLen));
			}

			int newSize = size - 1;
			dataIO.seekg(start - sizeof(int));
			dataIO.write(reinterpret_cast<char*>(&newSize), sizeof(int));
			return;
		}
		address += DataType::DataTypeLen;
	}
}

void Database::erase(const std::string &key, const std::string &uniqueKey)
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
			dataIO.seekg(curAddress + BlockLen - sizeof(int));
			int next;
			dataIO.read(reinterpret_cast<char*>(&next), sizeof(next));
			if (next == -1) break;
			curAddress = next;
		}
	}
}

void Database::cleanup()
{
	int curAddress = startAddress;
	while (true)
	{
		dataIO.seekg(curAddress);
		int curSize;
		dataIO.read(reinterpret_cast<char*>(&curSize), sizeof(curSize));

		dataIO.seekg(curAddress + BlockLen - sizeof(int));
		int next, nextSize;
		dataIO.read(reinterpret_cast<char*>(&next), sizeof(next));
		if (next == -1) break;

		dataIO.seekg(next);
		dataIO.read(reinterpret_cast<char*>(&nextSize), sizeof(nextSize));
		int nextOfNext;
		dataIO.seekg(next + BlockLen - sizeof(int));
		dataIO.read(reinterpret_cast<char*>(&nextOfNext), sizeof(nextOfNext));

		if (curSize + nextSize < BlockSize)
		{
			int a1 = curAddress + sizeof(int) + curSize * DataType::DataTypeLen;
			int a2 = next + sizeof(int);
			for (int i = 1; i <= nextSize; i++, a1 += DataType::DataTypeLen, a2 += DataType::DataTypeLen)
			{
				auto &&t = readWholeBlock(a2);
				writeWholeBlock(a1, t);
			}
			curSize += nextSize;
			dataIO.seekg(curAddress);
			dataIO.write(reinterpret_cast<char*>(&curSize), sizeof(curSize));
			dataIO.seekg(curAddress + BlockLen - sizeof(int));
			dataIO.write(reinterpret_cast<char*>(&nextOfNext), sizeof(nextOfNext));
		}
		else curAddress = next;
	}
}
