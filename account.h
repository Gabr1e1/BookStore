#ifndef Account_H
#define Account_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>

#include "dataSystem.h"

class User
{
public:
	static const int NumLen = 1;
	static const int StringLen = 30;
	static const int UserLen = NumLen * 2 + 3 * StringLen;

private:
	int level;
	std::string userId; //aligned to 30
	std::string name; //aligned to 30
	std::string password; //algiend to 30
	bool deleted;

public:
	User(int _level, const std::string &_id,
		const std::string &_name, const std::string &_password);
	~User() = default;

public:
	std::string printToString();
};

class AccountSystem : public dataSystem
{
public:
	int curLevel;
	std::string curUserId;

public:
	AccountSystem(const std::string &file);
	~AccountSystem();

private:
	int readLevel(int address);
	std::string readUserId(int address);
	std::string readPassword(int address);
	bool readDeleted(int address);
	bool exist(const std::string &userId);

public:
	void add(int level, const std::string &userId,
		const std::string &password, const std::string &name);
	void addRegister(const std::string &userId,
		const std::string &password, const std::string &name);
	void erase(const std::string &userId);
	void changePassword(const std::string &userId, const std::string &newPassword,
		const std::string &oldPassword = "");
	void login(const std::string &userId, const std::string &password = "");
	void logout();
};

#endif
