#include "Logger.h"

#include <stdio.h>
#include <vector>
#include <sstream>

// TODO: Make buffer configurable
static int constexpr BUFFER_SIZE = 0;
static std::stringstream ss;
static int bufferCount = 0;

Logger::Logger(std::string path)
{
	this->path = path;
	file.open(path, std::fstream::out);
}

void Logger::write(std::string text)
{
	write(text.c_str());
}

void Logger::write(const char *text)
{
	ss << text;
	bufferCount++;

	if (bufferCount > BUFFER_SIZE)
	{
		file << ss.str();
		ss.str("");
		bufferCount = 0;
	}
}

Logger::~Logger()
{
	if (file.is_open())
	{
		// Write any last text
		if (bufferCount > 0)
		{
			file << ss.str();
		}

		file.close();
	}
}