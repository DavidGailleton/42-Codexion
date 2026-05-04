NAME			= codexion
CC				= cc
MAKE			+= --no-print-directory
CCFLAGS		= -Wall -Wextra -Werror -pthread -MMD -MP
CCFLAGS		+= -Icoders
DEBUG_CCFLAGS = -g3
CLANG_FLAGS = -Wall -Wextra -Werror -Wpedantic \
  -Wshadow -Wconversion -Wsign-conversion -Wcast-qual -Wstrict-prototypes \
  -Wmissing-prototypes -Wpointer-arith -Wundef -Wwrite-strings \
  -Winit-self -Wswitch-enum -Wformat=2 -Wunreachable-code \
  -pthread -g3 -O0 -fno-omit-frame-pointer \
  -fsanitize=address,undefined \

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
	$(CC) $(DEBUG_CCFLAGS) -Icoders $(SRC) -pthread -o $(NAME) 

%.o: %.c Makefile
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ) $(SRC:.c=.d)

fclean: clean
	$(RM) $(NAME)

re: fclean all

lint:
	norminette
	clang $(CLANG_FLAGS) $(SRC) -Icoders

format:
	c_formatter_42 $(SRC) coders/codexion.h

FORCE: ;
-include $(SRC:.c=.d)

.PHONY: all clean fclean re

