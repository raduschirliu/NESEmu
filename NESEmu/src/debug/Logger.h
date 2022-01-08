#pragma once

#include <string>
#include <fstream>

class Logger
{
public:
	// Create the logger and open the file
	Logger(std::string path);

	// Close the file
	~Logger();

	// Write to the file
	void write(std::string text);
	void write(const char *text);

private:
	std::string path;
	std::ofstream file;
};