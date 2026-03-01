#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <set>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

#include "Server.hpp"
#include "ircdefine.hpp"

char IrcCompare::toIrcLower(const char c) {
  if (c >= 'A' && c <= 'Z')
    return c - 'A' + 'a';
  if (c == '[')
    return '{';
  if (c == ']')
    return '}';
  if (c == '\\')
    return '|';
  if (c == '~')
    return '^';
  return c;
}

bool IrcCompare::operator()(const std::string &a, const std::string &b) const {
  const size_t min_len = (a.length() < b.length()) ? a.length() : b.length();
  for (size_t i = 0; i < min_len; ++i) {
    const char ca = toIrcLower(a[i]);
    const char cb = toIrcLower(b[i]);
    if (ca < cb)
      return true;
    if (ca > cb)
      return false;
  }
  return a.length() < b.length();
}

Server::Server(const uint16_t port, const std::string &password) : _port(port), _serverFd(-1), _password(password) {
  _initCommands();
}

Server::~Server() {
  for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
    if (close(it->first) == -1)
      std::cerr << "Error: close failed" << std::endl;

    delete it->second;
  }
  _clients.clear();

  for (std::map<std::string, Channel *, IrcCompare>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
    delete it->second;
  }
  _channels.clear();

  _commands.clear();
  _fds.clear();

  if (_serverFd != -1 && close(_serverFd) == -1)
    std::cerr << "Error: close failed" << std::endl;
}

std::vector<std::string> Server::_split(const std::string &s, const char delimiter) {
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(s);

  while (std::getline(tokenStream, token, delimiter)) {
    if (!token.empty()) {
      tokens.push_back(token);
    }
  }
  return tokens;
}

void Server::_handleCap(const int fd, const std::vector<std::string> &args) {
  if (args.size() < 2)
    return;

  const std::string &subcommand = args[1];
  if (subcommand == "LS") {
    _sendResponse(fd, ":" + SERV_NAME + " CAP * LS :\r\n");
  } else if (subcommand == "REQ") {
    const std::string requestedCaps = (args.size() > 2) ? args[2] : "";
    _sendResponse(fd, ":" + SERV_NAME + " CAP * NAK :" + requestedCaps + "\r\n");
  } else if (subcommand == "END") {
    std::cout << "FD " << fd << " finished CAP negotiation." << std::endl;
  }
}

void Server::_handleNick(const int fd, const std::vector<std::string> &args) {
  Client *client = _clients[fd];

  if (!_password.empty() && !client->hasPassword()) {
    const std::string nickname = client->getNickname();
    _sendResponse(fd, ERR_PASSWDMISMATCH((nickname.empty() ? "*" : nickname)));
    return;
  }

  if (args.size() < 2) {
    const std::string nickname = _clients[fd]->getNickname();
    _sendResponse(fd, ERR_NONICKNAMEGIVEN((nickname.empty() ? "*" : nickname)));
    return;
  }

  const std::string &newNick = args[1];
  const std::string oldNick = client->getNickname();

  if (oldNick == newNick)
    return;

  if (!_isValidNickname(newNick)) {
    const std::string nickname = oldNick.empty() ? "*" : oldNick;
    _sendResponse(fd, ERR_ERRONEUSNICKNAME(nickname, newNick));
    return;
  }

  if (_isNicknameInUse(newNick)) {
    const std::string nickname = oldNick.empty() ? "*" : oldNick;
    _sendResponse(fd, ERR_NICKNAMEINUSE(nickname, newNick));
    return;
  }

  client->setNickname(newNick);

  if (client->isRegistered()) {
    const std::string ip = client->getIpAddress();
    const std::string nickMsg = ":" + oldNick + "!~" + client->getUsername() + "@" + ip + " NICK :" + newNick + "\r\n";

    _broadcastToNeighbors(fd, nickMsg);

    std::cout << "Broadcasted NICK change to clients." << std::endl;
  }

  std::cout << "FD " << fd << " set nickname to " << newNick << std::endl;

  _checkRegistration(client);
}

