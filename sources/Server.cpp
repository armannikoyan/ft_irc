#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

#include "Server.hpp"

#include "ircdefine.hpp"

Server::Server() : _port(0), _serverFd(-1), _password("") { _initCommands(); }

Server::Server(const uint16_t port, const std::string &password) : _port(port), _serverFd(-1), _password(password) {
  _initCommands();
}

Server::~Server() {
  for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
    if (close(it->first) == -1)
      throw std::runtime_error("Error: close failed");

    delete it->second;
  }

  if (_serverFd != -1 && close(_serverFd) == -1)
    throw std::runtime_error("Error: close failed");
}

Server::Server(const Server &other) { *this = other; }

Server &Server::operator=(const Server &other) {
  if (this == &other)
    return *this;

  if (_serverFd != -1 && close(_serverFd) == -1)
    throw std::runtime_error("Error: close failed");
  _fds.clear();
  _clients.clear();

  _port = other._port;
  _password = other._password;
  _serverFd = other._serverFd;
  _fds = other._fds;
  _clients = other._clients;

  return *this;
}

void Server::start() {
  _initServer();
  _runServer();
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
    const int pollCount = poll(&_fds[0], _fds.size(), -1);
    if (pollCount == -1)
      throw std::runtime_error("Error: poll failed");

    for (size_t i = 0; i < _fds.size();) {
      struct pollfd &pfd = _fds[i];
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

        const ssize_t bytesSent = send(pfd.fd, data.c_str(), data.size(), 0);
        if (bytesSent == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
          std::cerr << "Error sending data to FD " << _fds[i].fd << std::endl;
          _removeClient(_fds[i].fd);
          clientRemoved = true;
        } else if (bytesSent > 0) {
          client->clearSentData(bytesSent);
          if (client->getSendBuffer().empty())
            pfd.events &= ~POLLOUT;
        }
      }

      if (!clientRemoved)
        ++i;
    }
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

  const struct pollfd pfd = {clientFd, POLLIN, 0};
  _fds.push_back(pfd);

  const std::string ip = inet_ntoa(address.sin_addr);
  _clients[clientFd] = new Client(clientFd, ip);

  std::cout << "Client " << clientFd << " connected" << std::endl;
}

void Server::_receiveData(const int fd) {
  char buffer[1024] = {};

  // TODO: Change logging from writing fd to writing client's nickname
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

  const std::string receivedData(buffer, bytesRead);
  _clients[fd]->appendReadBuffer(receivedData);
  while (_clients[fd]->hasCompleteCommand()) {
    std::string command = _clients[fd]->getCompleteCommand();

    if (!command.empty()) {
      std::cout << "Received command from FD " << fd << ": [" << command << "]" << std::endl;

      _processCommand(fd, command);
    }
  }
}

void Server::_removeClient(const int fd) {
  if (close(fd) == -1) {
    std::cerr << "Error: close failed" << std::endl;
    return;
  }

  if (_clients.find(fd) != _clients.end()) {
    delete _clients[fd];
    _clients.erase(fd);
  }

  for (std::vector<struct pollfd>::iterator it = _fds.begin(); it != _fds.end(); ++it) {
    if (it->fd == fd) {
      _fds.erase(it);
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

void Server::_handleCap(const int fd, const std::vector<std::string> &args) {
  if (args.size() < 2)
    return;

  const std::string subcommand = args[1];
  if (subcommand == "LS") {
    _sendResponse(fd, ":" + SERV_NAME + " CAP * LS :\r\n");
  } else if (subcommand == "REQ") {
    const std::string requestedCaps = (args.size() > 2) ? args[2] : "";
    _sendResponse(fd, ":" + SERV_NAME + " CAP * NAK :" + requestedCaps + "\r\n");
  } else if (subcommand == "END") {
    std::cout << "FD " << fd << " finished CAP negotiation." << std::endl;
  }
}

void Server::_handlePing(const int fd, const std::vector<std::string> &args) {
  if (args.size() > 1) {
    _sendResponse(fd, "PONG " + args[1] + "\r\n");
  } else {
    _sendResponse(fd, "PONG\r\n");
  }
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

void Server::_checkRegistration(Client *client) {
  if (client->isRegistered() || client->getNickname().empty() || client->getUsername().empty())
    return;

  client->setRegistered(true);

  _sendResponse(client->getFd(), RPL_WELCOME(client->getNickname(), client->getUsername(), client->getHostname()));

  std::cout << "Client FD " << client->getFd() << " is fully registered!" << std::endl;
}

bool Server::_isNicknameInUse(const std::string &nickname) const {
  for (std::map<int, Client *>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
    if (it->second->getNickname() == nickname) {
      return true;
    }
  }
  return false;
}

void Server::_handleNick(const int fd, const std::vector<std::string> &args) {
  Client *client = _clients[fd];

  if (!client->hasPassword()) {
    const std::string nickname = client->getNickname();
    _sendResponse(fd, ERR_PASSWDMISMATCH((nickname.empty() ? "*" : nickname)));
    return;
  }

  if (args.size() < 2) {
    const std::string nickname = _clients[fd]->getNickname();
    _sendResponse(fd, ERR_NONICKNAMEGIVEN((nickname.empty() ? "*" : nickname)));
    return;
  }

  const std::string newNick = args[1];
  const std::string oldNick = client->getNickname();

  if (oldNick == newNick)
    return;

  if (_isNicknameInUse(newNick)) {
    const std::string nickname = oldNick.empty() ? "*" : oldNick;
    _sendResponse(fd, ERR_NICKNAMEINUSE(nickname, newNick));
    return;
  }

  client->setNickname(newNick);

  if (client->isRegistered()) {
    const std::string ip = client->getIpAddress();
    const std::string nickMsg = ":" + oldNick + "!~" + client->getUsername() + "@" + ip + " NICK :" + newNick + "\r\n";

    _sendResponse(fd, nickMsg);

    // TODO: Broadcast new nickname to every channel.
  }

  std::cout << "FD " << fd << " set nickname to " << newNick << std::endl;

  _checkRegistration(client);
}

void Server::_handleUser(const int fd, const std::vector<std::string> &args) {
  Client *client = _clients[fd];

  if (!client->hasPassword()) {
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
  client->setHostname(args[2]);
  client->setServername(args[3]);
  client->setRealname(args[4]);

  std::cout << "FD " << fd << " set username to " << args[1] << ", hostname to " << args[2] << ", servername to "
            << args[3] << " and realname to " << args[4] << std::endl;

  _checkRegistration(client);
}
