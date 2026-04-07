NAME			= codexion
CC				= cc
MAKE			+= --no-print-directory
CCFLAGS		= -Wall -Wextra -Werror -pthread -MMD -MP
CCFLAGS		+= -Iinc -g3 -DEBUG=1
LDFLAGS		= -Llibft -lunit -lft -L.

SRC				= coders/main.c \
						coders/init_coders.c \
						coders/parsing.c
OBJ				= $(SRC:.c=.o)


all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(OBJ) -o $(NAME) 

%.o: %.c Makefile
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ) $(SRC:.c=.d)

fclean: clean
	$(RM) $(NAME)

re: fclean all

FORCE: ;
-include $(SRC:.c=.d)

.PHONY: all clean fclean re

