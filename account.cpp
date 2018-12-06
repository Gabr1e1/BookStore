#include "account.h"

User::User(const int _level, const std::string &_id,
	const std::string &_name, const std::string &_password) :
	level(_level), userId(_id), name(_name), password(_password), deleted(false)
{
	/* Empty */
}

std::string User::printToString()
{
	std::ostringstream ret;
	ret.setf(std::ios::left);
	ret << std::setw(User::NumLen) << level;
	ret << std::setw(User::StringLen) << userId;
	ret << std::setw(User::StringLen) << name;
	ret << std::setw(User::StringLen) << password;
	return ret.str() + (char)(deleted + '0');
}

AccountSystem::AccountSystem(const std::string &file) : dataSystem(file)
{
	if (size == 0)
	{
		User root(7, "root", "root", "sjtu");
		printToBack(root.printToString());
		size = 1;
	}
	curLevel = 7;
	curUserId = "root";
}

AccountSystem::~AccountSystem()
{
	dataIO.seekg(std::ios::beg);
	dataIO.write(reinterpret_cast<char*>(&size), sizeof(size));
}

std::string AccountSystem::readUserId(int address)
{
	return read(address + User::NumLen, User::StringLen);
}

std::string AccountSystem::readPassword(int address)
{
	return read(address + User::NumLen + 2 * User::StringLen, User::StringLen);
}

int AccountSystem::readLevel(int address)
{
	return read(address, User::NumLen)[0] - '0';
}

void AccountSystem::add(int level, const std::string &userId,
	const std::string &password, const std::string &name)
{
	if (!(curLevel >= 3)) throw std::exception("Invalid");
	if (level >= curLevel) throw std::exception("Invalid");
	User cur(level, userId, name, password);
	printToBack(cur.printToString());
	size++;
}

void AccountSystem::addRegister(const std::string &userId,
	const std::string &password, const std::string &name)
{
	if (!(curLevel >= 0)) throw std::exception("Invalid");
	User cur(1, userId, name, password);
	printToBack(cur.printToString());
	size++;
}

void AccountSystem::erase(const std::string &userId)
{
	if (!(curLevel >= 7)) throw std::exception("Invalid");
	int curAddress = sizeof(int);
	for (int i = 1; i <= size; i++)
	{
		std::string curUserId = readUserId(curAddress);
		if (curUserId == userId)
		{
			dataIO.seekg(curAddress + User::UserLen - 1);
			dataIO << '0';
			return;
		}
		curAddress += User::UserLen;
	}
	throw std::exception("Invalid");
}

void AccountSystem::changePassword(const std::string &userId, const std::string &newPassword,
	const std::string &oldPassword)
{
	if (!(curLevel >= 1)) throw std::exception("Invalid");
	int curAddress = sizeof(int);
	for (int i = 1; i <= size; i++)
	{
		std::string curUserId = readUserId(curAddress);
		if (curUserId == userId)
		{
			std::string curPassword = readPassword(curAddress);
			if (curPassword == oldPassword || curLevel == 7)
			{
				std::ostringstream ret; ret.setf(std::ios::left);
				ret << std::setw(User::StringLen) << newPassword;
				write(curAddress + User::NumLen + 2 * User::StringLen, ret.str());
				return;
			}
			else throw std::exception("Invalid");
		}
		curAddress += User::UserLen;
	}
	throw std::exception("Invalid");
}

void AccountSystem::login(const std::string &userId, const std::string &password)
{
	int curAddress = sizeof(int);
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
			else throw std::exception("Invalid");
		}
		curAddress += User::UserLen;
	}
	throw std::exception("Invalid");
}

void AccountSystem::logout()
{
	curLevel = 0;
	curUserId = "";
}

