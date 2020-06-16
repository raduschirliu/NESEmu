#include "Logger.h"

Logger::Logger(std::string path)
{
	this->path = path;
	file.open(path);
}

void Logger::write(std::string text)
{
	file << text;
}

Logger::~Logger()
{
	if (file.is_open())
	{
		file.close();
	}
}