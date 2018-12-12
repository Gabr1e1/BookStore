#include "index.h"

std::string formatSubstr(const std::string &str, int start, int len)
{
	std::string ret = str.substr(start, len);
	while (ret.length() != 0 && (ret[ret.length() - 1] == ' ' || ret[ret.length() - 1] == 0)) ret.pop_back();
	return ret;
}

IndexType::IndexType(const std::string &str)
{
	key = formatSubstr(str, 0, StringLen);
	ISBN = formatSubstr(str, StringLen, ISBNLen);
	const char* a = str.c_str();
	corresAddress = *reinterpret_cast<const int*>(a + StringLen + ISBNLen);
}

IndexType::IndexType(const std::string &str, const std::string &_ISBN, int address) 
					: key(str), ISBN(_ISBN), corresAddress(address)
{
	/* Empy */
}
	
std::string IndexType::printToString()
{
	std::ostringstream ret;
	ret.setf(std::ios::left);
	ret << std::setw(StringLen) << key;
	ret << std::setw(ISBNLen) << ISBN;

	std::string retstr = ret.str();
	for (size_t i = 0; i < sizeof(corresAddress); i++)
		retstr += *(reinterpret_cast<char*>(&corresAddress) + i);
	return retstr;
}

IndexDatabase::IndexDatabase(const std::string &file)
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

IndexDatabase::~IndexDatabase()
{
	dataIO.seekg(0, std::ios::beg);
	dataIO.write(reinterpret_cast<char*>(&startAddress), sizeof(startAddress));
	dataIO.close();
}

std::string IndexDatabase::readWholeBlock(int address)
{
	char *t = new char[IndexType::IndexTypeLen + 1];
	for (int i = 0; i < IndexType::IndexTypeLen + 1; i++) t[i] = 0;
	dataIO.seekg(address);
	dataIO.read(t, IndexType::IndexTypeLen);
	std::string ret = "";
	for (int i = 0; i < IndexType::IndexTypeLen; i++) ret += t[i];
	delete[] t;
	return ret;
}

std::string IndexDatabase::readString(int address, int len) // len : maxminum read length
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

int IndexDatabase::readAddress(int address)
{
	char *t = new char[IndexType::NumLen];
	dataIO.seekg(address + IndexType::StringLen + IndexType ::ISBNLen);
	dataIO.read(t, IndexType::NumLen);
	int ret = *(reinterpret_cast<int*>(t));
	delete[] t;
	return ret;
}

void IndexDatabase::writeBlock(int address, const std::string &str)
{
	dataIO.seekg(address);
	dataIO << str;
}

bool IndexDatabase::inCurBlock(const std::string &key, const std::string &uniqueKey, int curSize)
{
	if (curSize == 0) return false;
	int curAddress = (int)dataIO.tellg();
	std::string &&strBegin = readString(curAddress, IndexType::StringLen);
	std::string &&strBegin2 = (uniqueKey != "") ? readString(curAddress + IndexType::StringLen, IndexType::ISBNLen) : "";
	std::string &&strEnd = readString(curAddress + (curSize - 1) * IndexType::IndexTypeLen, IndexType::StringLen);
	std::string &&strEnd2 = (uniqueKey != "") ? readString(curAddress + (curSize - 1) * IndexType::IndexTypeLen + IndexType::StringLen, IndexType::ISBNLen) : "";
	return (make_pair(key, uniqueKey) >= make_pair(strBegin, strBegin2))
		&& (make_pair(key, uniqueKey) <= make_pair(strEnd, strEnd2));
}

std::vector<int> IndexDatabase::readInsideBlock(const std::string &key, int address, int size, const std::string &uniqueKey)
{
	std::vector<int> ret;
	for (int i = 1; i <= size; i++)
	{
		std::string &&cur = readString(address, IndexType::StringLen);
		std::string &&curKey = readString(address + IndexType::StringLen, IndexType::ISBNLen);
		if ((key == "" || cur == key) && (uniqueKey == "" || curKey == uniqueKey)) ret.push_back(readAddress(address));
		address += IndexType::IndexTypeLen;
	}
	return ret;
}

