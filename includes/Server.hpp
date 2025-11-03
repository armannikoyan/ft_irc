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
    
private:
    void setupSocket();
    void acceptNewClient();
    void handleClient(int fd);
    void removeClient(int fd);
    
    void processMessage(Client* client, const std::string& message);
    void sendToClient(int fd, const std::string& message);
    
    Client* getClientByNick(const std::string& nickname);
    Channel* getChannel(const std::string& name);
    Channel* createChannel(const std::string& name);
    void deleteChannel(const std::string& name);
    
    void handlePass(Client* client, Message& msg);
    void handleNick(Client* client, Message& msg);
    void handleUser(Client* client, Message& msg);
    void handleJoin(Client* client, Message& msg);
    void handlePrivmsg(Client* client, Message& msg);
    void handleKick(Client* client, Message& msg);
    void handleInvite(Client* client, Message& msg);
    void handleTopic(Client* client, Message& msg);
    void handleMode(Client* client, Message& msg);
    void handleQuit(Client* client, Message& msg);
    void handlePing(Client* client, Message& msg);
};

#endif