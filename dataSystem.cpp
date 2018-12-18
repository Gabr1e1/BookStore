#include "dataSystem.h"

int stringToInteger(const std::string &str)
{
	std::istringstream stream(str);
	int value;
	stream >> value;
	if (stream.fail()) throw std::logic_error("Invalid");
	return value;
}

double stringToDouble(const std::string &str)
{
	std::istringstream stream(str);
	double value;
	stream >> value;
	if (stream.fail()) throw std::logic_error("Invalid");
	return value;
}

dataSystem::dataSystem(const std::string &file)
{
	std::ofstream tmp(file, std::ios::app); tmp.close();
	dataIO.open(file, std::ios::binary | std::ios::out | std::ios::in);
	dataIO.seekg(0, std::ios::end);

	if (!dataIO.tellg())
	{
		size = 0;
		dataIO.write(reinterpret_cast<const char*>(&size), sizeof(size));
		if (!dataIO) throw std::logic_error("Create file failed");
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

int dataSystem::printToBack(const std::string &str)
{
	dataIO.seekg(0, std::ios::end);
	auto *t = str.c_str();
	dataIO.write(t, str.length());
	if (!dataIO) throw std::logic_error("Write Failed");
	return (int)dataIO.tellg() - (int)str.length();
}

std::string dataSystem::read(int address, int len)
{
	char *t = new char[len + 1];
	for (int i = 0; i < len + 1; i++) t[i] = 0;
	dataIO.seekg(address);
	dataIO.read(t, len);
	std::string ret = t;
	while (ret.length() != 0 && (ret[ret.length() - 1] == ' ' || ret[ret.length() - 1] == 0)) ret.pop_back();
	delete[] t;
	return ret;
}

std::string dataSystem::readAll(int address, int len)
{
	char *t = new char[len];
	dataIO.seekg(address);
	dataIO.read(t, len);
	std::string ret = "";
	for (int i = 0; i < len; i++) ret += t[i];
	delete[] t;
	return ret;
}

void dataSystem::write(int address, const std::string &str)
{
	dataIO.seekg(address);
	dataIO << str;
}
