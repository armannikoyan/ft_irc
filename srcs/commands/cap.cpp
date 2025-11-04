#include "../includes/Commands.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/Channel.hpp"

void handleCap(Client* client, const std::vector<std::string>& params)
{
    if (params.empty())
        return;

    std::string subcommand = params[0];

    if (subcommand == "LS")
    {
        client->sendMessage(":server CAP * LS :\r\n");
    }
    else if (subcommand == "REQ")
    {
        client->sendMessage(":server CAP * NAK :" + (params.size() > 1 ? params[1] : "") + "\r\n");
    }
    else if (subcommand == "END")
    {
        client->sendMessage(":server CAP * ACK :\r\n");
    }
    else
    {
        client->sendMessage(":server 410 " + client->getNickname() + " CAP :Invalid CAP command\r\n");
    }
}
