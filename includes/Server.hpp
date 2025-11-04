#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <map>
#include <poll.h>
#include "Client.hpp"
#include "Channel.hpp"
#include "Message.hpp"

class Server
{
private:
    int _port;
    int _serverSocket;
    std::string _password;
    std::vector<struct pollfd> _pollfds;
    std::map<int, Client*> _clients;
    std::map<std::string, Channel*> _channels;
    bool _running;

public:
    Server(int port, const std::string& password);
    ~Server();
    
    void run();
    void stop();
    
    Client* getClientByNick(const std::string& nickname);
    Channel* getChannel(const std::string& name);
    Channel* createChannel(const std::string& name, Client* creator);
    void removeChannel(const std::string& name);
    bool isNicknameTaken(const std::string& nickname);
    std::string getPassword() const;
    void sendToChannel(const std::string& channelName, const std::string& message, Client* exclude);

private:
    void setupSocket();
    void acceptNewClient();
    void handleClient(int fd);
    void removeClient(int fd);
    void processMessage(Client* client, const std::string& message);
};

#endif