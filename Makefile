NAME        = ircserv
BOT_NAME    = bot
CXX         = c++
CXXFLAGS    = -Iincludes -Wall -Wextra -Werror -std=c++98
SRCDIR      = sources
OBJDIR      = objects

SRC_FILES   = main.cpp Server.cpp Client.cpp Channel.cpp

SRCS        = $(addprefix $(SRCDIR)/, $(SRC_FILES))
OBJS        = $(addprefix $(OBJDIR)/, $(SRC_FILES:.cpp=.o))

BOT_SRC     = $(SRCDIR)/bot/Bot.cpp
BOT_OBJ     = $(OBJDIR)/bot/Bot.o

all: $(NAME)

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

bot: $(BOT_NAME)

$(BOT_NAME): $(BOT_OBJ)
	@$(CXX) $(CXXFLAGS) $(BOT_OBJ) -o $(BOT_NAME)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR) $(OBJDIR)/bot
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/bot:
	@mkdir -p $(OBJDIR)/bot

debug: CXXFLAGS += -O0 -g
debug: re

clean:
	@rm -rf $(OBJDIR)

fclean: clean
	@rm -f $(NAME) $(BOT_NAME)

re: fclean all

.PHONY: all debug clean fclean re bot