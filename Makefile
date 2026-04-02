NAME			= codexion
CC				= cc
MAKE			+= --no-print-directory
CCFLAGS		= -Wall -Wextra -Werror -MMD -MP
CCFLAGS		+= -Iinc
LDFLAGS		= -Llibft -lunit -lft -L.

SRC				= 
OBJ				= $(SRC:.c=.o)


all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(NAME) $(OBJ)

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

