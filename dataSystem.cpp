#include "dataSystem.h"

int stringToInteger(const std::string &str)
{
	std::istringstream stream(str);
	int value;
	stream >> value;
	return value;
}

double stringToDouble(const std::string &str)
{
	std::istringstream stream(str);
	double value;
	stream >> value;
	return value;
}

dataSystem::dataSystem(const std::string &file)
{
	std::ofstream tmp(file/*, std::ios::app*/); tmp.close();
	dataIO.open(file, std::ios::binary | std::ios::out | std::ios::in);
	dataIO.seekg(0, std::ios::end);
	
	if (!dataIO.gcount())
	{
		size = 0;
		dataIO.write(reinterpret_cast<const char*>(&size), sizeof(size));
		if (!dataIO) throw std::exception("Create file failed");
	}
	else
	{
		dataIO.seekg(0, std::ios::beg);
		dataIO.read(reinterpret_cast<char*>(&size), sizeof(size));
	}
}

dataSystem::~dataSystem()
{
	dataIO.close();
}

void dataSystem::printToBack(const std::string &str)
{
	dataIO.seekg(0, std::ios::end);
	auto *t = str.c_str();
	dataIO.write(t, str.length());
	if (!dataIO) throw std::exception("Write Failed");
}

std::string dataSystem::read(int address, int len)
{
	char *t = new char[len + 1];
	for (int i = 0; i < len + 1; i++) t[i] = 0;
	dataIO.seekg(address);
	dataIO.read(t, len);
	std::string ret = t;
	while (ret.length() != 0 && (ret[ret.length() - 1] == ' ' || ret[ret.length() - 1] == 0))
		ret = ret.substr(0, ret.length() - 1);
	delete[] t;
	return ret;
}

void dataSystem::write(int address, const std::string &str)
{
	dataIO.seekg(address);
	dataIO << str;
}