void Server::_handleUser(const int fd, const std::vector<std::string> &args) {
  Client *client = _clients[fd];

  if (!_password.empty() && !client->hasPassword()) {
    const std::string nickname = client->getNickname();
    _sendResponse(fd, ERR_PASSWDMISMATCH((nickname.empty() ? "*" : nickname)));
    return;
  }

  if (client->isRegistered()) {
    const std::string nickname = client->getNickname();
    _sendResponse(fd, ERR_ALREADYREGISTRED((nickname.empty() ? "*" : nickname)));
    return;
  }

  if (args.size() < 5) {
    const std::string nickname = _clients[fd]->getNickname();
    _sendResponse(fd, ERR_NEEDMOREPARAMS((nickname.empty() ? "*" : nickname), args[0]));
    return;
  }

  client->setUsername(args[1]);
  client->setHostname(client->getIpAddress());
  client->setServername(SERV_NAME);
  client->setRealname(args[4]);

  std::cout << "FD " << fd << " set username to " << args[1] << ", hostname to " << client->getIpAddress()
            << ", servername to " << SERV_NAME << " and realname to " << args[4] << std::endl;

  _checkRegistration(client);
}

void Server::_handlePass(const int fd, const std::vector<std::string> &args) {
  Client *client = _clients[fd];

  if (client->isRegistered()) {
    const std::string nickname = client->getNickname();
    _sendResponse(fd, ERR_ALREADYREGISTRED((nickname.empty() ? "*" : nickname)));
    return;
  }

  if (args.size() < 2) {
    const std::string nickname = _clients[fd]->getNickname();
    _sendResponse(fd, ERR_NEEDMOREPARAMS((nickname.empty() ? "*" : nickname), args[0]));
    return;
  }

  if (args[1] == _password) {
    client->setHasPassword(true);
    std::cout << "FD " << fd << " provided correct password." << std::endl;
  } else {
    const std::string nickname = _clients[fd]->getNickname();
    _sendResponse(fd, ERR_PASSWDMISMATCH((nickname.empty() ? "*" : nickname)));
  }
}

void Server::_handlePing(const int fd, const std::vector<std::string> &args) {
  const Client *client = _clients[fd];
  const std::string nick = client->getNickname().empty() ? "*" : client->getNickname();

  if (args.size() < 2 || args[1].empty()) {
    _sendResponse(fd, ERR_NOORIGIN(nick));
    return;
  }

  const std::string response = ":" + SERV_NAME + " PONG " + SERV_NAME + " :" + args[1] + "\r\n";
  _sendResponse(fd, response);
}

void Server::_handlePong(const int fd, const std::vector<std::string> &args) {
  Client *client = _clients[fd];
  const std::string nick = client->getNickname().empty() ? "*" : client->getNickname();

  if (args.size() < 2 || args[1].empty()) {
    _sendResponse(fd, ERR_NOORIGIN(nick));
    return;
  }

  if (_clients.find(fd) != _clients.end()) {
    _clients[fd]->setPingSent(false);
    _clients[fd]->updateActivityTime();
  }
}

void Server::_handleJoin(const int fd, const std::vector<std::string> &args) {
  Client *client = _clients[fd];

  if (!client->isRegistered()) {
    const std::string nick = client->getNickname().empty() ? "*" : client->getNickname();
    _sendResponse(fd, ERR_NOTREGISTERED(nick));
    return;
  }

  if (args.size() < 2) {
    _sendResponse(fd, ERR_NEEDMOREPARAMS(client->getNickname(), "JOIN"));
    return;
  }

  if (args[1] == "0") {
    std::vector<std::string> channelsToLeave;
    for (std::map<std::string, Channel *>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
      if (it->second->hasClient(fd)) {
        channelsToLeave.push_back(it->first);
      }
    }

    for (size_t i = 0; i < channelsToLeave.size(); ++i) {
      const std::string &channelName = channelsToLeave[i];
      const std::string partMsg = ":" + client->getNickname() + "!~" + client->getUsername() + "@" +
                                  client->getIpAddress() + " PART " + channelName + " :Left all channels\r\n";

      _broadcastToChannel(channelName, partMsg);

      _removeClientFromChannel(fd, channelName);
    }

    std::cout << "FD " << fd << " left all channels (JOIN 0)." << std::endl;
    return;
  }

  const std::vector<std::string> channelsToJoin = _split(args[1], ',');

  // TODO: handling of MODE +k

  for (size_t i = 0; i < channelsToJoin.size(); ++i) {
    const std::string &channelName = channelsToJoin[i];

    if (channelName.empty())
      continue;

    if (channelName[0] != '#' && channelName[0] != '&') {
      _sendResponse(fd, ERR_NOSUCHCHANNEL(client->getNickname(), channelName));
      continue;
    }

    if (_channels.find(channelName) == _channels.end()) {
      _channels[channelName] = new Channel(channelName);
      _channels[channelName]->addOperator(client);
      std::cout << "Channel " << channelName << " created by FD " << fd << std::endl;
    }

    Channel *channel = _channels[channelName];

    if (channel->hasClient(fd))
      continue;

    channel->addClient(client);

    std::string joinMsg = ":" + client->getNickname() + "!~" + client->getUsername() + "@" + client->getIpAddress() +
                          " JOIN :" + channelName + "\r\n";
    _broadcastToChannel(channelName, joinMsg);

    if (channel->getTopic().empty()) {
      _sendResponse(fd, RPL_NOTOPIC(client->getNickname(), channelName));
    } else {
      _sendResponse(fd, RPL_TOPIC(client->getNickname(), channelName, channel->getTopic()));
    }

    std::string namesList = channel->getClientList();
    _sendResponse(fd, RPL_NAMREPLY(client->getNickname(), channelName, namesList));
    _sendResponse(fd, RPL_ENDOFNAMES(client->getNickname(), channelName));
  }
}

