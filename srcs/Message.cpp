#include "Message.hpp"
#include "Utils.hpp"

Message::Message() {}

Message::Message(const std::string& raw) : _raw(raw)
{
    parse(raw);
}

Message::~Message() {}

bool Message::parse(const std::string& raw)
{
    _raw = raw;
    std::string line = trim(raw);
    
    if (line.empty())
        return false;
    
    size_t pos = 0;
    
    if (line[0] == ':')
    {
        size_t space = line.find(' ');
        if (space == std::string::npos)
            return false;
        _prefix = line.substr(1, space - 1);
        pos = space + 1;
    }
    
    size_t space = line.find(' ', pos);
    if (space == std::string::npos)
    {
        _command = toUpper(line.substr(pos));
        return true;
    }
    
    _command = toUpper(line.substr(pos, space - pos));
    pos = space + 1;
    
    while (pos < line.length())
    {
        if (line[pos] == ':')
        {
            _parameters.push_back(line.substr(pos + 1));
            break;
        }
        
        space = line.find(' ', pos);
        if (space == std::string::npos)
        {
            _parameters.push_back(line.substr(pos));
            break;
        }
        
        _parameters.push_back(line.substr(pos, space - pos));
        pos = space + 1;
    }
    
    return true;
}

std::string Message::getPrefix() const { return _prefix; }
std::string Message::getCommand() const { return _command; }
std::vector<std::string> Message::getParameters() const { return _parameters; }

std::string Message::getParameter(size_t index) const
{
    if (index < _parameters.size())
        return _parameters[index];
    return "";
}

size_t Message::getParameterCount() const
{
    return _parameters.size();
}

std::string Message::getRaw() const { return _raw; }

std::string Message::format(const std::string& prefix, 
                           const std::string& command, 
                           const std::vector<std::string>& params)
{
    std::string result;
    
    if (!prefix.empty())
        result = ":" + prefix + " ";
    
    result += command;
    
    for (size_t i = 0; i < params.size(); i++)
    {
        result += " ";
        if (i == params.size() - 1 && params[i].find(' ') != std::string::npos)
            result += ":";
        result += params[i];
    }
    
    result += "\r\n";
    return result;
}