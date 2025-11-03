#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

std::vector<std::string> split(const std::string& str, char delimiter);
std::string trim(const std::string& str);
std::string toUpper(const std::string& str);
std::string toLower(const std::string& str);

#endif