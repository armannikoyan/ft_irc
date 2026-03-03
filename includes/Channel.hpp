#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <map>
#include <string>

#include "Client.hpp"

class Channel {
  private:
  std::string _name;
  std::string _topic;
  std::string _password;

  std::map<int, Client *> _clients;
  std::map<int, Client *> _operators;
  std::list<int> _joinOrder;

  bool _isInviteOnly;
  bool _isTopicRestricted;
  size_t _clientLimit;
  std::vector<std::string> _invitedUsers;

  Channel();
  Channel(const Channel &);
  Channel &operator=(const Channel &);

  public:
  explicit Channel(const std::string &);
  ~Channel();

  const std::string &getName() const;
  const std::string &getTopic() const;
  void setTopic(const std::string &);

  const std::map<int, Client *> &getClients() const;

  void addClient(Client *);
  void removeClient(int);
  bool hasClient(int) const;

  void addOperator(Client *);
  void removeOperator(int);
  bool isOperator(int) const;

  std::string getClientList() const;
  Client *getOldestClient() const;

  bool isInviteOnly() const;
  void setInviteOnly(bool);

  bool isTopicRestricted() const;
  void setTopicRestricted(bool);

  const std::string &getPassword() const;
  void setPassword(const std::string &);

  size_t getClientLimit() const;
  void setClientLimit(size_t);

  bool isInvited(const std::string &) const;
  void addInvite(const std::string &);
  void removeInvite(const std::string &);
};

#endif
