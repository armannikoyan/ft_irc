#ifndef IRCDEFINE_HPP
#define IRCDEFINE_HPP

#include <climits>

#define SERV_NAME std::string("ircserv")

#define IRC_ARG_COUNT 3

#define MIN_PORT 1024
#define MAX_PORT SHRT_MAX

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

#endif
