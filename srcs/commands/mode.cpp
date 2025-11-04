#include "../includes/Commands.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/Channel.hpp"

void handleMode(Client* client, Server* server, const std::vector<std::string>& params)
{
    if (!client->isRegistered())
    {
        client->sendMessage(":server 451 " + client->getNickname() + " :You have not registered\r\n");
        return;
    }
    
    if (params.empty())
    {
        client->sendMessage(":server 461 " + client->getNickname() + " MODE :Not enough parameters\r\n");
        return;
    }
    
    std::string channelName = params[0];
    Channel* channel = server->getChannel(channelName);
    
    if (!channel)
    {
        client->sendMessage(":server 403 " + client->getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }
    
    if (params.size() == 1)
    {
        std::string modes = channel->getModes();
        client->sendMessage(":server 324 " + client->getNickname() + " " + channelName + " +" + modes + "\r\n");
        return;
    }
    
    if (!channel->isOperator(client))
    {
        client->sendMessage(":server 482 " + client->getNickname() + " " + channelName + " :You're not channel operator\r\n");
        return;
    }
    
    std::string modeStr = params[1];
    bool adding = true;
    size_t paramIndex = 2;
    
    for (size_t i = 0; i < modeStr.length(); i++)
    {
        char mode = modeStr[i];
        
        if (mode == '+')
        {
            adding = true;
            continue;
        }
        else if (mode == '-')
        {
            adding = false;
            continue;
        }
        
        if (mode == 'i')
        {
            channel->setInviteOnly(adding);
            std::string modeMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@host MODE " + channelName + " " + (adding ? "+" : "-") + "i\r\n";
            channel->broadcast(modeMsg);
        }
        else if (mode == 't')
        {
            channel->setTopicRestricted(adding);
            std::string modeMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@host MODE " + channelName + " " + (adding ? "+" : "-") + "t\r\n";
            channel->broadcast(modeMsg);
        }
        else if (mode == 'k')
        {
            if (paramIndex >= params.size())
            {
                client->sendMessage(":server 461 " + client->getNickname() + " MODE :Not enough parameters\r\n");
                return;
            }
            std::string key = params[paramIndex++];
            if (adding)
            {
                channel->setKey(key);
            }
            else
            {
                channel->removeKey();
            }
            std::string modeMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@host MODE " + channelName + " " + (adding ? "+" : "-") + "k";
            if (adding)
            {
                modeMsg += " " + key;
            }
            modeMsg += "\r\n";
            channel->broadcast(modeMsg);
        }
        else if (mode == 'o')
        {
            if (paramIndex >= params.size())
            {
                client->sendMessage(":server 461 " + client->getNickname() + " MODE :Not enough parameters\r\n");
                return;
            }
            std::string targetNick = params[paramIndex++];
            Client* target = server->getClientByNick(targetNick);
            
            if (!target)
            {
                client->sendMessage(":server 401 " + client->getNickname() + " " + targetNick + " :No such nick/channel\r\n");
                continue;
            }
            
            if (!channel->hasClient(target))
            {
                client->sendMessage(":server 441 " + client->getNickname() + " " + targetNick + " " + channelName + " :They aren't on that channel\r\n");
                continue;
            }
            
            if (adding)
            {
                channel->addOperator(target);
            }
            else
            {
                channel->removeOperator(target);
            }
            
            std::string modeMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@host MODE " + channelName + " " + (adding ? "+" : "-") + "o " + targetNick + "\r\n";
            channel->broadcast(modeMsg);
        }
        else if (mode == 'l')
        {
            if (adding)
            {
                if (paramIndex >= params.size())
                {
                    client->sendMessage(":server 461 " + client->getNickname() + " MODE :Not enough parameters\r\n");
                    return;
                }
                std::string limitStr = params[paramIndex++];
                int limit = atoi(limitStr.c_str());
                channel->setUserLimit(limit);
                std::string modeMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@host MODE " + channelName + " +l " + limitStr + "\r\n";
                channel->broadcast(modeMsg);
            }
            else
            {
                channel->removeUserLimit();
                std::string modeMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@host MODE " + channelName + " -l\r\n";
                channel->broadcast(modeMsg);
            }
        }
        else
        {
            client->sendMessage(":server 472 " + client->getNickname() + " " + std::string(1, mode) + " :is unknown mode char to me\r\n");
        }
    }
}