void Server::_handleQuit(const int fd, const std::vector<std::string> &args) {
  Client *client = _clients[fd];

  const std::string reason = args.size() > 1 ? args[1] : "Client Quit";

  if (client->isRegistered()) {
    const std::string quitMsg = ":" + client->getNickname() + "!~" + client->getUsername() + "@" +
                                client->getIpAddress() + " QUIT :" + reason + "\r\n";
    _broadcastToNeighbors(fd, quitMsg);
  }

  _sendResponse(fd, "ERROR :Closing Link: " + client->getIpAddress() + " (" + reason + ")\r\n");

  std::cout << "FD " << fd << " is disconnecting (QUIT)." << std::endl;

  client->setDisconnecting(true);
}

void Server::_handlePrivMsg(const int fd, const std::vector<std::string> &args) {
  const Client *client = _clients[fd];

  if (!client->isRegistered()) {
    const std::string nick = client->getNickname().empty() ? "*" : client->getNickname();
    _sendResponse(fd, ERR_NOTREGISTERED(nick));
    return;
  }

  if (args.size() < 2) {
    _sendResponse(fd, ERR_NORECIPIENT(client->getNickname(), "PRIVMSG"));
    return;
  }
  if (args.size() > 3) {
    _sendResponse(fd, ERR_NOTEXTTOSEND(client->getNickname()));
    return;
  }

  const std::vector<std::string> targets = _split(args[1], ',');
  const std::string &message = args[2];

  for (size_t i = 0; i < targets.size(); ++i) {
    const std::string &target = targets[i];
    if (target.empty())
      continue;

    const std::string fullMsg = ":" + client->getNickname() + "!~" + client->getUsername() + "@" +
                                client->getIpAddress() + " PRIVMSG " + target + " :" + message + "\r\n";

    if (target[0] == '#' || target[0] == '&') {
      if (_channels.find(target) == _channels.end()) {
        _sendResponse(fd, ERR_NOSUCHNICK(client->getNickname(), target));
        return;
      }

      const Channel *channel = _channels[target];
      if (!channel->hasClient(fd)) {
        _sendResponse(fd, ERR_CANNOTSENDTOCHAN(client->getNickname(), target));
        return;
      }

      _broadcastToChannel(target, fullMsg, fd);
    } else {
      const Client *targetClient = _getClientByNickname(target);
      if (!targetClient) {
        _sendResponse(fd, ERR_NOSUCHNICK(client->getNickname(), target));
        continue;
      }
      const int targetFd = targetClient->getFd();

      _sendResponse(targetFd, fullMsg);
    }
  }
}

void Server::_handleNotice(const int fd, const std::vector<std::string> &args) {
  const Client *client = _clients[fd];

  if (!client->isRegistered() || args.size() < 3)
    return;

  const std::vector<std::string> targets = _split(args[1], ',');
  const std::string &message = args[2];

  for (size_t i = 0; i < targets.size(); ++i) {
    const std::string &target = targets[i];
    if (target.empty())
      continue;

    const std::string fullMsg = ":" + client->getNickname() + "!~" + client->getUsername() + "@" +
                                client->getIpAddress() + " NOTICE " + target + " :" + message + "\r\n";

    if (target[0] == '#' || target[0] == '&') {
      if (_channels.find(target) == _channels.end())
        continue;

      const Channel *channel = _channels[target];
      if (!channel->hasClient(fd))
        continue;

      _broadcastToChannel(target, fullMsg, fd);
    } else {
      const Client *targetClient = _getClientByNickname(target);
      if (!targetClient)
        continue;
      const int targetFd = targetClient->getFd();

      _sendResponse(targetFd, fullMsg);
    }
  }
}