int IndexDatabase::read(const std::string &key, const std::string &uniqueKey)
{
	int curAddress = startAddress;
	while (true)
	{
		int curSize;
		dataIO.seekg(curAddress);
		dataIO.read(reinterpret_cast<char*>(&curSize), sizeof(curSize));

		if (inCurBlock(key, uniqueKey, curSize))
		{
			std::vector<int> r = readInsideBlock(key, curAddress + sizeof(int), curSize, uniqueKey);
			return (r.size() > 0) ? r[0] : 0;
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
	return 0;
}

std::vector<int> IndexDatabase::readAll(const std::string &key)
{
	int curAddress = startAddress;
	std::vector<int> ret = {};
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

int IndexDatabase::createNewBlock(int size, int next)
{
	int ret;
	if (!que.empty())
	{
		dataIO.seekg(que.front());
		ret = que.front();
		que.pop();
	}
	else
	{
		dataIO.seekg(0, std::ios::end);
		ret = (int)dataIO.tellg();

	}
	dataIO.write(reinterpret_cast<char*>(&size), sizeof(size));
	dataIO.seekg(BlockLen - 2 * sizeof(int), std::ios::cur);
	dataIO.write(reinterpret_cast<char*>(&next), sizeof(next));
	return ret;
}

int IndexDatabase::split(int start, int beginEle, int size)
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
	start = start + IndexType::IndexTypeLen * curSize;
	t += sizeof(int);
	for (int i = beginEle; i <= size; i++, t += IndexType::IndexTypeLen, start += IndexType::IndexTypeLen)
	{
		std::string &&cur = readWholeBlock(start);
		writeBlock(t, cur);
	}
	return t2;
}

void IndexDatabase::writeInsideBlock(const std::string &key, int address, int size,
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
		std::string &&curKey = readString(address, IndexType::StringLen);
		std::string &&curUniqueKey = readString(address + IndexType::StringLen, IndexType::ISBNLen);
		if (curKey == key && curUniqueKey == uniqueKey)
		{
			dataIO.seekg(address);
			dataIO << value;
			return;
		}
		//std::cout << curKey << " " << cur << " " << key << " " << uniqueKey << std::endl;
		if (make_pair(curKey, curUniqueKey) < make_pair(key, uniqueKey)) pre = i;
		else break; //break if the current key > key
		address += IndexType::IndexTypeLen;
	}

	if (size + 1 < BlockSize)
	{
		int tAdd = start + IndexType::IndexTypeLen * (size - 1);
		for (int i = size; i >= pre + 1; i--, tAdd -= IndexType::IndexTypeLen)
		{
			writeBlock(tAdd + IndexType::IndexTypeLen, readWholeBlock(tAdd));
		}
		writeBlock(tAdd + IndexType::IndexTypeLen, value);
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
	writeBlock(newBlock + sizeof(int), value);
}

void IndexDatabase::write(const std::string &key, const std::string &uniqueKey, const std::string &data)
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
			writeInsideBlock(key, curAddress + sizeof(int), curSize, uniqueKey, data);
			return;
		}
		else
		{
			if (curSize >= 1)
			{
				std::string &&strEnd = readString(curAddress + sizeof(int) + (curSize - 1) * IndexType::IndexTypeLen, IndexType::StringLen);
				std::string &&strEnd2 = readString(curAddress + sizeof(int) + (curSize - 1) * IndexType::IndexTypeLen + IndexType::StringLen, IndexType::ISBNLen);
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
	writeBlock(t + sizeof(int), data); //write data to the new block
}

void IndexDatabase::eraseInsideBlock(const std::string &key, int address, int size,
	const std::string &uniqueKey)
{
	int pre = 0;
	int start = address;
	for (int i = 1; i <= size; i++)
	{
		std::string &&curKey = readString(address, IndexType::StringLen);
		std::string &&curUniqueKey = readString(address + IndexType::StringLen, IndexType::ISBNLen);
		if (curUniqueKey == uniqueKey && curKey == key)
		{
			int tAdd = address;
			for (int j = i + 1; j <= size; j++, tAdd += IndexType::IndexTypeLen)
			{
				writeBlock(tAdd, readWholeBlock(tAdd + IndexType::IndexTypeLen));
			}

			int newSize = size - 1;
			dataIO.seekg(start - sizeof(int));
			dataIO.write(reinterpret_cast<char*>(&newSize), sizeof(int));
			return;
		}
		address += IndexType::IndexTypeLen;
	}
}

void IndexDatabase::erase(const std::string &key, const std::string &uniqueKey)
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

void IndexDatabase::cleanup()
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
			que.push(next);
			int a1 = curAddress + sizeof(int) + curSize * IndexType::IndexTypeLen;
			int a2 = next + sizeof(int);
			for (int i = 1; i <= nextSize; i++, a1 += IndexType::IndexTypeLen, a2 += IndexType::IndexTypeLen)
			{
				auto &&t = readWholeBlock(a2);
				writeBlock(a1, t);
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
