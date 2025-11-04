#include "../includes/Commands.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"

bool isValidNickname(const std::string& nick)
{
    if (nick.empty() || nick.length() > 9)
        return false;

    unsigned char first = static_cast<unsigned char>(nick[0]);
    if (!((first >= 'A' && first <= 'Z') || (first >= 'a' && first <= 'z')))
        return false;

    for (size_t i = 1; i < nick.length(); ++i)
    {
        unsigned char c = static_cast<unsigned char>(nick[i]);
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '-' || c == '[' || c == ']' ||
              c == '\\' || c == '`' ||
              c == '^' || c == '{' || c == '}'))
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

    if (!oldNick.empty())
    {
        client->sendMessage(":" + oldNick + " NICK :" + newNick + "\r\n");
    }

    if (!client->isRegistered() && !client->getNickname().empty() && !client->getUsername().empty())
    {
        client->setRegistered(true);
        client->sendMessage(":server 001 " + client->getNickname() + " :Welcome to the IRC Network\r\n");
    }
}
