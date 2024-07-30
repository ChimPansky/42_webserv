NAME = webserv

all: $(NAME)

SOURCE_DIR = src
BUILD_DIR = build
DEFAULT_CONF = conf/webserv.conf

CC = c++
CFLAGS = -std=c++98
CFLAGS += -Wall -Werror -Wextra -Wpedantic
CFLAGS += -g -Og #-fsanitize=address,undefined,leak

#IFLAGS = -I/usr/include -I./include
#LFLAGS = -lm

FILENAMES = \
	webserv.cpp

SRC = $(addprefix $(SOURCE_DIR)/,$(FILENAMES))
OBJ = $(SRC:%.cpp=$(BUILD_DIR)/%.o)
DEP = $(OBJ:%.o=%.d)

#Colors
DEF_COLOR = \033[0;39m
GRAY = \033[0;90m
RED = \033[0;91m
GREEN = \033[0;92m
YELLOW = \033[0;93m
BLUE = \033[0;94m
MAGENTA = \033[0;95m
CYAN = \033[0;96m
WHITE = \033[0;97m

$(NAME): $(OBJ) $(LIBFT) $(LIBMLX)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME) $(LFLAGS)
	@echo "$(GREEN)Executable $(NAME) created!$(DEF_COLOR)"

$(BUILD_DIR)/%.o : %.cpp
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(IFLAGS) -MMD -c $< -o $@

-include $(DEP)

clean:
	rm -fr $(BUILD_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

valgrind:
	valgrind --trace-children=yes --track-origins=yes --leak-check=full --show-leak-kinds=all --track-fds=all ./$(NAME) $(DEFAULT_CONF)

.PHONY: all bonus clean fclean re test
