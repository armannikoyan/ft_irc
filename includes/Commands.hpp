#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>
#include <vector>

class Client;
class Server;

void handlePass(Client* client, Server* server, const std::vector<std::string>& params);
void handleNick(Client* client, Server* server, const std::vector<std::string>& params);
void handleUser(Client* client, const std::vector<std::string>& params);
void handlePrivmsg(Client* client, Server* server, const std::vector<std::string>& params);
void handleNotice(Client* client, Server* server, const std::vector<std::string>& params);
void handleJoin(Client* client, Server* server, const std::vector<std::string>& params);
void handlePart(Client* client, Server* server, const std::vector<std::string>& params);
void handlePing(Client* client, const std::vector<std::string>& params);
void handlePong(Client* client, const std::vector<std::string>& params);
void handleQuit(Client* client, Server* server, const std::vector<std::string>& params);
void handleKick(Client* client, Server* server, const std::vector<std::string>& params);
void handleInvite(Client* client, Server* server, const std::vector<std::string>& params);
void handleTopic(Client* client, Server* server, const std::vector<std::string>& params);
void handleMode(Client* client, Server* server, const std::vector<std::string>& params);
void handleCap(Client* client, const std::vector<std::string>& params);

#endif