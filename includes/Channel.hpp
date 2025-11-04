#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include "Client.hpp"

class Channel
{
private:
    std::string _name;
    std::string _topic;
    std::string _key;
    std::vector<Client*> _clients;
    std::vector<Client*> _operators;
    std::vector<Client*> _invited;
    bool _inviteOnly;
    bool _topicRestricted;
    int _userLimit;

public:
    Channel(const std::string& name);
    ~Channel();
    
    std::string getName() const;
    std::string getTopic() const;
    std::string getKey() const;
    bool isInviteOnly() const;
    bool isTopicRestricted() const;
    int getUserLimit() const;
    
    void setTopic(const std::string& topic);
    void setKey(const std::string& key);
    void setInviteOnly(bool value);
    void setTopicRestricted(bool value);
    void setUserLimit(int limit);
    
    void addClient(Client* client);
    void removeClient(Client* client);
    void addOperator(Client* client);
    void removeOperator(Client* client);
    void addInvited(Client* client);
    
    bool isOperator(Client* client) const;
    bool isInvited(Client* client) const;
    bool isMember(Client* client) const;
    bool isFull() const;
    
    std::vector<Client*> getClients() const;
    void broadcast(const std::string& message, Client* exclude = NULL);
    
    bool hasKey() const;
    bool hasUserLimit() const;
    void removeKey();
    void removeUserLimit();
    bool hasClient(Client* client) const;
    size_t getUserCount() const;
    bool isEmpty() const;
    void addInvite(Client* client);
    std::string getModes() const;
    std::string getNamesList() const;
};

#endif