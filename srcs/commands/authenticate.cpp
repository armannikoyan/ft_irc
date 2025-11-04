#include "../includes/Commands.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include <sys/socket.h>

void handlePass(Client* client, Server* server, const std::vector<std::string>& params) {
    if (client->isRegistered()) {
        client->sendMessage(":server 462 * :You may not reregister\r\n");
        return;
    }
    
    if (params.empty()) {
        client->sendMessage(":server 461 * PASS :Not enough parameters\r\n");
        return;
    }
    
    if (params[0] == server->getPassword()) {
        client->setAuthenticated(true);
    } else {
        client->sendMessage(":server 464 * :Password incorrect\r\n");
        client->setAuthenticated(false);
    }
}