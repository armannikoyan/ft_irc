#include <algorithm>

#include "Channel.hpp"

Channel::Channel(const std::string &name) :
    _name(name), _isInviteOnly(false), _isTopicRestricted(true), _clientLimit(0) {}

Channel::~Channel() {
  _clients.clear();
  _operators.clear();
  _joinOrder.clear();
  _invitedUsers.clear();
}

const std::string &Channel::getName() const { return _name; }

const std::string &Channel::getTopic() const { return _topic; }

void Channel::setTopic(const std::string &topic) { _topic = topic; }

const std::map<int, Client *> &Channel::getClients() const { return _clients; }

void Channel::addClient(Client *client) {
  _clients[client->getFd()] = client;
  _joinOrder.push_back(client->getFd());
}

void Channel::removeClient(const int fd) {
  _clients.erase(fd);
  _operators.erase(fd);
  _joinOrder.remove(fd);
}

bool Channel::hasClient(const int fd) const { return _clients.find(fd) != _clients.end(); }

void Channel::addOperator(Client *client) { _operators[client->getFd()] = client; }

void Channel::removeOperator(const int fd) { _operators.erase(fd); }

bool Channel::isOperator(const int fd) const { return _operators.find(fd) != _operators.end(); }

std::string Channel::getClientList() const {
  std::string list;

  for (std::map<int, Client *>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
    if (isOperator(it->first)) {
      list += "@";
    }
    list += it->second->getNickname() + " ";
  }

  if (!list.empty())
    list.erase(list.size() - 1);

  return list;
}

Client *Channel::getOldestClient() const {
  if (_joinOrder.empty())
    return NULL;

  const int oldestFd = _joinOrder.front();

  std::map<int, Client *>::const_iterator it = _clients.find(oldestFd);
  if (it != _clients.end()) {
    return it->second;
  }

  return NULL;
}

bool Channel::isInviteOnly() const { return _isInviteOnly; }
void Channel::setInviteOnly(const bool inviteOnly) { _isInviteOnly = inviteOnly; }

bool Channel::isTopicRestricted() const { return _isTopicRestricted; }
void Channel::setTopicRestricted(const bool topicRestricted) { _isTopicRestricted = topicRestricted; }

const std::string &Channel::getPassword() const { return _password; }
void Channel::setPassword(const std::string &password) { _password = password; }

size_t Channel::getClientLimit() const { return _clientLimit; }
void Channel::setClientLimit(const size_t limit) { _clientLimit = limit; }

bool Channel::isInvited(const std::string &nickname) const {
  return std::find(_invitedUsers.begin(), _invitedUsers.end(), nickname) != _invitedUsers.end();
}

void Channel::addInvite(const std::string &nickname) {
  if (!isInvited(nickname)) {
    _invitedUsers.push_back(nickname);
  }
}

void Channel::removeInvite(const std::string &nickname) {
  const std::vector<std::string>::iterator it = std::find(_invitedUsers.begin(), _invitedUsers.end(), nickname);
  if (it != _invitedUsers.end()) {
    _invitedUsers.erase(it);
  }
}
