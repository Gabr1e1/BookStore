#ifndef DataSystem_H
#define DataSystem_H

//base class (abstract class) for any dataSystem

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>

int stringToInteger(const std::string &str);
double stringToDouble(const std::string &str);

class dataSystem
{
protected:
	std::fstream dataIO;
	int size;

protected:
	dataSystem(const std::string &file);
	~dataSystem();

public:
	int printToBack(const std::string &str);
	std::string read(int address, int len);
	std::string readAll(int address, int len);
	void write(int address, const std::string &str);
};

#endif
