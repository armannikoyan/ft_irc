#include "Channel.hpp"
#include <algorithm>
#include <sstream>

Channel::Channel(const std::string& name) 
    : _name(name), _inviteOnly(false), _topicRestricted(false), _userLimit(-1) {}

Channel::~Channel() {}

std::string Channel::getName() const { return _name; }
std::string Channel::getTopic() const { return _topic; }
std::string Channel::getKey() const { return _key; }
bool Channel::isInviteOnly() const { return _inviteOnly; }
bool Channel::isTopicRestricted() const { return _topicRestricted; }
int Channel::getUserLimit() const { return _userLimit; }

void Channel::setTopic(const std::string& topic) { _topic = topic; }
void Channel::setKey(const std::string& key) { _key = key; }
void Channel::setInviteOnly(bool value) { _inviteOnly = value; }
void Channel::setTopicRestricted(bool value) { _topicRestricted = value; }
void Channel::setUserLimit(int limit) { _userLimit = limit; }

void Channel::addClient(Client* client)
{
    _clients.push_back(client);
}

void Channel::removeClient(Client* client)
{
    _clients.erase(std::remove(_clients.begin(), _clients.end(), client), _clients.end());
    _operators.erase(std::remove(_operators.begin(), _operators.end(), client), _operators.end());
}

void Channel::addOperator(Client* client)
{
    if (std::find(_operators.begin(), _operators.end(), client) == _operators.end())
        _operators.push_back(client);
}

void Channel::removeOperator(Client* client)
{
    _operators.erase(std::remove(_operators.begin(), _operators.end(), client), _operators.end());
}

void Channel::addInvited(Client* client)
{
    if (std::find(_invited.begin(), _invited.end(), client) == _invited.end())
        _invited.push_back(client);
}

bool Channel::isOperator(Client* client) const
{
    return std::find(_operators.begin(), _operators.end(), client) != _operators.end();
}

bool Channel::isInvited(Client* client) const
{
    return std::find(_invited.begin(), _invited.end(), client) != _invited.end();
}

bool Channel::isMember(Client* client) const
{
    return std::find(_clients.begin(), _clients.end(), client) != _clients.end();
}

bool Channel::isFull() const
{
    if (_userLimit == -1)
        return false;
    return static_cast<int>(_clients.size()) >= _userLimit;
}

std::vector<Client*> Channel::getClients() const
{
    return _clients;
}

void Channel::broadcast(const std::string& message, Client* exclude)
{
    for (size_t i = 0; i < _clients.size(); i++)
    {
        if (_clients[i] != exclude)
        {
            _clients[i]->sendMessage(message);
        }
    }
}

bool Channel::hasKey() const
{
    return !_key.empty();
}

bool Channel::hasUserLimit() const
{
    return _userLimit != -1;
}

void Channel::removeKey()
{
    _key = "";
}

void Channel::removeUserLimit()
{
    _userLimit = -1;
}

bool Channel::hasClient(Client* client) const
{
    return isMember(client);
}

size_t Channel::getUserCount() const
{
    return _clients.size();
}

bool Channel::isEmpty() const
{
    return _clients.empty();
}

void Channel::addInvite(Client* client)
{
    addInvited(client);
}

std::string Channel::getModes() const
{
    std::string modes = "";
    if (_inviteOnly) modes += "i";
    if (_topicRestricted) modes += "t";
    if (!_key.empty()) modes += "k";
    if (_userLimit != -1) modes += "l";
    return modes;
}

std::string Channel::getNamesList() const
{
    std::ostringstream oss;
    for (size_t i = 0; i < _clients.size(); i++)
    {
        if (isOperator(_clients[i]))
            oss << "@";
        oss << _clients[i]->getNickname();
        if (i < _clients.size() - 1)
            oss << " ";
    }
    return oss.str();
}