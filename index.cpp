#include "index.h"

std::string formatSubstr(const std::string &str, int start, int len)
{
	std::string ret = str.substr(start, len);
	while (ret.length() != 0 && (ret[ret.length() - 1] == ' ' || ret[ret.length() - 1] == 0)) ret.pop_back();
	return ret;
}

ull getHash(const std::string &str)
{
	ull ret = 0;
	for (int i = 0; i < str.length(); i++) ret = ret * B + str[i];
	return ret;
}

IndexType::IndexType(const std::string &str)
{
	const char* a = str.c_str();
	key = *reinterpret_cast<const ull*>(a);
	ISBN = *reinterpret_cast<const ull*>(a + StringLen);
	corresAddress = *reinterpret_cast<const int*>(a + StringLen + ISBNLen);
}

IndexType::IndexType(const std::string &str, const std::string &_ISBN, int address) 
					: key(getHash(str)), ISBN(getHash(_ISBN)), corresAddress(address)
{
	/* Empy */
}
	
std::string IndexType::printToString()
{
	std::string retstr = "";
	for (size_t i = 0; i < sizeof(key); i++)
		retstr += *(reinterpret_cast<char*>(&key) + i);
	for (size_t i = 0; i < sizeof(ISBN); i++)
		retstr += *(reinterpret_cast<char*>(&ISBN) + i);
	for (size_t i = 0; i < sizeof(corresAddress); i++)
		retstr += *(reinterpret_cast<char*>(&corresAddress) + i);
	return retstr;
}

IndexDatabase::IndexDatabase(const std::string &file, MainDatabase *_db) : maindb(_db)
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

std::string IndexDatabase::readWholeBlock(int address, bool reSeek)
{
	char *t = new char[IndexType::IndexTypeLen];
	if (reSeek) dataIO.seekg(address);
	dataIO.read(t, IndexType::IndexTypeLen);
	std::string ret = "";
	for (int i = 0; i < IndexType::IndexTypeLen; i++) ret += t[i];
	delete[] t;
	return ret;
}

template<typename T>
T IndexDatabase::readNum(int address, bool reSeek)
{
	T ret;
	if (reSeek) dataIO.seekg(address);
	dataIO.read(reinterpret_cast<char*>(&T), sizeof(T));
	return ret;
}

int IndexDatabase::readAddress(int address, bool reSeek)
{
	return readNum<int>(address + IndexType::StringLen + IndexType::ISBNLen);
}

ull IndexDatabase::readISBN(int address, bool reSeek)
{
	return readNum<ull>(address);
}

ull IndexDatabase::readKey(int address, bool reSeek)
{
	return readNum<ull>(address);
}

void IndexDatabase::writeBlock(int address, const std::string &str, bool reSeek)
{
	if (reSeek) dataIO.seekg(address);
	dataIO << str;
}

bool IndexDatabase::inCurBlock(ull key, ull uniqueKey, int curSize)
{
	if (curSize == 0) return false;
	int curAddress = (int)dataIO.tellg();
	ull strBegin = readKey(curAddress);
	ull strBegin2 = (uniqueKey != 0) ? readISBN(curAddress + IndexType::StringLen, false) : 0;
	ull strEnd = readKey(curAddress + (curSize - 1) * IndexType::IndexTypeLen);
	ull strEnd2 = (uniqueKey != 0) ? readISBN(curAddress + (curSize - 1) * IndexType::IndexTypeLen + IndexType::StringLen, false) : 0;
	return (std::make_pair(key, uniqueKey) >= std::make_pair(strBegin, strBegin2))
		&& (std::make_pair(key, uniqueKey) <= std::make_pair(strEnd, strEnd2));
}

std::vector<int> IndexDatabase::readInsideBlock(ull key, int address, int size, ull uniqueKey, const std::string &uniqueStr)
{
	std::vector<int> ret;
	for (int i = 1; i <= size; i++)
	{
		ull cur = readKey(address);
		auto curKey = readISBN(address + IndexType::StringLen, false);
		if ((key == 0 || cur == key) && (uniqueKey == 0 || (curKey == uniqueKey && maindb->read(readAddress(address)).ISBN == uniqueStr))) ret.push_back(readAddress(address));
		address += IndexType::IndexTypeLen;
	}
	return ret;
}