void Server::_handleKick(const int fd, const std::vector<std::string> &args) {
  const Client *kicker = _clients[fd];

  if (!kicker->isRegistered()) {
    const std::string nick = kicker->getNickname().empty() ? "*" : kicker->getNickname();
    _sendResponse(fd, ERR_NOTREGISTERED(nick));
    return;
  }

  if (args.size() < 3) {
    _sendResponse(fd, ERR_NEEDMOREPARAMS(kicker->getNickname(), "KICK"));
    return;
  }

  std::string reason = args.size() > 3 ? args[3] : "Kicked by operator";

  const std::vector<std::string> channels = _split(args[1], ',');
  const std::vector<std::string> targetsToKick = _split(args[2], ',');

  if (channels.empty() || targetsToKick.empty()) {
    _sendResponse(fd, ERR_NEEDMOREPARAMS(kicker->getNickname(), "KICK"));
    return;
  }

  if (channels.size() != 1 && channels.size() != targetsToKick.size()) {
    _sendResponse(fd, ERR_NEEDMOREPARAMS(kicker->getNickname(), "KICK"));
    return;
  }

  for (size_t i = 0; i < targetsToKick.size(); ++i) {
    const std::string &targetNick = targetsToKick[i];

    const std::string &channelName = (channels.size() == 1) ? channels[0] : channels[i];

    if (targetNick.empty() || channelName.empty())
      continue;

    if (_channels.find(channelName) == _channels.end()) {
      _sendResponse(fd, ERR_NOSUCHCHANNEL(kicker->getNickname(), channelName));
      continue;
    }

    Channel *channel = _channels[channelName];

    if (!channel->hasClient(fd)) {
      _sendResponse(fd, ERR_NOTONCHANNEL(kicker->getNickname(), channelName));
      continue;
    }

    if (!channel->isOperator(fd)) {
      _sendResponse(fd, ERR_CHANOPRIVSNEEDED(kicker->getNickname(), channelName));
      continue;
    }

    Client *targetClient = NULL;
    const std::map<int, Client *> &chanClients = channel->getClients();

    for (std::map<int, Client *>::const_iterator it = chanClients.begin(); it != chanClients.end(); ++it) {
      if (it->second->getNickname() == targetNick) {
        targetClient = it->second;
        break;
      }
    }

    if (!targetClient) {
      _sendResponse(fd, ERR_USERNOTINCHANNEL(kicker->getNickname(), targetNick, channelName));
      continue;
    }

    const std::string kickMsg = ":" + kicker->getNickname() + "!~" + kicker->getUsername() + "@" +
                                kicker->getIpAddress() + " KICK " + channelName + " " + targetNick + " :" + reason +
                                "\r\n";

    _broadcastToChannel(channelName, kickMsg);

    _removeClientFromChannel(targetClient->getFd(), channelName);
  }
}

void Server::_handleTopic(const int fd, const std::vector<std::string> &args) {
  const Client *client = _clients[fd];

  if (!client->isRegistered()) {
    const std::string nick = client->getNickname().empty() ? "*" : client->getNickname();
    _sendResponse(fd, ERR_NOTREGISTERED(nick));
    return;
  }

  if (args.size() < 2) {
    _sendResponse(fd, ERR_NEEDMOREPARAMS(client->getNickname(), "TOPIC"));
    return;
  }

  const std::string &channelName = args[1];

  if (_channels.find(channelName) == _channels.end()) {
    _sendResponse(fd, ERR_NOSUCHCHANNEL(client->getNickname(), channelName));
    return;
  }

  Channel *channel = _channels[channelName];

  if (!channel->hasClient(fd)) {
    _sendResponse(fd, ERR_NOTONCHANNEL(client->getNickname(), channelName));
    return;
  }

  if (args.size() == 2) {
    if (channel->getTopic().empty()) {
      _sendResponse(fd, RPL_NOTOPIC(client->getNickname(), channelName));
    } else {
      _sendResponse(fd, RPL_TOPIC(client->getNickname(), channelName, channel->getTopic()));
    }
    return;
  }

  // TODO: rewrite after MODE
  if (!channel->isOperator(fd)) {
    _sendResponse(fd, ERR_CHANOPRIVSNEEDED(client->getNickname(), channelName));
    return;
  }

  const std::string &newTopic = args[2];
  channel->setTopic(newTopic);

  const std::string topicMsg = ":" + client->getNickname() + "!~" + client->getUsername() + "@" +
                               client->getIpAddress() + " TOPIC " + channelName + " :" + newTopic + "\r\n";

  _broadcastToChannel(channelName, topicMsg);

  std::cout << "User " << client->getNickname() << " changed topic of " << channelName << " to: " << newTopic
            << std::endl;
}

