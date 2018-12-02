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
	dataIO.open(file);
	dataIO.seekg(0);
	dataIO.read(reinterpret_cast<char*>(&size), sizeof(size));
	if (!dataIO) size = 0, dataIO.write(reinterpret_cast<char*>(&size), sizeof(size));
}

dataSystem::~dataSystem()
{
	dataIO.close();
}

void dataSystem::printToBack(const std::string &str)
{
	auto *t = str.c_str();
	dataIO.write(t, str.length());
	delete[] t;
}

std::string dataSystem::read(int address, int len)
{
	char *t = new char[len];
	dataIO.seekg(address);
	dataIO.write(t, len);
	std::string ret = t;
	delete[] t;
	return ret;
}

void dataSystem::write(int address, const std::string &str)
{
	dataIO.seekg(address);
	dataIO << str;
}
