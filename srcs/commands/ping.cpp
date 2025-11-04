#include "../includes/Commands.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"

void handlePing(Client* client, const std::vector<std::string>& params)
{
    if (!client->isRegistered())
    {
        client->sendMessage(":server 451 * :You have not registered\r\n");
        return;
    }

    if (params.empty())
    {
        client->sendMessage(":server 461 " + client->getNickname() + " PING :Not enough parameters\r\n");
        return;
    }

    std::string response = ":server PONG server :" + params[0] + "\r\n";
    client->sendMessage(response);
}