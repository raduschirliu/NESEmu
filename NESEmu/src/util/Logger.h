#pragma once

#include <fstream>
#include <string>

class Logger
{
  public:
    // Create the logger and open the file
    Logger(std::string path);

    // Close the file
    ~Logger();

    // Write to the file
    void Write(std::string text);
    void Write(const char *text);

  private:
    std::string path_;
    std::ofstream file_;
};