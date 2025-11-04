#include "../includes/Commands.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/Channel.hpp"

void handleTopic(Client* client, Server* server, const std::vector<std::string>& params)
{
    if (!client->isRegistered())
    {
        client->sendMessage(":server 451 " + client->getNickname() + " :You have not registered\r\n");
        return;
    }
    
    if (params.empty())
    {
        client->sendMessage(":server 461 " + client->getNickname() + " TOPIC :Not enough parameters\r\n");
        return;
    }
    
    std::string channelName = params[0];
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
    
    if (params.size() == 1)
    {
        if (channel->getTopic().empty())
        {
            client->sendMessage(":server 331 " + client->getNickname() + " " + channelName + " :No topic is set\r\n");
        }
        else
        {
            client->sendMessage(":server 332 " + client->getNickname() + " " + channelName + " :" + channel->getTopic() + "\r\n");
        }
        return;
    }
    
    if (channel->isTopicRestricted() && !channel->isOperator(client))
    {
        client->sendMessage(":server 482 " + client->getNickname() + " " + channelName + " :You're not channel operator\r\n");
        return;
    }
    
    std::string newTopic = params[1];
    channel->setTopic(newTopic);
    
    std::string topicMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@host TOPIC " + channelName + " :" + newTopic + "\r\n";
    channel->broadcast(topicMsg);
}