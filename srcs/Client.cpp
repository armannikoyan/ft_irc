#include "Client.hpp"

Client::Client(int fd) : _fd(fd), _authenticated(false), _registered(false) {}

Client::~Client() {}

int Client::getFd() const { return _fd; }
std::string Client::getNickname() const { return _nickname; }
std::string Client::getUsername() const { return _username; }
bool Client::isAuthenticated() const { return _authenticated; }
bool Client::isRegistered() const { return _registered; }

void Client::setNickname(const std::string& nickname) { _nickname = nickname; }
void Client::setUsername(const std::string& username) { _username = username; }
void Client::setRealname(const std::string& realname) { _realname = realname; }
void Client::setAuthenticated(bool auth) { _authenticated = auth; }
void Client::setRegistered(bool reg) { _registered = reg; }

void Client::appendBuffer(const std::string& data)
{
    _buffer += data;
}

std::string Client::extractMessage()
{
    size_t pos = _buffer.find("\r\n");
    if (pos == std::string::npos)
        return "";
    
    std::string message = _buffer.substr(0, pos);
    _buffer = _buffer.substr(pos + 2);
    return message;
}