#include "../includes/Commands.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/Channel.hpp"

void handleJoin(Client* client, Server* server, const std::vector<std::string>& params)
{
    if (!client->isRegistered())
    {
        client->sendMessage(":server 451 " + client->getNickname() + " :You have not registered\r\n");
        return;
    }
    
    if (params.empty())
    {
        client->sendMessage(":server 461 " + client->getNickname() + " JOIN :Not enough parameters\r\n");
        return;
    }
    
    std::string channelName = params[0];
    std::string key = "";
    
    if (params.size() > 1)
    {
        key = params[1];
    }
    
    if (channelName.empty() || (channelName[0] != '#' && channelName[0] != '&'))
    {
        client->sendMessage(":server 403 " + client->getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }
    
    Channel* channel = server->getChannel(channelName);
    
    if (!channel)
    {
        channel = server->createChannel(channelName, client);
        if (!channel)
        {
            client->sendMessage(":server 403 " + client->getNickname() + " " + channelName + " :Cannot create channel\r\n");
            return;
        }
    }
    
    if (channel->hasClient(client))
    {
        return;
    }
    
    if (channel->isInviteOnly() && !channel->isInvited(client))
    {
        client->sendMessage(":server 473 " + client->getNickname() + " " + channelName + " :Cannot join channel (+i)\r\n");
        return;
    }
    
    if (channel->hasKey() && channel->getKey() != key)
    {
        client->sendMessage(":server 475 " + client->getNickname() + " " + channelName + " :Cannot join channel (+k)\r\n");
        return;
    }
    
    if (channel->hasUserLimit() && channel->getUserCount() >= static_cast<size_t>(channel->getUserLimit()))
    {
        client->sendMessage(":server 471 " + client->getNickname() + " " + channelName + " :Cannot join channel (+l)\r\n");
        return;
    }
    
    channel->addClient(client);
    
    std::string joinMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@host JOIN :" + channelName + "\r\n";
    channel->broadcast(joinMsg);
    
    if (!channel->getTopic().empty())
    {
        client->sendMessage(":server 332 " + client->getNickname() + " " + channelName + " :" + channel->getTopic() + "\r\n");
    }
    
    std::string namesList = channel->getNamesList();
    client->sendMessage(":server 353 " + client->getNickname() + " = " + channelName + " :" + namesList + "\r\n");
    client->sendMessage(":server 366 " + client->getNickname() + " " + channelName + " :End of /NAMES list\r\n");
}