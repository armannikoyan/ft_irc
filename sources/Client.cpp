#include <ctime>

#include "Client.hpp"

Client::Client(const int fd, const std::string &ipAddress) :
    _fd(fd), _ipAddress(ipAddress), _hasPassword(false), _isRegistered(false), _pingSent(false),
    _isDisconnecting(false) {
  _lastActivityTime = time(NULL);
};

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

bool Client::isDisconnecting() const { return _isDisconnecting; }

void Client::setNickname(const std::string &nickname) { _nickname = nickname; }

void Client::setUsername(const std::string &username) { _username = username; }

void Client::setHostname(const std::string &hostname) { _hostname = hostname; }

void Client::setServername(const std::string &servername) { _servername = servername; }

void Client::setRealname(const std::string &realname) { _realname = realname; }

void Client::setHasPassword(const bool hasPassword) { _hasPassword = hasPassword; }

void Client::setRegistered(const bool registered) { _isRegistered = registered; }

void Client::setDisconnecting(const bool disconnecting) { _isDisconnecting = disconnecting; }

void Client::appendSendBuffer(const std::string &data) { _sendBuffer += data; }
void Client::appendReadBuffer(const std::string &data) { _readBuffer += data; }

bool Client::isBufferViolatingRFC() const {
  const size_t pos = _readBuffer.find('\n');

  if (pos == std::string::npos)
    return _readBuffer.length() > 512;

  return pos >= 512;
}

bool Client::hasCompleteCommand() const { return _readBuffer.find('\n') != std::string::npos; }

std::string Client::getCompleteCommand() {
  const size_t pos = _readBuffer.find('\n');
  if (pos == std::string::npos)
    return "";

  const std::string command = _readBuffer.substr(0, pos + 1);
  _readBuffer.erase(0, pos + 1);

  const size_t end = command.find_last_not_of("\r\n");
  return end != std::string::npos ? command.substr(0, end + 1) : "";
}

void Client::clearSentData(const size_t length) { _sendBuffer.erase(0, length); }

time_t Client::getLastActivityTime() const { return _lastActivityTime; }

void Client::updateActivityTime() { _lastActivityTime = time(NULL); }

bool Client::isPingSent() const { return _pingSent; }

void Client::setPingSent(const bool sent) { _pingSent = sent; }
