NAME = ircserv
BONUS_NAME = ircbot
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98

# Directories
SRCS_DIR = srcs
OBJS_DIR = objs
INCLUDES_DIR = includes
CMDS_DIR = $(SRCS_DIR)/commands
BOT_DIR = bot

# Source files
SRCS = $(SRCS_DIR)/main.cpp \
       $(SRCS_DIR)/Server.cpp \
       $(SRCS_DIR)/Client.cpp \
       $(SRCS_DIR)/Channel.cpp \
       $(SRCS_DIR)/Message.cpp \
       $(CMDS_DIR)/authenticate.cpp \
       $(CMDS_DIR)/nick.cpp \
       $(CMDS_DIR)/user.cpp \
       $(CMDS_DIR)/join.cpp \
       $(CMDS_DIR)/part.cpp \
       $(CMDS_DIR)/privmsg.cpp \
       $(CMDS_DIR)/kick.cpp \
       $(CMDS_DIR)/invite.cpp \
       $(CMDS_DIR)/topic.cpp \
       $(CMDS_DIR)/mode.cpp \
       $(CMDS_DIR)/quit.cpp \
       $(CMDS_DIR)/ping.cpp \
       $(CMDS_DIR)/utils.cpp \
       $(CMDS_DIR)/pong.cpp \
       $(CMDS_DIR)/notice.cpp

# Bonus source files
BONUS_SRCS = $(SRCS_DIR)/Server.cpp \
             $(SRCS_DIR)/Client.cpp \
             $(SRCS_DIR)/Channel.cpp \
             $(SRCS_DIR)/Message.cpp \
             $(SRCS_DIR)/FileTransfer.cpp \
             $(CMDS_DIR)/authenticate.cpp \
             $(CMDS_DIR)/nick.cpp \
             $(CMDS_DIR)/user.cpp \
             $(CMDS_DIR)/join.cpp \
             $(CMDS_DIR)/part.cpp \
             $(CMDS_DIR)/privmsg.cpp \
             $(CMDS_DIR)/kick.cpp \
             $(CMDS_DIR)/invite.cpp \
             $(CMDS_DIR)/topic.cpp \
             $(CMDS_DIR)/mode.cpp \
             $(CMDS_DIR)/quit.cpp \
             $(CMDS_DIR)/ping.cpp \
             $(CMDS_DIR)/dcc.cpp \
             $(CMDS_DIR)/utils.cpp \
             $(BOT_DIR)/bot_main.cpp \
             $(BOT_DIR)/BotCommands.cpp

OBJS = $(SRCS:.cpp=.o)
BONUS_OBJS = $(BONUS_SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

bonus: $(BONUS_NAME)

$(BONUS_NAME): $(BONUS_OBJS)
	$(CC) $(CFLAGS) $(BONUS_OBJS) -o $(BONUS_NAME)

%.o: %.cpp
	$(CC) $(CFLAGS) -I$(INCLUDES_DIR) -c $< -o $@

clean:
	rm -f $(OBJS) $(BONUS_OBJS)

fclean: clean
	rm -f $(NAME) $(BONUS_NAME)

re: fclean all

.PHONY: all bonus clean fclean re