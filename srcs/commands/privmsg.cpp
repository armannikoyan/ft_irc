#include "../includes/Commands.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include <sys/socket.h>

void handlePrivmsg(Client* client, Server* server, const std::vector<std::string>& params) {
    if (!client->isRegistered()) {
        std::string response = ":server 451 * :You have not registered\r\n";
        send(client->getFd(), response.c_str(), response.length(), 0);
        return;
    }
    
    if (params.size() < 2) {
        if (params.empty()) {
            std::string response = ":server 411 " + client->getNickname() + " :No recipient given (PRIVMSG)\r\n";
            send(client->getFd(), response.c_str(), response.length(), 0);
        } else {
            std::string response = ":server 412 " + client->getNickname() + " :No text to send\r\n";
            send(client->getFd(), response.c_str(), response.length(), 0);
        }
        return;
    }
    
    std::string target = params[0];
    std::string message = params[1];
    
    std::string fullMessage = ":" + client->getNickname() + "!" + client->getUsername() + "@host PRIVMSG " + target + " :" + message + "\r\n";
    
    if (target[0] == '#') {
        server->sendToChannel(target, fullMessage, client);
    } else {
        Client* recipient = server->getClientByNick(target);
        if (recipient) {
            send(recipient->getFd(), fullMessage.c_str(), fullMessage.length(), 0);
        } else {
            std::string response = ":server 401 " + client->getNickname() + " " + target + " :No such nick/channel\r\n";
            send(client->getFd(), response.c_str(), response.length(), 0);
        }
    }
}