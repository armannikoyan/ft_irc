#include "../includes/Commands.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/Channel.hpp"

void handleKick(Client* client, Server* server, const std::vector<std::string>& params)
{
    if (!client->isRegistered())
    {
        client->sendMessage(":server 451 " + client->getNickname() + " :You have not registered\r\n");
        return;
    }
    
    if (params.size() < 2)
    {
        client->sendMessage(":server 461 " + client->getNickname() + " KICK :Not enough parameters\r\n");
        return;
    }
    
    std::string channelName = params[0];
    std::string targetNick = params[1];
    std::string reason = "No reason given";
    
    if (params.size() > 2)
    {
        reason = params[2];
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
    
    if (!channel->isOperator(client))
    {
        client->sendMessage(":server 482 " + client->getNickname() + " " + channelName + " :You're not channel operator\r\n");
        return;
    }
    
    Client* target = server->getClientByNick(targetNick);
    
    if (!target)
    {
        client->sendMessage(":server 401 " + client->getNickname() + " " + targetNick + " :No such nick/channel\r\n");
        return;
    }
    
    if (!channel->hasClient(target))
    {
        client->sendMessage(":server 441 " + client->getNickname() + " " + targetNick + " " + channelName + " :They aren't on that channel\r\n");
        return;
    }
    
    std::string kickMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@host KICK " + channelName + " " + targetNick + " :" + reason + "\r\n";
    channel->broadcast(kickMsg);
    
    channel->removeClient(target);
}