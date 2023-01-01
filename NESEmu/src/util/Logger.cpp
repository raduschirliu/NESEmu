#include "Logger.h"

#include <stdio.h>

#include <sstream>
#include <vector>

Logger::Logger(std::string path) : path_(path)
{
    file_.open(path_, std::fstream::out);
}

void Logger::Write(std::string text)
{
    file_ << text;
}

void Logger::Write(const char *text)
{
    file_ << text;
}

Logger::~Logger()
{
    if (file_.is_open())
    {
        file_.close();
    }
}