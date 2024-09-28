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

NAME = webserv_test

all: $(NAME)

test:
	c++ -std=c++14 googletest.cpp -I$(HOME)/googletest/googletest/include -L$(HOME)/googletest/build/lib -lgtest -lgtest_main -pthread -o $(NAME)


CXX98 = c++ -std=c++98
CXX17 = c++ -std=c++17
CXXFLAGS += -Wall -Werror -Wextra -Wpedantic
CXXFLAGS += -g -Og
# CXXFLAGS += -fsanitize=address,undefined,leak

SOURCE_DIR = ../src
BUILD_DIR = build

IFLAGS = -I./$(SOURCE_DIR)
LDFLAGS =

UTIL_FILES = \
	c_api/utils.cpp

UTIL_SRC = $(addprefix $(SOURCE_DIR)/,$(UTIL_FILES))
UTIL_OBJ = $(patsubst $(SOURCE_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(UTIL_SRC))
UTIL_DEP = $(UTIL_OBJ:%.o=%.d)

# Create necessary directories under build/
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX98) $(CXXFLAGS) -c $< -o $@

utils: $(UTIL_OBJ)

$(NAME): $(UTIL_OBJ)
	$(CXX17) $(CXXFLAGS) googletest.cpp $(UTIL_OBJ) -I$(HOME)/googletest/googletest/include -L$(HOME)/googletest/build/lib -lgtest -lgtest_main -pthread -o $(NAME)
	@echo "$(GREEN)Executable $(NAME) created!$(DEF_COLOR)"

# $(NAME): $(UTIL_OBJ)
# 	$(CXX98) $(CXXFLAGS) $(UTIL_FILES)
# 	$(CXX98) $(CXXFLAGS) $(OBJ) -o $(NAME) $(LDFLAGS)
# 	@echo "$(GREEN)Executable $(NAME) created!$(DEF_COLOR)"

# utils:
# 	$(CXX98) $(CXXFLAGS) -c $(UTIL_SRC) -o $(UTIL_OBJ)


$(BUILD_DIR)/%.o : %.cpp
	mkdir -p $(@D)
	$(CC) $(CXXFLAGS) $(IFLAGS) -MMD -c $< -o $@

-include $(DEP)

clean:
	rm -fr $(BUILD_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

# valgrind:
# 	valgrind --trace-children=yes --track-origins=yes --leak-check=full --show-leak-kinds=all --track-fds=all ./$(NAME) $(DEFAULT_CONF)

.PHONY: all clean fclean re
