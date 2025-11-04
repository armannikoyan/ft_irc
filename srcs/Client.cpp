#include "Client.hpp"
#include <sys/socket.h>
#include <algorithm>

Client::Client(int fd)
    : _fd(fd),
      _authenticated(false),
      _registered(false),
      _disconnected(false)
{
}

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
    size_t skip = 2;
    if (pos == std::string::npos) {
        pos = _buffer.find("\n");
        skip = 1;
    }
    if (pos == std::string::npos)
        return "";

    std::string message = _buffer.substr(0, pos);
    _buffer = _buffer.substr(pos + skip);

    if (!message.empty() && message.back() == '\r')
        message.pop_back();

    return message;
}

void Client::sendMessage(const std::string& msg)
{
    send(_fd, msg.c_str(), msg.length(), 0);
}

const std::string& Client::getHostname() const
{
    return _hostname;
}

void Client::setHostname(const std::string& hostname)
{
    _hostname = hostname;
}

void Client::setDisconnected(bool state)
{
    _disconnected = state;
}

bool Client::isDisconnected() const
{
    return _disconnected;
}

const std::vector<std::string>& Client::getChannels() const
{
    return _channels;
}

void Client::addChannel(const std::string& channel)
{
    _channels.push_back(channel);
}

void Client::removeChannel(const std::string& channel)
{
    _channels.erase(std::remove(_channels.begin(), _channels.end(), channel), _channels.end());
}