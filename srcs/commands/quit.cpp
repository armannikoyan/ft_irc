#include "../includes/Commands.hpp"
#include "../includes/Client.hpp"
#include "../includes/Server.hpp"

void handleQuit(Client* client, Server* server, const std::vector<std::string>& params)
{
    (void)server;

    std::string quitMsg = "Client quit";
    if (!params.empty())
        quitMsg = params[0];
    
    client->sendMessage("ERROR :Closing connection (" + quitMsg + ")\r\n");
    
    const std::vector<std::string>& channels = client->getChannels();
    
    for (size_t i = 0; i < channels.size(); ++i)
    {
        std::string notification = ":" + client->getNickname() + "!" + 
                                   client->getUsername() + "@" + 
                                   client->getHostname() + 
                                   " QUIT :" + quitMsg + "\r\n";
    }

    client->setDisconnected(true);
}
