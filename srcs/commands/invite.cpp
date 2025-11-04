#include "../includes/Commands.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/Channel.hpp"

void handleInvite(Client* client, Server* server, const std::vector<std::string>& params)
{
    if (!client->isRegistered())
    {
        client->sendMessage(":server 451 " + client->getNickname() + " :You have not registered\r\n");
        return;
    }
    
    if (params.size() < 2)
    {
        client->sendMessage(":server 461 " + client->getNickname() + " INVITE :Not enough parameters\r\n");
        return;
    }
    
    std::string targetNick = params[0];
    std::string channelName = params[1];
    
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
    
    if (channel->isInviteOnly() && !channel->isOperator(client))
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
    
    if (channel->hasClient(target))
    {
        client->sendMessage(":server 443 " + client->getNickname() + " " + targetNick + " " + channelName + " :is already on channel\r\n");
        return;
    }
    
    channel->addInvite(target);
    
    client->sendMessage(":server 341 " + client->getNickname() + " " + targetNick + " " + channelName + "\r\n");
    
    std::string inviteMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@host INVITE " + targetNick + " :" + channelName + "\r\n";
    target->sendMessage(inviteMsg);
}