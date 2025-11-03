#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>

class Message
{
private:
    std::string _prefix;
    std::string _command;
    std::vector<std::string> _parameters;
    std::string _raw;

public:
    Message();
    Message(const std::string& raw);
    ~Message();
    
    bool parse(const std::string& raw);
    
    std::string getPrefix() const;
    std::string getCommand() const;
    std::vector<std::string> getParameters() const;
    std::string getParameter(size_t index) const;
    size_t getParameterCount() const;
    std::string getRaw() const;
    
    static std::string format(const std::string& prefix, 
                             const std::string& command, 
                             const std::vector<std::string>& params);
};

#endif