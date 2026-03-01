#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <poll.h>
#include <string>
#include <vector>

#include "Channel.hpp"
#include "Client.hpp"

struct IrcCompare {
  static char toIrcLower(char);

  bool operator()(const std::string &, const std::string &) const;
};

class Server {
  private:
  uint16_t _port;
  int _serverFd;
  std::string _password;

  std::vector<struct pollfd> _fds;
  std::map<int, Client *> _clients;

  std::map<std::string, Channel *, IrcCompare> _channels;

  typedef void (Server::*CommandHandler)(int, const std::vector<std::string> &);
  std::map<std::string, CommandHandler> _commands;

  public:
  Server(uint16_t, const std::string &);
  ~Server();

  private:
  Server();
  Server(const Server &);
  Server &operator=(const Server &);

  static std::vector<std::string> _split(const std::string &, char);

  void _handleCap(int, const std::vector<std::string> &);
  void _handleNick(int, const std::vector<std::string> &);
  void _handleUser(int, const std::vector<std::string> &);
  void _handlePass(int, const std::vector<std::string> &);
  void _handlePing(int, const std::vector<std::string> &);
  void _handlePong(int, const std::vector<std::string> &);
  void _handleJoin(int, const std::vector<std::string> &);
  void _handleQuit(int, const std::vector<std::string> &);
  void _handlePrivMsg(int, const std::vector<std::string> &);
  void _handleNotice(int, const std::vector<std::string> &);
  void _handleKick(int, const std::vector<std::string> &);
  void _handleTopic(int, const std::vector<std::string> &);
  void _handleInvite(int, const std::vector<std::string> &);
  void _handlePart(int, const std::vector<std::string> &);

  void _initServer();
  void _runServer();
  void _acceptClient();
  void _receiveData(int);
  void _removeClient(int);

  void _sendResponse(int, const std::string &);

  void _initCommands();
  void _processCommand(int, const std::string &);

  bool _isNicknameInUse(const std::string &) const;
  bool _isValidNickname(const std::string &) const;
  Client *_getClientByNickname(const std::string &) const;
  void _checkRegistration(Client *);
  void _checkTimeouts();
  void _removeClientFromChannel(int, const std::string &);

  void _broadcastToChannel(const std::string &, const std::string &, int = -1);
  void _broadcastToNeighbors(int, const std::string &);

  public:
  void start();
};

#endif
