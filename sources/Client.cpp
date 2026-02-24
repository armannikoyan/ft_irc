#include <cerrno>
#include <stdexcept>
#include <unistd.h>

#include "Client.hpp"

Client::Client() :
    _fd(-1), _ipAddress(""), _readBuffer(""), _sendBuffer(""), _nickname(""), _username(""), _realname(""),
    _hasPassword(false), _isRegistered(false) {};

Client::Client(const int fd, const std::string &ipAddress) :
    _fd(fd), _ipAddress(ipAddress), _readBuffer(""), _sendBuffer(""), _nickname(""), _username(""), _realname(""),
    _hasPassword(false), _isRegistered(false) {};

Client::Client(const Client &other) { *this = other; }

Client &Client::operator=(const Client &other) {
  if (this == &other)
    return *this;

  if (close(_fd) == -1 && errno != EBADF)
    throw std::runtime_error("Error: close failed");

  _fd = other._fd;
  _ipAddress = other._ipAddress;
  _readBuffer = other._readBuffer;
  _sendBuffer = other._sendBuffer;
  _nickname = other._nickname;
  _username = other._username;
  _realname = other._realname;
  _hasPassword = other._hasPassword;
  _isRegistered = other._isRegistered;

  return *this;
}

Client::~Client() {}

int Client::getFd() const { return _fd; }

const std::string &Client::getIpAddress() const { return _ipAddress; }

const std::string &Client::getSendBuffer() const { return _sendBuffer; }

const std::string &Client::getNickname() const { return _nickname; }

const std::string &Client::getUsername() const { return _username; }

const std::string &Client::getHostname() const { return _hostname; }

const std::string &Client::getServername() const { return _servername; }

const std::string &Client::getRealname() const { return _realname; }

bool Client::hasPassword() const { return _hasPassword; }

bool Client::isRegistered() const { return _isRegistered; }

void Client::setNickname(const std::string &nickname) { _nickname = nickname; }

void Client::setUsername(const std::string &username) { _username = username; }

void Client::setHostname(const std::string &hostname) { _hostname = hostname; }

void Client::setServername(const std::string &servername) { _servername = servername; }

void Client::setRealname(const std::string &realname) { _realname = realname; }

void Client::setHasPassword(const bool hasPassword) { _hasPassword = hasPassword; }

void Client::setRegistered(const bool registered) { _isRegistered = registered; }

void Client::appendSendBuffer(const std::string &data) { _sendBuffer += data; }
void Client::appendReadBuffer(const std::string &data) { _readBuffer += data; }

bool Client::hasCompleteCommand() const { return _readBuffer.find('\n') != std::string::npos; }

std::string Client::getCompleteCommand() {
  const size_t pos = _readBuffer.find('\n');
  if (pos == std::string::npos)
    return "";

  std::string command = _readBuffer.substr(0, pos + 1);
  _readBuffer.erase(0, pos + 1);

  const size_t end = command.find_last_not_of("\r\n");
  return end != std::string::npos ? command.substr(0, end + 1) : "";
}

void Client::clearSentData(const size_t length) { _sendBuffer.erase(0, length); }
