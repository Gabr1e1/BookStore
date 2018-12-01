#include "database.h"

int stringToInteger(const std::string &str)
{
	std::istringstream stream(str);
	int value;
	stream >> value;
	return value;
}

DataType::DataType(std::string str)
{
	ISBN = str.substr(0, ISBNLen);
	name = str.substr(ISBNLen, StringLen);
	author = str.substr(ISBNLen + StringLen, StringLen);
	keyword = str.substr(ISBNLen + StringLen * 2, StringLen);
	price = stringToInteger(str.substr(ISBNLen + StringLen * 3, NumLen));
	quantity = stringToInteger(str.substr(ISBNLen + StringLen * 3 + NumLen, NumLen));
	deleted = str[str.length() - 1] - '0';
}

DataType::DataType(std::string _ISBN, std::string _name, std::string _author,
	std::string _keyword, int _price, int _quantity)
	: ISBN(_ISBN), name(_name), author(_author), keyword(_keyword), price(_price), quantity(_quantity)
{
	/* Empty */
}

std::string DataType::printToString()
{
	char s[ISBNLen + StringLen * 2 + NumLen * 2];
	sprintf(s, "%-20s", ISBN);
	sprintf(s + ISBNLen, "%-40s", name);
	sprintf(s + ISBNLen + StringLen, "%-40s", author);
	sprintf(s + ISBNLen + 2 * StringLen, "%-40s", keyword);
	sprintf(s + ISBNLen + 3 * StringLen, "%-5d", price);
	sprintf(s + ISBNLen + 3 * StringLen + NumLen, "%-5d", quantity);
	std::string ret = s + (char)(deleted + '0');
	return ret;
}

Database::Database(const std::string &file, int _offset, int len) : offset(_offset), readLen(len)
{
	dataIO.open(file, std::ios::app | std::ios::out);
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
	char *t = new char[DataTypeLen];
	dataIO.seekg(address);
	dataIO.read(t, DataTypeLen);
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
	int curAddress = dataIO.tellg();
	std::string strBegin = readBlock(curAddress + offset, readLen);
	std::string strEnd = readBlock(curAddress + curSize * DataTypeLen + offset, readLen);
	return (key >= strBegin) && (key <= strEnd);
}

DataType Database::readInsideBlock(std::string &key, int address, int size)
{
	for (int i = 1; i <= size; i++)
	{
		std::string cur = readBlock(address, readLen);
		if (cur == key) return readWholeBlock(address);
		address += DataTypeLen;
	}
	return DataType();
}

DataType Database::read(std::string &key)
{
	int curAddress = startAddress;
	while (true)
	{
		dataIO.seekg(curAddress);
		int curSize;
		dataIO.read(reinterpret_cast<char*>(&curSize), sizeof(curSize));

		if (inCurBlock(key, curSize)) return readInsideBlock(key, dataIO.tellg(), curSize);
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

int Database::createNewBlock(int size, int next)
{
	dataIO.seekg(0, std::ios::end);
	dataIO.write(reinterpret_cast<char*>(&size), sizeof(int));
	dataIO.seekg(BlockLen - sizeof(int), std::ios::cur);
	dataIO.write(reinterpret_cast<char*>(&next), sizeof(int));
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
	start = start + sizeof(int) + DataTypeLen * (size - 1);
	t += sizeof(int);
	for (int i = beginEle; i <= size; i++, t += DataTypeLen)
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

void Database::writeInsideBlock(std::string &key, int address, int size, std::string value)
{
	int pre = 0;
	int start = address;
	for (int i = 1; i <= size; i++)
	{
		std::string cur = readBlock(address, readLen);
		if (cur == key)
		{
			char *t = new char[DataTypeLen];
			for (int i = 0; i < DataTypeLen; i++) t[i] = cur[i];
			for (int i = offset; i < offset + readLen; i++) t[i] = key[i - offset];
			dataIO.write(t, DataTypeLen);
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
		address += DataTypeLen;
	}
}

void Database::write(std::string key, DataType &data)
{
	int curAddress = startAddress;
	while (true)
	{
		dataIO.seekg(curAddress);
		int curSize;
		dataIO.read(reinterpret_cast<char*>(&curSize), sizeof(curSize));

		if (inCurBlock(key, curSize)) writeInsideBlock(key, dataIO.tellg(), curSize, data.printToString());
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
	dataIO.seekg(curAddress + DataTypeLen - sizeof(int));
	dataIO.write(reinterpret_cast<char*>(&t), sizeof(t));
	//write data to the new block
	writeWholeBlock(t + sizeof(int), data.printToString());
}

void Database::erase(std::string key)
{
	auto data = read(key);
	data.deleted = true;
	write(key, data);
}