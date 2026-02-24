#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client {
  private:
  int _fd;
  std::string _ipAddress;

  std::string _readBuffer;
  std::string _sendBuffer;

  std::string _nickname;
  std::string _username;
  std::string _hostname;
  std::string _servername;
  std::string _realname;

  bool _hasPassword;
  bool _isRegistered;

  Client();
  Client(const Client &);
  Client &operator=(const Client &);

  public:
  Client(int, const std::string &);
  ~Client();

  int getFd() const;
  const std::string &getIpAddress() const;
  const std::string &getSendBuffer() const;
  const std::string &getNickname() const;
  const std::string &getUsername() const;
  const std::string &getHostname() const;
  const std::string &getServername() const;
  const std::string &getRealname() const;
  bool hasPassword() const;
  bool isRegistered() const;

  void setNickname(const std::string &);
  void setUsername(const std::string &);
  void setHostname(const std::string &);
  void setServername(const std::string &);
  void setRealname(const std::string &);
  void setHasPassword(bool);
  void setRegistered(bool);


  void appendSendBuffer(const std::string &);
  void appendReadBuffer(const std::string &);

  bool hasCompleteCommand() const;
  std::string getCompleteCommand();

  void clearSentData(size_t length);
};

#endif
