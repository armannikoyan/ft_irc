#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>

class Client
{
private:
    int _fd;
    std::string _nickname;
    std::string _username;
    std::string _realname;
    std::string _hostname;
    std::string _buffer;
    bool _authenticated;
    bool _registered;
    bool _disconnected;
    std::vector<std::string> _channels;

public:
    Client(int fd);
    ~Client();
    
    int getFd() const;
    std::string getNickname() const;
    std::string getUsername() const;
    bool isAuthenticated() const;
    bool isRegistered() const;

    void setNickname(const std::string& nickname);
    void setUsername(const std::string& username);
    void setRealname(const std::string& realname);
    void setAuthenticated(bool auth);
    void setRegistered(bool reg);

    void appendBuffer(const std::string& data);
    std::string extractMessage();

    void sendMessage(const std::string& msg);

    const std::vector<std::string>& getChannels() const;
    void addChannel(const std::string& channel);
    void removeChannel(const std::string& channel);

    const std::string& getHostname() const;
    void setHostname(const std::string& hostname);

    void setDisconnected(bool state);
    bool isDisconnected() const;
};

#endif
