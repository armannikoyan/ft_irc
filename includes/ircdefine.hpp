#ifndef IRCDEFINE_HPP
#define IRCDEFINE_HPP

#include <climits>

#define SERV_NAME std::string("ircserv")

#define IRC_ARG_COUNT 3

#define MIN_PORT 1024
#define MAX_PORT SHRT_MAX

// #ifndef MSG_NOSIGNAL
// #define MSG_NOSIGNAL 0
// #endif

#define PING_TIMEOUT 60
#define DEAD_TIMEOUT 120

#define SPACES " \t\n\v\f\r"

#define RPL_WELCOME(nickname, username, hostname)                                                                      \
  (":" + SERV_NAME + " 001 " + nickname + " :Welcome to the " + SERV_NAME + " Network, " + nickname + "!" + username + \
   "@" + hostname + "\r\n")

#define ERR_NONICKNAMEGIVEN(nickname) (":" + SERV_NAME + " 431 " + nickname + " :No nickname given\r\n")

#define ERR_NICKNAMEINUSE(nickname, requested_nickname)                                                                \
  (":" + SERV_NAME + " 433 " + nickname + " " + requested_nickname + " :Nickname is already in use\r\n")

#define ERR_NEEDMOREPARAMS(nickname, command)                                                                          \
  (":" + SERV_NAME + " 461 " + nickname + " " + command + " :Not enough parameters\r\n")

#define ERR_ALREADYREGISTRED(nickname) (":" + SERV_NAME + " 462 " + nickname + " :You may not reregister\r\n")

#define ERR_PASSWDMISMATCH(nickname) (":" + SERV_NAME + " 464 " + nickname + " :Password incorrect\r\n")

#define ERR_NOTREGISTERED(nickname) (":" + SERV_NAME + " 451 " + nickname + " :You have not registered\r\n")

#define RPL_NOTOPIC(nickname, channel) (":" + SERV_NAME + " 331 " + nickname + " " + channel + " :No topic is set\r\n")

#define RPL_TOPIC(nickname, channel, topic)                                                                            \
  (":" + SERV_NAME + " 332 " + nickname + " " + channel + " :" + topic + "\r\n")

#define RPL_NAMREPLY(nickname, channel, clients_list)                                                                  \
  (":" + SERV_NAME + " 353 " + nickname + " = " + channel + " :" + clients_list + "\r\n")

#define RPL_ENDOFNAMES(nickname, channel)                                                                              \
  (":" + SERV_NAME + " 366 " + nickname + " " + channel + " :End of /NAMES list.\r\n")

#define ERR_NOSUCHCHANNEL(nickname, channel)                                                                           \
  (":" + SERV_NAME + " 403 " + nickname + " " + channel + " :No such channel\r\n")

#define ERR_NORECIPIENT(nickname, command)                                                                             \
  (":" + SERV_NAME + " 411 " + nickname + " :No recipient given (" + command + ")\r\n")

#define ERR_NOTEXTTOSEND(nickname) (":" + SERV_NAME + " 412 " + nickname + " :No text to send\r\n")

#define ERR_NOSUCHNICK(nickname, target)                                                                               \
  (":" + SERV_NAME + " 401 " + nickname + " " + target + " :No such nick/channel\r\n")

#define ERR_CANNOTSENDTOCHAN(nickname, channel)                                                                        \
  (":" + SERV_NAME + " 404 " + nickname + " " + channel + " :Cannot send to channel\r\n")

#define ERR_NOTONCHANNEL(nickname, channel)                                                                            \
  (":" + SERV_NAME + " 442 " + nickname + " " + channel + " :You're not on that channel\r\n")

#define ERR_CHANOPRIVSNEEDED(nickname, channel)                                                                        \
  (":" + SERV_NAME + " 482 " + nickname + " " + channel + " :You're not channel operator\r\n")

#define ERR_USERNOTINCHANNEL(nickname, target, channel)                                                                \
  (":" + SERV_NAME + " 441 " + nickname + " " + target + " " + channel + " :They aren't on that channel\r\n")

#define RPL_INVITING(nickname, target, channel)                                                                        \
  (":" + SERV_NAME + " 341 " + nickname + " " + target + " " + channel + "\r\n")

#define ERR_USERONCHANNEL(nickname, target, channel)                                                                   \
  (":" + SERV_NAME + " 443 " + nickname + " " + target + " " + channel + " :is already on channel\r\n")

#define ERR_ERRONEUSNICKNAME(nickname, requested_nickname)                                                             \
  (":" + SERV_NAME + " 432 " + nickname + " " + requested_nickname + " :Erroneous nickname\r\n")

#define ERR_NOORIGIN(nickname) (":" + SERV_NAME + " 409 " + nickname + " :No origin specified\r\n")

#define RPL_CHANNELMODEIS(nickname, channel, mode, mode_params)                                                        \
  (":" + SERV_NAME + " 324 " + nickname + " " + channel + " " + mode +                                                 \
   (mode_params.empty() ? "" : " " + mode_params) + "\r\n")

#define ERR_KEYSET(nickname, channel)                                                                                  \
  (":" + SERV_NAME + " 467 " + nickname + " " + channel + " :Channel key already set\r\n")

#define ERR_UNKNOWNMODE(nickname, modechar)                                                                            \
  (":" + SERV_NAME + " 472 " + nickname + " " + modechar + " :is unknown mode char to me\r\n")

#define ERR_CHANNELISFULL(nickname, channel)                                                                           \
  (":" + SERV_NAME + " 471 " + nickname + " " + channel + " :Cannot join channel (+l)\r\n")

#define ERR_INVITEONLYCHAN(nickname, channel)                                                                          \
  (":" + SERV_NAME + " 473 " + nickname + " " + channel + " :Cannot join channel (+i)\r\n")

#define ERR_BADCHANNELKEY(nickname, channel)                                                                           \
  (":" + SERV_NAME + " 475 " + nickname + " " + channel + " :Cannot join channel (+k)\r\n")

#define ERR_UMODEUNKNOWNFLAG(nickname) (":" + SERV_NAME + " 501 " + nickname + " :Unknown MODE flag\r\n")

#define ERR_USERSDONTMATCH(nickname) (":" + SERV_NAME + " 502 " + nickname + " :Cannot change mode for other users\r\n")

#define RPL_UMODEIS(nickname, modes) (":" + SERV_NAME + " 221 " + nickname + " " + modes + "\r\n")

#endif
