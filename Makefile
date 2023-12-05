NAME = ircserv

CFLAGS = -Wall -Wextra -Werror -std=c++98 -g

CC = c++

SRC = main.cpp Server.cpp Channel.cpp User.cpp

OBJS = ${SRC:.cpp=.o}

DEPENDENCES = ${OBJS:.o=.d}

%.o: %.cpp
			@$(CC) $(CFLAGS) -c $< -o $@

$(NAME): ${OBJS}
			@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
			@echo "$(COLOUR)MAKE ircserv$(COLOUR_END)"

all: $(NAME)

clean: 
		@rm -f $(OBJS)
		@rm -f $(DEPENDENCES)
		@echo "$(COLOUR)CLEAN ircserv$(COLOUR_END)"

fclean:
		@rm -f $(OBJS)
		@rm -f $(DEPENDENCES)
		@rm -f $(NAME)
		@echo "$(COLOUR)FCLEAN ircserv$(COLOUR_END)"

re: fclean all

.PHONY: all clean fclean re

COLOUR=\033[0;34m
COLOUR_END=\033[0;34m
