NAME = webserv

all: $(NAME) bonus

SOURCE_DIR = src
BUILD_DIR = build
DEFAULT_CONF = conf/webserv.conf

CXX = c++
CXXFLAGS += -Wall -Werror -Wextra -Wpedantic -std=c++98
CXXFLAGS += -g -Og
# CXXFLAGS += -fsanitize=address,undefined,leak
MAKEFLAGS = -j$(nproc)

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
	config/ParsedConfig.cpp \
	config/Config.cpp \
	config/HttpConfig.cpp \
	config/ServerConfig.cpp \
	config/LocationConfig.cpp \
	config/LocationConfigBuilder.cpp \
	config/ServerConfigBuilder.cpp \
	config/HttpConfigBuilder.cpp \
	config/ConfigBuilder.cpp \
	config/InheritedSettings.cpp \
	http/Request.cpp \
	http/RequestBuilder.cpp \
	http/RequestParser.cpp \
	server/Server.cpp \
	utils/logger.cpp \
	utils/utils.cpp \
	ClientSession.cpp \
	ServerCluster.cpp

SRC = $(addprefix $(SOURCE_DIR)/,$(FILENAMES) main.cpp)
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

CONFIG_TEST = test_configs/config_parser_test.cpp \
	$(addprefix $(SOURCE_DIR)/,$(FILENAMES))

GTEST_DIR = $(HOME)/googletest/googletest/include
GTEST_LIB_DIR = $(HOME)/googletest/build/lib

tests: $(CONFIG_TEST)
	$(CXX)	-I$(SOURCE_DIR) -I$(GTEST_DIR) -L$(GTEST_LIB_DIR) $^ -lgtest -lgtest_main -o $(BUILD_DIR)/runTests
	$(BUILD_DIR)/runTests

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
