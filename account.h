#ifndef Account_H
#define Account_H

#include <iostream>
#include <fstream>
#include <string>

static const int NumLen = 1;
static const int StringLen = 30;
static const int UserLen = NumLen * 2 + 3 * StringLen;

class User
{
private:
	int level;
	std::string userId; //aligned to 30
	std::string name; //aligned to 30
	std::string password; //algiend to 30
	bool deleted;

public:
	User(int _level, const std::string &_id,
		const std::string &_name, const std::string &_password);

public:
	std::string printToString();
};

class AccountSystem
{
private:
	std::fstream dataIO;
	int curLevel;
	std::string curUserId;
	int size;

public:
	AccountSystem(const std::string &file);
	~AccountSystem();

private:
	void printToBack(const std::string &str);
	std::string read(int address, int len);
	void write(int address, const std::string &str);

private:
	int readLevel(int address);
	std::string readUserId(int address);
	std::string readPassword(int address);

public:
	void add(int level, const std::string &userId,
		const std::string &password, const std::string &name);
	void erase(const std::string &userId);
	void changePassword(const std::string &userId, const std::string &newPassword,
		const std::string &oldPassword = "");
	void login(const std::string &userId, const std::string &password = "");
	void logout();
};

#endif Account_H