void Server::_handleInvite(const int fd, const std::vector<std::string> &args) {
  const Client *client = _clients[fd];

  if (!client->isRegistered()) {
    const std::string nick = client->getNickname().empty() ? "*" : client->getNickname();
    _sendResponse(fd, ERR_NOTREGISTERED(nick));
    return;
  }

  if (args.size() < 3) {
    _sendResponse(fd, ERR_NEEDMOREPARAMS(client->getNickname(), "INVITE"));
    return;
  }

  const std::string &targetNick = args[1];
  const std::string &channelName = args[2];

  const Client *targetClient = _getClientByNickname(targetNick);
  if (!targetClient) {
    _sendResponse(fd, ERR_NOSUCHNICK(client->getNickname(), targetNick));
    return;
  }

  if (_channels.find(channelName) == _channels.end()) {
    _sendResponse(fd, ERR_NOSUCHCHANNEL(client->getNickname(), channelName));
    return;
  }

  const Channel *channel = _channels[channelName];

  if (!channel->hasClient(fd)) {
    _sendResponse(fd, ERR_NOTONCHANNEL(client->getNickname(), channelName));
    return;
  }

  if (!channel->isOperator(fd)) {
    _sendResponse(fd, ERR_CHANOPRIVSNEEDED(client->getNickname(), channelName));
    return;
  }

  if (channel->hasClient(targetClient->getFd())) {
    _sendResponse(fd, ERR_USERONCHANNEL(client->getNickname(), targetNick, channelName));
    return;
  }

  _sendResponse(fd, RPL_INVITING(client->getNickname(), targetNick, channelName));

  const std::string inviteMsg = ":" + client->getNickname() + "!~" + client->getUsername() + "@" +
                                client->getIpAddress() + " INVITE " + targetNick + " :" + channelName + "\r\n";
  _sendResponse(targetClient->getFd(), inviteMsg);

  // TODO: rewrite after MODE +i
}

void Server::_handlePart(const int fd, const std::vector<std::string> &args) {
  const Client *client = _clients[fd];

  if (!client->isRegistered()) {
    const std::string nick = client->getNickname().empty() ? "*" : client->getNickname();
    _sendResponse(fd, ERR_NOTREGISTERED(nick));
    return;
  }

  if (args.size() < 2) {
    _sendResponse(fd, ERR_NEEDMOREPARAMS(client->getNickname(), "PART"));
    return;
  }

  std::string reason = args.size() > 2 ? args[2] : "Leaving";

  const std::vector<std::string> channelsToPart = _split(args[1], ',');

  for (size_t i = 0; i < channelsToPart.size(); ++i) {
    const std::string &channelName = channelsToPart[i];

    if (channelName.empty())
      continue;

    if (_channels.find(channelName) == _channels.end()) {
      _sendResponse(fd, ERR_NOSUCHCHANNEL(client->getNickname(), channelName));
      continue;
    }

    Channel *channel = _channels[channelName];

    if (!channel->hasClient(fd)) {
      _sendResponse(fd, ERR_NOTONCHANNEL(client->getNickname(), channelName));
      continue;
    }

    const std::string partMsg = ":" + client->getNickname() + "!~" + client->getUsername() + "@" +
                                client->getIpAddress() + " PART " + channelName + " :" + reason + "\r\n";

    _broadcastToChannel(channelName, partMsg);

    _removeClientFromChannel(fd, channelName);
  }
}

