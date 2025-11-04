#include "../includes/Commands.hpp"
#include "../includes/Client.hpp"
#include <sys/socket.h>

void handleUser(Client* client, const std::vector<std::string>& params) {
    if (client->isRegistered()) {
        client->sendMessage(":server 462 " + client->getNickname() + " :You may not reregister\r\n");
        return;
    }
    
    if (params.size() < 4) {
        client->sendMessage(":server 461 * USER :Not enough parameters\r\n");
        return;
    }
    
    client->setUsername(params[0]);
    client->setRealname(params[3]);
    
    if (client->isAuthenticated() && !client->getNickname().empty()) {
        client->setRegistered(true);
        client->sendMessage(":server 001 " + client->getNickname() + " :Welcome to the IRC Network\r\n");
    }
}