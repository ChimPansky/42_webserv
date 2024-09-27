NAME = webserv

all: $(NAME) bonus

SOURCE_DIR = src
BUILD_DIR = build
DEFAULT_CONF = conf/webserv.conf

CXX = c++
CXXFLAGS += -Wall -Werror -Wextra -Wpedantic -std=c++98
CXXFLAGS += -g -Og
# CXXFLAGS += -fsanitize=address,undefined,leak

IFLAGS = -I./$(SOURCE_DIR)
LDFLAGS =

FILENAMES = \
	c_api/multiplexers/EpollMultiplexer.cpp \
	c_api/multiplexers/IMultiplexer.cpp \
	c_api/multiplexers/SelectMultiplexer.cpp \
	c_api/ClientSocket.cpp \
	c_api/EventManager.cpp \
	c_api/MasterSocket.cpp \
	c_api/utils.cpp \
	utils/logger.cpp \
	ClientSession.cpp \
	Config.cpp \
	server/Server.cpp \
	ServerCluster.cpp \
	main.cpp

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
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME) $(LDFLAGS)
	@echo "$(GREEN)Executable $(NAME) created!$(DEF_COLOR)"

bonus:
	@echo "$(RED)no bonus yet$(DEF_COLOR)"

$(BUILD_DIR)/%.o : %.cpp
	mkdir -p $(@D)
	$(CC) $(CXXFLAGS) $(IFLAGS) -MMD -c $< -o $@

-include $(DEP)

clean:
	rm -fr $(BUILD_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

run:
	./$(NAME) $(DEFAULT_CONF)

debug:
	./lldb $(NAME) $(DEFAULT_CONF)

generate_compile_commands:
	bear -- make re

clang-uml: generate_compile_commands
	clang-uml

# valgrind:
# 	valgrind --trace-children=yes --track-origins=yes --leak-check=full --show-leak-kinds=all --track-fds=all ./$(NAME) $(DEFAULT_CONF)

.PHONY: all bonus clean fclean re run debug valgrind clang-uml