void Server::_initServer() {
  _serverFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (_serverFd == -1)
    throw std::runtime_error("Error: socket creation failed");

  const int opt = 1;
  if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
    if (close(_serverFd) == -1)
      throw std::runtime_error("Error: close failed");

    throw std::runtime_error("Error: setsockopt failed");
  }

  if (fcntl(_serverFd, F_SETFL, O_NONBLOCK) == -1) {
    if (close(_serverFd) == -1)
      throw std::runtime_error("Error: close failed");

    throw std::runtime_error("Error: fcntl failed");
  }

  struct sockaddr_in address = {};
  address.sin_family = AF_INET;
  address.sin_port = htons(_port);
  address.sin_addr.s_addr = INADDR_ANY;

  if (bind(_serverFd, reinterpret_cast<struct sockaddr *>(&address), sizeof(address)) == -1) {
    if (close(_serverFd) == -1)
      throw std::runtime_error("Error: close failed");

    throw std::runtime_error("Error: bind failed");
  }

  if (listen(_serverFd, SOMAXCONN) == -1) {
    if (close(_serverFd) == -1)
      throw std::runtime_error("Error: close failed");

    throw std::runtime_error("Error: listen failed");
  }

  const struct pollfd pfd = {_serverFd, POLLIN, 0};
  _fds.push_back(pfd);

  std::cout << "Server listening on port " << _port << std::endl;
}

void Server::_runServer() {
  while (true) {
    const int pollCount = poll(&_fds[0], _fds.size(), 5000);
    if (pollCount == -1) {
      if (errno == EINTR)
        continue;
      throw std::runtime_error("Error: poll failed");
    }

    for (size_t i = 0; i < _fds.size();) {
      struct pollfd pfd = _fds[i];
      bool clientRemoved = false;

      if (pfd.revents & POLLIN) {
        if (pfd.fd == _serverFd)
          _acceptClient();
        else {
          _receiveData(pfd.fd);

          if (_clients.find(pfd.fd) == _clients.end())
            clientRemoved = true;
        }
      }

      if (!clientRemoved && (pfd.revents & POLLOUT)) {
        Client *client = _clients[pfd.fd];
        const std::string data = client->getSendBuffer();

        const ssize_t bytesSent = send(pfd.fd, data.c_str(), data.size(), MSG_NOSIGNAL);
        if (bytesSent == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
          std::cerr << "Error sending data to FD " << _fds[i].fd << std::endl;
          _removeClient(_fds[i].fd);
          clientRemoved = true;
        } else if (bytesSent > 0) {
          client->clearSentData(bytesSent);

          if (client->getSendBuffer().empty()) {
            _fds[i].events &= ~POLLOUT;

            if (client->isDisconnecting()) {
              std::cout << "Buffer empty, closing FD " << _fds[i].fd << std::endl;
              _removeClient(_fds[i].fd);
              clientRemoved = true;
            }
          }
        }
      }

      if (!clientRemoved)
        ++i;
    }
    _checkTimeouts();
  }
}

void Server::_acceptClient() {
  struct sockaddr_in address = {};
  socklen_t address_len = sizeof(address);

  const int clientFd = accept(_serverFd, reinterpret_cast<struct sockaddr *>(&address), &address_len);
  if (clientFd == -1) {
    std::cerr << "Error: accept failed" << std::endl;
    return;
  }

  if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1) {
    std::cerr << "Error: fcntl failed" << std::endl;
    close(clientFd);
    return;
  }

#ifdef __APPLE__
  int no_sigpipe = 1;
  if (setsockopt(clientFd, SOL_SOCKET, SO_NOSIGPIPE, &no_sigpipe, sizeof(no_sigpipe)) == -1) {
    std::cerr << "Error: setsockopt SO_NOSIGPIPE failed" << std::endl;
    close(clientFd);
    return;
  }
#endif

  const struct pollfd pfd = {clientFd, POLLIN, 0};
  _fds.push_back(pfd);

  const std::string ip = inet_ntoa(address.sin_addr);
  _clients[clientFd] = new Client(clientFd, ip);

  std::cout << "Client " << clientFd << " connected" << std::endl;
}