int IndexDatabase::read(const std::string &_key, const std::string &_uniqueKey)
{
	ull key = getHash(_key);
	ull uniqueKey = getHash(_uniqueKey);
	int curAddress = startAddress;
	while (true)
	{
		int curSize;
		dataIO.seekg(curAddress);
		dataIO.read(reinterpret_cast<char*>(&curSize), sizeof(curSize));

		if (inCurBlock(key, uniqueKey, curSize))
		{
			std::vector<int> r = readInsideBlock(key, curAddress + sizeof(int), curSize, uniqueKey, _uniqueKey);
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

std::vector<int> IndexDatabase::readAll(const std::string &_key)
{
	ull key = getHash(_key);
	int curAddress = startAddress;
	std::vector<int> ret = {};
	bool flg = false;
	while (true)
	{
		dataIO.seekg(curAddress);
		int curSize;
		dataIO.read(reinterpret_cast<char*>(&curSize), sizeof(curSize));

		if (inCurBlock(key, 0, curSize) || key == 0)
		{
			flg = true;
			auto t = readInsideBlock(key, curAddress + sizeof(int), curSize);
			ret.insert(ret.end(), t.begin(), t.end());
		}
		else
		{
			if (flg && key != 0 && curSize != 0) break;
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

void IndexDatabase::writeInsideBlock(ull key, int address, int size,
	ull uniqueKey, const std::string &value, const std::string &uniqueStr)
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
		ull curKey = readKey(address);
		ull curUniqueKey = readISBN(address + IndexType::StringLen, false);
		if (curKey == key && curUniqueKey == uniqueKey && maindb->read(readAddress(address)).ISBN == uniqueStr)
		{
			dataIO.seekg(address);
			dataIO << value;
			return;
		}
		//std::cout << curKey << " " << cur << " " << key << " " << uniqueKey << std::endl;
		if (std::make_pair(curKey, curUniqueKey) < std::make_pair(key, uniqueKey)) pre = i;
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

void IndexDatabase::write(const std::string &_key, const std::string &_uniqueKey, const std::string &data)
{
	ull key = getHash(_key);
	ull uniqueKey = getHash(_uniqueKey);
	int curAddress = startAddress;
	int pre = -1;

	while (true)
	{
		dataIO.seekg(curAddress);
		int curSize;
		dataIO.read(reinterpret_cast<char*>(&curSize), sizeof(curSize));

		if (inCurBlock(key, uniqueKey, curSize))
		{
			writeInsideBlock(key, curAddress + sizeof(int), curSize, uniqueKey, data, _uniqueKey);
			return;
		}
		else
		{
			if (curSize >= 1)
			{
				ull strEnd = readKey(curAddress + sizeof(int) + (curSize - 1) * IndexType::IndexTypeLen);
				ull strEnd2 = readISBN(curAddress + sizeof(int) + (curSize - 1) * IndexType::IndexTypeLen + IndexType::StringLen, false);
				if (std::make_pair(strEnd, strEnd2) < std::make_pair(key, uniqueKey)) pre = curAddress;
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

void IndexDatabase::eraseInsideBlock(ull key, int address, int size, ull uniqueKey, const std::string &uniqueStr)
{
	int pre = 0;
	int start = address;
	for (int i = 1; i <= size; i++)
	{
		ull curKey = readKey(address);
		auto curUniqueKey = readISBN(address + IndexType::StringLen, false);
		if (curUniqueKey == uniqueKey && curKey == key && maindb->read(readAddress(address)).ISBN == uniqueStr)
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

void IndexDatabase::erase(const std::string &_key, const std::string &_uniqueKey)
{
	ull key = getHash(_key);
	ull uniqueKey = getHash(_uniqueKey);
	int curAddress = startAddress;
	while (true)
	{
		dataIO.seekg(curAddress);
		int curSize;
		dataIO.read(reinterpret_cast<char*>(&curSize), sizeof(curSize));

		if (inCurBlock(key, uniqueKey, curSize))
		{
			eraseInsideBlock(key, curAddress + sizeof(int), curSize, uniqueKey, _uniqueKey);
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
