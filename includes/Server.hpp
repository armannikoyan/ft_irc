#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <poll.h>
#include <string>
#include <vector>

#include "Client.hpp"

class Server {
  private:
  uint16_t _port;
  int _serverFd;
  std::string _password;

  std::vector<struct pollfd> _fds;
  std::map<int, Client *> _clients;

  typedef void (Server::*CommandHandler)(int, const std::vector<std::string> &);
  std::map<std::string, CommandHandler> _commands;

  void _handleCap(int, const std::vector<std::string> &);
  void _handleNick(int, const std::vector<std::string> &);
  void _handleUser(int, const std::vector<std::string> &);
  void _handlePass(int, const std::vector<std::string> &);
  void _handlePing(int, const std::vector<std::string> &);

  void _initServer();
  void _runServer();
  void _acceptClient();
  void _receiveData(int);
  void _removeClient(int);

  void _sendResponse(int, const std::string &);

  void _initCommands();
  void _processCommand(int, const std::string &);

  bool _isNicknameInUse(const std::string &) const;
  void _checkRegistration(Client *);

  Server();
  Server(const Server &);
  Server &operator=(const Server &);

  public:
  Server(uint16_t, const std::string &);
  ~Server();

  void start();
};

#endif
