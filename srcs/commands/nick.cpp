#include "../includes/Commands.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"

bool isValidNickname(const std::string& nick)
{
    if (nick.empty() || nick.length() > 9)
    {
        return false;
    }
    
    if (!isalpha(nick[0]))
    {
        return false;
    }
    
    for (size_t i = 0; i < nick.length(); i++)
    {
        char c = nick[i];
        if (!isalnum(c) && c != '-' && c != '_' && c != '[' && c != ']' && 
            c != '{' && c != '}' && c != '\\' && c != '|')
        {
            return false;
        }
    }
    return true;
}

void handleNick(Client* client, Server* server, const std::vector<std::string>& params)
{
    if (params.empty())
    {
        client->sendMessage(":server 431 * :No nickname given\r\n");
        return;
    }
    
    std::string newNick = params[0];
    
    if (!isValidNickname(newNick))
    {
        client->sendMessage(":server 432 * " + newNick + " :Erroneous nickname\r\n");
        return;
    }
    
    if (server->isNicknameTaken(newNick))
    {
        client->sendMessage(":server 433 * " + newNick + " :Nickname is already in use\r\n");
        return;
    }
    
    std::string oldNick = client->getNickname();
    client->setNickname(newNick);
    
    if (oldNick.empty())
    {
        if (client->isAuthenticated() && !client->getUsername().empty()) {
            client->setRegistered(true);
            client->sendMessage(":server 001 " + newNick + " :Welcome to the IRC Network\r\n");
        }
    }
    else
    {
        client->sendMessage(":" + oldNick + " NICK :" + newNick + "\r\n");
    }
}