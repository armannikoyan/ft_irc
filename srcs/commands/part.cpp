#include "../includes/Commands.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/Channel.hpp"

void handlePart(Client* client, Server* server, const std::vector<std::string>& params)
{
    if (!client->isRegistered())
    {
        client->sendMessage(":server 451 " + client->getNickname() + " :You have not registered\r\n");
        return;
    }
    
    if (params.empty())
    {
        client->sendMessage(":server 461 " + client->getNickname() + " PART :Not enough parameters\r\n");
        return;
    }
    
    std::string channelName = params[0];
    std::string reason = "";
    
    if (params.size() > 1)
    {
        reason = params[1];
    }
    
    Channel* channel = server->getChannel(channelName);
    
    if (!channel)
    {
        client->sendMessage(":server 403 " + client->getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }
    
    if (!channel->hasClient(client))
    {
        client->sendMessage(":server 442 " + client->getNickname() + " " + channelName + " :You're not on that channel\r\n");
        return;
    }
    
    std::string partMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@host PART " + channelName;
    if (!reason.empty())
    {
        partMsg += " :" + reason;
    }
    partMsg += "\r\n";
    
    channel->broadcast(partMsg);
    channel->removeClient(client);
    
    if (channel->isEmpty())
    {
        server->removeChannel(channelName);
    }
}