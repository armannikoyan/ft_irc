#include "../includes/Commands.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"

void handleNotice(Client* client, Server* server, const std::vector<std::string>& params)
{
    if (!client->isRegistered())
    {
        return;
    }
    
    if (params.size() < 2)
    {
        return;
    }
    
    std::string target = params[0];
    std::string message = params[1];
    
    std::string fullMessage = ":" + client->getNickname() + "!" + client->getUsername() + "@host NOTICE " + target + " :" + message + "\r\n";
    
    if (target[0] == '#')
    {
        server->sendToChannel(target, fullMessage, client);
    }
    else
    {
        Client* recipient = server->getClientByNick(target);
        if (recipient)
        {
            recipient->sendMessage(fullMessage);
        }
    }
}