NAME			= codexion
CC				= cc
MAKE			+= --no-print-directory
CCFLAGS		= -Wall -Wextra -Werror -pthread -MMD -MP
CCFLAGS		+= -Icoders
DEBUG_CCFLAGS = -g3
ADDRSAN_FLAGS = -Wall -Wextra -Werror -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \

SRC				= coders/main.c \
						coders/init.c \
						coders/parsing.c \
						coders/compile_process.c \
						coders/destroy.c \
						coders/thread_work.c \
						coders/utils.c \
						coders/burnout_checker.c \
						coders/config_utils.c \
						coders/time_utils.c
OBJ				= $(SRC:.c=.o)


all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CCFLAGS) $(OBJ) -o $(NAME) 

debug:
	$(CC) $(DEBUG_CCFLAGS) $(SRC) -pthread -o $(NAME) 

addrsan:
	$(CC) $(ADDRSAN_FLAGS) $(SRC) -pthread -o $(NAME) 

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

