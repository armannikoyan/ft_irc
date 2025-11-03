#include "Utils.hpp"

std::vector<std::string> split(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    size_t start = 0;
    size_t end = str.find(delimiter);
    
    while (end != std::string::npos)
    {
        tokens.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }
    tokens.push_back(str.substr(start));
    
    return tokens;
}

std::string trim(const std::string& str)
{
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::string toUpper(const std::string& str)
{
    std::string result = str;
    for (size_t i = 0; i < result.length(); i++)
        result[i] = std::toupper(result[i]);
    return result;
}

std::string toLower(const std::string& str)
{
    std::string result = str;
    for (size_t i = 0; i < result.length(); i++)
        result[i] = std::tolower(result[i]);
    return result;
}