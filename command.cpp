#include "command.h"

CommandSystem::CommandSystem(const std::string &file)
{
	dataIO.open(file, std::ios::out | std::ios::app);
	ISBNDatabase = new Database("ISBNDatabase.txt", 0, ISBNLen);
	nameDatabase = new Database("nameDatabase.txt", ISBNLen, StringLen);
	authorDatabase = new Database("authorDatabase.txt", ISBNLen + StringLen, StringLen);
	keywordDatabase = new Database("keywordDatabase.txt", ISBNLen + StringLen * 2, StringLen);
}

CommandSystem::~CommandSystem()
{
	dataIO.close();
	if (Account != nullptr) delete Account;
	if (ISBNDatabase != nullptr) delete ISBNDatabase;
	if (nameDatabase != nullptr) delete nameDatabase;
	if (authorDatabase != nullptr) delete authorDatabase;
	if (keywordDatabase != nullptr) delete keywordDatabase;
}

std::vector<std::string> CommandSystem::parse(std::string str)
{
	std::vector<std::string> ret;
	int p = str.find(" ");
	while (p != std::string::npos)
	{
		ret.push_back(str.substr(0, p));
		str = str.substr(p + 1, str.length() - 1 - p);
	}
	ret.push_back(str);
	return ret;
}

void CommandSystem::modify(DataType &data)
{
	ISBNDatabase->write(data.ISBN, data);
	nameDatabase->write(data.name, data);
	authorDatabase->write(data.author, data);
	while (data.keyword.find("|") != std::string::npos)
	{

	}
}

ResultType CommandSystem::runCommand(const std::string &str)
{
	auto token = parse(str);

	if (token[0] == "exit") return Exit;
	if (token[0] == "load") return runLoadCommand(token[1]);

	if (token[0] == "su") Account->login(token[1], token[2]);
	else if (token[0] == "logout") Account->logout();
	else if (token[0] == "useradd")
	{
		Account->add(token[3][0] - '0', token[1], token[2], token[4]);
	}
	else if (token[0] == "register")
	{
		Account->add(1, token[1], token[2], token[3]);
	}
	else if (token[0] == "delete") Account->erase(token[1]);
	else if (token[0] == "passwd")
	{
		if (token.size() == 3) Account->changePassword(token[1], token[2]);
		else Account->changePassword(token[1], token[3], token[2]);
	}

	if (token[0] == "select")
	{
		curSelected.clear();
		curSelected.push_back(ISBNDatabase->read(token[1]));
	}
	else if (token[0] == "modify")
	{
		if (curSelected.size() != 1) throw("Invalid");
		DataType t = curSelected[0];
		for (size_t i = 1; i < token.size(); i++)
		{
			if (token[i].substr(0, 5) == "-ISBN")
			{
				t.ISBN = token[i].substr(6, token[i].length() - 6);
			}
			else if (token[i].substr(0, 5) == "-name")
			{
				t.name = token[i].substr(7, token[i].length() - 2 - 7 + 1);
			}
			else if (token[i].substr(0, 7) == "-author")
			{
				t.author = token[i].substr(9, token[i].length() - 2 - 9 + 1);
			}
			else if (token[i].substr(0, 8) == "-keyword")
			{
				t.keyword = token[i].substr(10, token[i].length() - 2 - 10 + 1);
			}
			else if (token[i].substr(0, 6) == "-price")
			{
				t.price = stringToInteger(token[i].substr(7, token[i].length() - 7));
			}
		}
		modify(t);
	}
	else if (true)
	{

	}
	return Executed;
}

ResultType CommandSystem::runLoadCommand(const std::string &file)
{

}