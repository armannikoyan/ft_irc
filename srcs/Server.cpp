#include "Server.hpp"
#include "Commands.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <cerrno>

Server::Server(int port, const std::string& password)
    : _port(port), _serverSocket(-1), _password(password), _running(false) {}

Server::~Server()
{
    stop();
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        delete it->second;
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
        delete it->second;
    if (_serverSocket != -1)
        close(_serverSocket);
}

void Server::setupSocket()
{
    _serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocket < 0)
        throw std::runtime_error("Failed to create socket");

    int opt = 1;
    if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("Failed to set socket options");

    fcntl(_serverSocket, F_SETFL, O_NONBLOCK);

    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(_port);

    if (bind(_serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
        throw std::runtime_error("Failed to bind socket");

    if (listen(_serverSocket, 10) < 0)
        throw std::runtime_error("Failed to listen on socket");

    struct pollfd serverPollfd;
    serverPollfd.fd = _serverSocket;
    serverPollfd.events = POLLIN;
    serverPollfd.revents = 0;
    _pollfds.push_back(serverPollfd);
}

void Server::acceptNewClient()
{
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    int clientFd = accept(_serverSocket, (struct sockaddr*)&clientAddr, &clientLen);

    if (clientFd < 0)
        return;

    fcntl(clientFd, F_SETFL, O_NONBLOCK);

    Client* newClient = new Client(clientFd);
    _clients[clientFd] = newClient;

    struct pollfd clientPollfd;
    clientPollfd.fd = clientFd;
    clientPollfd.events = POLLIN;
    clientPollfd.revents = 0;
    _pollfds.push_back(clientPollfd);
}

void Server::handleClient(int fd)
{
    char buffer[512];
    std::memset(buffer, 0, sizeof(buffer));

    int bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);

    if (bytesRead <= 0)
    {
        removeClient(fd);
        return;
    }

    Client* client = _clients[fd];
    client->appendBuffer(std::string(buffer, bytesRead));
    for (int i = 0; i < bytesRead; i++) {
        if (buffer[i] == '\r') std::cout << "\\r";
        else if (buffer[i] == '\n') std::cout << "\\n";
        else std::cout << buffer[i];
    }
    std::cout << "]" << std::endl;

    std::string message;
    while (!(message = client->extractMessage()).empty())
    {
        processMessage(client, message);
    }
}

void Server::removeClient(int fd)
{
    std::map<int, Client*>::iterator it = _clients.find(fd);
    if (it == _clients.end())
        return;

    Client* client = it->second;

    for (std::map<std::string, Channel*>::iterator chIt = _channels.begin(); chIt != _channels.end(); )
    {
        chIt->second->removeClient(client);
        if (chIt->second->isEmpty())
        {
            delete chIt->second;
            _channels.erase(chIt++);
        }
        else
            ++chIt;
    }

    for (std::vector<struct pollfd>::iterator pIt = _pollfds.begin(); pIt != _pollfds.end(); ++pIt)
    {
        if (pIt->fd == fd)
        {
            _pollfds.erase(pIt);
            break;
        }
    }

    delete client;
    _clients.erase(fd);
    close(fd);
}

void Server::processMessage(Client* client, const std::string& messageStr)
{
    Message msg(messageStr);
    
    std::string command = msg.getCommand();
    std::cout << "Command: " << command << std::endl;
    std::vector<std::string> params = msg.getParameters();

    if (command == "PASS")
        handlePass(client, this, params);
    else if (command == "NICK")
        handleNick(client, this, params);
    else if (command == "USER")
        handleUser(client, params);
    else if (command == "JOIN")
        handleJoin(client, this, params);
    else if (command == "PART")
        handlePart(client, this, params);
    else if (command == "PRIVMSG")
        handlePrivmsg(client, this, params);
    else if (command == "NOTICE")
        handleNotice(client, this, params);
    else if (command == "KICK")
        handleKick(client, this, params);
    else if (command == "INVITE")
        handleInvite(client, this, params);
    else if (command == "TOPIC")
        handleTopic(client, this, params);
    else if (command == "MODE")
        handleMode(client, this, params);
    else if (command == "QUIT")
    {
        handleQuit(client, this, params);
        removeClient(client->getFd());
    }
    else if (command == "PING")
        handlePing(client, params);
    else if (command == "PONG")
        handlePong(client, params);
    else if (command == "CAP")
        handleCap(client, params);
    else
    {
        client->sendMessage(":server 421 " + client->getNickname() + " " + command + " :Unknown command\r\n");
    }
}

void Server::run()
{
    setupSocket();
    _running = true;

    while (_running)
    {
        int pollCount = poll(&_pollfds[0], _pollfds.size(), -1);

        if (pollCount < 0)
        {
            if (errno == EINTR)
                continue;
            throw std::runtime_error("Poll error");
        }

        for (size_t i = 0; i < _pollfds.size(); i++)
        {
            if (_pollfds[i].revents & POLLIN)
            {
                if (_pollfds[i].fd == _serverSocket)
                    acceptNewClient();
                else
                    handleClient(_pollfds[i].fd);
            }
        }
    }
}

void Server::stop()
{
    _running = false;
}

Client* Server::getClientByNick(const std::string& nickname)
{
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second->getNickname() == nickname)
            return it->second;
    }
    return NULL;
}

Channel* Server::getChannel(const std::string& name)
{
    std::map<std::string, Channel*>::iterator it = _channels.find(name);
    if (it != _channels.end())
        return it->second;
    return NULL;
}

Channel* Server::createChannel(const std::string& name, Client* creator)
{
    Channel* channel = new Channel(name);
    _channels[name] = channel;
    channel->addClient(creator);
    channel->addOperator(creator);
    return channel;
}

void Server::removeChannel(const std::string& name)
{
    std::map<std::string, Channel*>::iterator it = _channels.find(name);
    if (it != _channels.end())
    {
        delete it->second;
        _channels.erase(it);
    }
}

bool Server::isNicknameTaken(const std::string& nickname)
{
    return getClientByNick(nickname) != NULL;
}

std::string Server::getPassword() const
{
    return _password;
}

void Server::sendToChannel(const std::string& channelName, const std::string& message, Client* exclude)
{
    Channel* channel = getChannel(channelName);
    if (channel)
        channel->broadcast(message, exclude);
}