void Server::_receiveData(const int fd) {
  char buffer[1024] = {};

  const ssize_t bytesRead = recv(fd, buffer, sizeof(buffer), 0);
  if (bytesRead == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
    std::cerr << "Error reading from client FD: " << fd << std::endl;
    _removeClient(fd);
    return;
  }
  if (bytesRead == 0) {
    std::cout << "Client FD " << fd << " disconnected." << std::endl;
    _removeClient(fd);
    return;
  }

  if (_clients.find(fd) != _clients.end()) {
    _clients[fd]->updateActivityTime();
    _clients[fd]->setPingSent(false);
  }

  const std::string receivedData(buffer, bytesRead);
  _clients[fd]->appendReadBuffer(receivedData);

  if (_clients[fd]->isBufferViolatingRFC()) {
    std::cerr << "FD " << fd << " violated RFC 2812 message length limit (DoS protection). Disconnecting." << std::endl;
    _sendResponse(fd, "ERROR :Closing Link: Message length exceeded 512 bytes\r\n");
    _clients[fd]->setDisconnecting(true);
    return;
  }

  if (_clients[fd]->isDisconnecting())
    return;

  while (_clients.find(fd) != _clients.end() && _clients[fd]->hasCompleteCommand()) {
    std::string command = _clients[fd]->getCompleteCommand();

    if (!command.empty()) {
      std::cout << "Received command from FD " << fd << ": [" << command << "]" << std::endl;

      _processCommand(fd, command);
    }
  }
}

void Server::_removeClient(const int fd) {
  std::vector<std::string> channelsToLeave;
  for (std::map<std::string, Channel *>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
    if (it->second->hasClient(fd)) {
      channelsToLeave.push_back(it->first);
    }
  }

  for (size_t i = 0; i < channelsToLeave.size(); ++i)
    _removeClientFromChannel(fd, channelsToLeave[i]);

  if (close(fd) == -1)
    std::cerr << "Error: close failed" << std::endl;

  if (_clients.find(fd) != _clients.end()) {
    delete _clients[fd];
    _clients.erase(fd);
  }

  for (size_t j = 0; j < _fds.size(); ++j) {
    if (_fds[j].fd == fd) {
      _fds[j] = _fds.back();
      _fds.pop_back();
      break;
    }
  }
}

void Server::_sendResponse(const int fd, const std::string &response) {
  if (_clients.find(fd) == _clients.end())
    return;

  _clients[fd]->appendSendBuffer(response);
  for (std::vector<struct pollfd>::iterator it = _fds.begin(); it != _fds.end(); ++it) {
    if (it->fd == fd) {
      it->events |= POLLOUT;
      break;
    }
  }
}

void Server::_initCommands() {
  _commands["CAP"] = &Server::_handleCap;
  _commands["NICK"] = &Server::_handleNick;
  _commands["USER"] = &Server::_handleUser;
  _commands["PASS"] = &Server::_handlePass;
  _commands["PING"] = &Server::_handlePing;
  _commands["PONG"] = &Server::_handlePong;
  _commands["JOIN"] = &Server::_handleJoin;
  _commands["QUIT"] = &Server::_handleQuit;
  _commands["PRIVMSG"] = &Server::_handlePrivMsg;
  _commands["NOTICE"] = &Server::_handleNotice;
  _commands["KICK"] = &Server::_handleKick;
  _commands["TOPIC"] = &Server::_handleTopic;
  _commands["INVITE"] = &Server::_handleInvite;
  _commands["PART"] = &Server::_handlePart;
}

void Server::_processCommand(const int fd, const std::string &command) {
  if (command.empty())
    return;

  std::vector<std::string> args;
  size_t start = 0;
  size_t end = command.find(' ');

  while (end != std::string::npos) {
    args.push_back(command.substr(start, end - start));
    start = end + 1;

    while (start < command.length() && command[start] == ' ')
      ++start;

    if (start < command.length() && command[start] == ':') {
      args.push_back(command.substr(start + 1));
      start = command.length();
      break;
    }

    end = command.find(' ', start);
  }
  if (start < command.length()) {
    args.push_back(command.substr(start));
  }

  if (args.empty())
    return;

  const std::string &cmd = args[0];
  const std::map<std::string, CommandHandler>::iterator it = _commands.find(cmd);
  if (it == _commands.end()) {
    std::cerr << "Error: unknown command '" << cmd << "'" << std::endl;
    _sendResponse(fd, ":" + SERV_NAME + " 421 " + cmd + " :Unknown command\r\n");
    return;
  }
  (this->*(it->second))(fd, args);
}

bool Server::_isNicknameInUse(const std::string &nickname) const { return _getClientByNickname(nickname) != NULL; }

