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

-include $(DEP)

CXX98 = c++ -std=c++98
CXX17 = c++ -std=c++17
CXXFLAGS += -Wall -Werror -Wextra -Wpedantic
CXXFLAGS += -g -Og

SOURCE_DIR = ../src
BUILD_DIR = build

IFLAGS = -I./$(SOURCE_DIR)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX98) $(CXXFLAGS) -c $< -o $@

##### C_API_UTILS_TESTER #####
C_API_UTILS_TESTER = c_api_utils_tester

C_API_UTILS_FILES = \
	c_api/utils.cpp

C_API_UTILS_TESTER_FILES = \
	c_api_utils_tests.cpp

C_API_UTILS_SRC = $(addprefix $(SOURCE_DIR)/,$(C_API_UTILS_FILES))
C_API_UTILS_OBJ = $(patsubst $(SOURCE_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(C_API_UTILS_SRC))
C_API_UTILS_DEP = $(C_API_UTILS_OBJ:%.o=%.d)
C_API_UTILS_LIB = $(BUILD_DIR)/libc_api_utils.a

c_api_utils: $(C_API_UTILS_LIB)

$(C_API_UTILS_LIB): $(C_API_UTILS_OBJ)
	@echo "$(CYAN)Creating archive library $(C_API_UTILS_LIB)$(DEF_COLOR)"
	@ar rcs $@ $^
	@echo "$(GREEN)$(C_API_UTILS_LIB) created!$(DEF_COLOR)"

$(C_API_UTILS_TESTER): c_api_utils
	$(CXX17) $(CXXFLAGS) $(C_API_UTILS_TESTER_FILES) $(C_API_UTILS_LIB) -I$(HOME)/googletest/googletest/include -L$(HOME)/googletest/build/lib -lgtest -lgtest_main -pthread -o $(C_API_UTILS_TESTER)
	@echo "$(GREEN)Executable $(C_API_UTILS_TESTER) created!$(DEF_COLOR)"

##### ALL #####
all: $(C_API_UTILS_TESTER)

#### CLEAN ####
clean:
	rm -fr $(BUILD_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re c_api_utils
