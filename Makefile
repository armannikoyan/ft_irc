NAME        = ircserv
CXX         = c++
CXXFLAGS    = -Iincludes -Wall -Wextra -Werror -std=c++98
SRCDIR      = sources
OBJDIR      = objects

SRC_FILES   = main.cpp Server.cpp Client.cpp

SRCS        = $(addprefix $(SRCDIR)/, $(SRC_FILES))
OBJS        = $(addprefix $(OBJDIR)/, $(SRC_FILES:.cpp=.o))

all: $(NAME)

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	@mkdir -p $(OBJDIR)

debug: CXXFLAGS += -O0 -g
debug: re

clean:
	@rm -rf $(OBJDIR)

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all debug clean fclean re