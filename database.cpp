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
	price = stringToInteger(str.substr(ISBNLen + StringLen * 2, NumLen));
	quantity = stringToInteger(str.substr(ISBNLen + StringLen * 2 + NumLen, NumLen));
}

std::string DataType::printToString()
{
	char s[ISBNLen + StringLen * 2 + NumLen * 2];
	sprintf(s,"%-20s", ISBN);
	sprintf(s + ISBNLen, "%-40s", name);
	sprintf(s + ISBNLen + StringLen, "%-40s", author);
	sprintf(s + ISBNLen + 2 * StringLen, "%-5d", price);
	sprintf(s + ISBNLen + 2 * StringLen + NumLen, "%-5d", quantity);
	std::string ret = s;
	return ret;
}

Database::Database(const std::string &file, int _offset, int len) : offset(_offset), readLen(len)
{
	dataIO.open(file, std::ios::app|std::ios::out);
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

void Database::writeInsideBlock(std::string &key, int address, int size)
{
	int pre = 0;
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
		address += DataTypeLen;
	}
	createNewNode()
}

void Database::write(std::string &key, DataType &data)
{
	int curAddress = startAddress;
	while (true)
	{
		dataIO.seekg(curAddress);
		int curSize;
		dataIO.read(reinterpret_cast<char*>(&curSize), sizeof(curSize));

		if (inCurBlock(key, curSize)) writeInsideBlock(key, dataIO.tellg(), curSize);
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
	//new block at the end
	dataIO.seekg(curAddress);
	createNewNode(curAddress, -1, key, data);
}