bool Server::_isValidNickname(const std::string &nickname) const {
  if (nickname.empty() || nickname.length() > 9)
    return false;

  const char first = nickname[0];
  const bool isFirstValid = (first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z') ||
                            (first >= 0x5B && first <= 0x60) || (first >= 0x7B && first <= 0x7D);

  if (!isFirstValid)
    return false;

  for (size_t i = 1; i < nickname.length(); ++i) {
    const char c = nickname[i];
    const bool isValid = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
                         (c >= 0x5B && c <= 0x60) || (c >= 0x7B && c <= 0x7D) || (c == '-');

    if (!isValid)
      return false;
  }

  return true;
}

Client *Server::_getClientByNickname(const std::string &nickname) const {
  for (std::map<int, Client *>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
    const std::string &clientNick = it->second->getNickname();

    if (clientNick.length() == nickname.length()) {
      for (size_t i = 0; i < clientNick.length(); ++i) {
        if (IrcCompare::toIrcLower(clientNick[i]) == IrcCompare::toIrcLower(nickname[i])) {
          return (it->second);
        }
      }
    }
  }
  return NULL;
}

void Server::_checkRegistration(Client *client) {
  if (client->isRegistered() || client->getNickname().empty() || client->getUsername().empty())
    return;

  client->setRegistered(true);

  _sendResponse(client->getFd(), RPL_WELCOME(client->getNickname(), client->getUsername(), client->getHostname()));

  std::cout << "Client FD " << client->getFd() << " is fully registered!" << std::endl;
}

void Server::_checkTimeouts() {
  const time_t now = std::time(NULL);

  std::vector<int> fdsToDisconnect;
  for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
    Client *client = it->second;
    const int fd = it->first;

    if (now - client->getLastActivityTime() > DEAD_TIMEOUT) {
      fdsToDisconnect.push_back(fd);
    } else if (now - client->getLastActivityTime() > PING_TIMEOUT && !client->isPingSent()) {
      _sendResponse(fd, "PING :" + SERV_NAME + "\r\n");
      client->setPingSent(true);
      std::cout << "Sent PING to FD " << fd << " due to inactivity." << std::endl;
    }
  }

  for (size_t i = 0; i < fdsToDisconnect.size(); ++i) {
    const int fd = fdsToDisconnect[i];
    std::cout << "FD " << fd << " disconnected (Ping timeout)." << std::endl;

    std::vector<std::string> quitArgs;
    quitArgs.push_back("QUIT");
    quitArgs.push_back("Ping timeout");
    _handleQuit(fd, quitArgs);
  }
}

void Server::_removeClientFromChannel(const int fd, const std::string &channelName) {
  std::map<std::string, Channel *>::iterator it = _channels.find(channelName);
  if (it == _channels.end())
    return;

  Channel *channel = it->second;

  channel->removeClient(fd);

  if (channel->getClients().empty()) {
    delete channel;
    _channels.erase(it);
    std::cout << "Channel " << channelName << " deleted (empty)." << std::endl;
  } else {
    bool hasOperator = false;
    const std::map<int, Client *> &clients = channel->getClients();

    for (std::map<int, Client *>::const_iterator cit = clients.begin(); cit != clients.end(); ++cit) {
      if (channel->isOperator(cit->first)) {
        hasOperator = true;
        break;
      }
    }

    if (!hasOperator) {
      Client *newOp = clients.begin()->second;
      channel->addOperator(newOp);

      const std::string modeMsg = ":" + SERV_NAME + " MODE " + channelName + " +o " + newOp->getNickname() + "\r\n";
      _broadcastToChannel(channelName, modeMsg);

      std::cout << "Auto-assigned operator status to " << newOp->getNickname() << " in " << channelName << std::endl;
    }
  }
}

void Server::_broadcastToChannel(const std::string &channelName, const std::string &message, int excludeFd) {
  if (_channels.find(channelName) == _channels.end())
    return;

  const Channel *channel = _channels[channelName];
  const std::map<int, Client *> &clients = channel->getClients();

  for (std::map<int, Client *>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
    if (it->first != excludeFd) {
      _sendResponse(it->first, message);
    }
  }
}

void Server::_broadcastToNeighbors(const int fd, const std::string &message) {
  std::set<int> recipients;

  recipients.insert(fd);

  for (std::map<std::string, Channel *>::const_iterator it = _channels.begin(); it != _channels.end(); ++it) {
    if (it->second->hasClient(fd)) {
      const std::map<int, Client *> &chanClients = it->second->getClients();

      for (std::map<int, Client *>::const_iterator cit = chanClients.begin(); cit != chanClients.end(); ++cit) {
        recipients.insert(cit->first);
      }
    }
  }

  for (std::set<int>::const_iterator it = recipients.begin(); it != recipients.end(); ++it) {
    _sendResponse(*it, message);
  }
}

void Server::start() {
  _initServer();
  _runServer();
}
