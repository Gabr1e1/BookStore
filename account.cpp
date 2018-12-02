#include "account.h"

User::User(const int _level, const std::string &_id,
	const std::string &_name, const std::string &_password) :
	level(_level), userId(_id), name(_name), password(_password), deleted(false)
{
	/* Empty */
}

std::string User::printToString()
{
	char *t = new char[UserLen];
	sprintf(t, "%-1d", level);
	sprintf(t + NumLen, "%-30s", userId);
	sprintf(t + NumLen + StringLen, "%-30s", name);
	sprintf(t + NumLen + 2 * StringLen, "%-30s", password);
	sprintf(t + NumLen + 3 * StringLen, "%-1d", deleted);
	std::string ret = t;
	delete[] t;
	return ret;
}

AccountSystem::AccountSystem(const std::string &file) : dataSystem(file), curLevel(0)
{
	dataIO.open(file, std::ios::out | std::ios::app);
	if (size == 0)
	{
		User root(7, "root", "root", "sjtu");
		printToBack(root.printToString());
	}
}

AccountSystem::~AccountSystem()
{
	dataIO.seekg(std::ios::beg);
	dataIO.write(reinterpret_cast<char*>(&size), sizeof(size));
}

std::string AccountSystem::readUserId(int address)
{
	return read(address + NumLen, StringLen);
}

std::string AccountSystem::readPassword(int address)
{
	return read(address + NumLen + 2 * StringLen, StringLen);
}

int AccountSystem::readLevel(int address)
{
	return read(address, NumLen)[0] - '0';
}

void AccountSystem::add(int level, const std::string &userId,
	const std::string &password, const std::string &name)
{
	if (!(curLevel >= 3)) throw("Invalid");
	if (level >= curLevel) throw("Invalid");
	User cur(level, userId, name, password);
	printToBack(cur.printToString());
	size++;
}

void AccountSystem::erase(const std::string &userId)
{
	if (!(curLevel >= 7)) throw("Invalid");
	int curAddress = sizeof(int);
	for (int i = 1; i <= size; i++)
	{
		std::string curUserId = readUserId(curAddress);
		if (curUserId == userId)
		{
			dataIO.seekg(curAddress + UserLen - 1);
			dataIO << '0';
			return;
		}
		curAddress += UserLen;
	}
	throw("Invalid");
}

void AccountSystem::changePassword(const std::string &userId, const std::string &newPassword,
	const std::string &oldPassword = "")
{
	if (!(curLevel >= 1)) throw("Invalid");
	int curAddress = 0;
	dataIO.seekg(curAddress);
	for (int i = 1; i <= size; i++)
	{
		std::string curUserId = readUserId(curAddress);
		if (curUserId == userId)
		{
			std::string curPassword = readPassword(curAddress);
			if (curPassword == oldPassword || curLevel == 7)
			{
				write(curAddress + NumLen + 2 * StringLen, newPassword);
				return;
			}
			else throw("Inavlid");
		}
		curAddress += UserLen;
	}
	throw("Invalid");
}

void AccountSystem::login(const std::string &userId, const std::string &password)
{
	int curAddress = 0;
	dataIO.seekg(curAddress);
	for (int i = 1; i <= size; i++)
	{
		std::string curUserId = readUserId(curAddress);
		if (curUserId == userId)
		{
			std::string curPassword = readPassword(curAddress);
			int level = readLevel(curAddress);
			if (curPassword == password || curLevel > level)
			{
				curUserId = userId;
				curLevel = level;
				return;
			}
			else throw("Inavlid");
		}
	}
	throw("Invalid");
}

void AccountSystem::logout()
{
	curLevel = 0;
}

