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

CXX98 = c++ -std=c++98
CXX17 = c++ -std=c++17
CXXFLAGS += -Wall -Werror -Wextra -Wpedantic
CXXFLAGS += -g -Og

SOURCE_DIR = ../src
BUILD_DIR = build

IFLAGS = -I./$(SOURCE_DIR)

##### TESTERS #####
C_API_UTILS_TESTER = c_api_utils_tester
RQ_BUILDER_TESTER = rq_builder_tester

##### ALL #####
all: $(C_API_UTILS_TESTER) $(RQ_BUILDER_TESTER)

##### LOGGER Library #####
LOGGER_FILES = \
	utils/logger.cpp

LOGGER_SRC = $(addprefix $(SOURCE_DIR)/,$(LOGGER_FILES))
LOGGER_OBJ = $(patsubst $(SOURCE_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(LOGGER_SRC))
LOGGER_DEP = $(LOGGER_OBJ:%.o=%.d)
LOGGER_LIB = $(BUILD_DIR)/logger.a

$(LOGGER_LIB): $(LOGGER_OBJ)
	@echo "$(CYAN)Creating archive library $(LOGGER_LIB)$(DEF_COLOR)"
	@ar rcs $@ $^
	@echo "$(GREEN)$(LOGGER_LIB) created!$(DEF_COLOR)"

##### C_API_UTILS_TESTER #####
C_API_UTILS_FILES = \
	c_api/utils.cpp

C_API_UTILS_TESTER_FILES = \
	tests_c_api_utils.cpp

C_API_UTILS_SRC = $(addprefix $(SOURCE_DIR)/,$(C_API_UTILS_FILES))
C_API_UTILS_OBJ = $(patsubst $(SOURCE_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(C_API_UTILS_SRC))
C_API_UTILS_DEP = $(C_API_UTILS_OBJ:%.o=%.d)
C_API_UTILS_LIB = $(BUILD_DIR)/libc_api_utils.a

$(C_API_UTILS_LIB): $(C_API_UTILS_OBJ) $(C_API_UTILS_TESTER_FILES)
	@echo "$(CYAN)Creating archive library $(C_API_UTILS_LIB)$(DEF_COLOR)"
	@ar rcs $@ $^
	@echo "$(GREEN)$(C_API_UTILS_LIB) created!$(DEF_COLOR)"

$(C_API_UTILS_TESTER): $(C_API_UTILS_LIB)
	$(CXX17) $(CXXFLAGS) $(C_API_UTILS_TESTER_FILES) $(C_API_UTILS_LIB) -I$(HOME)/googletest/googletest/include -L$(HOME)/googletest/build/lib -lgtest -lgtest_main -pthread -o $(C_API_UTILS_TESTER)
	@echo "$(GREEN)Executable $(C_API_UTILS_TESTER) created!$(DEF_COLOR)"

##### RQ_BUILDER_TESTER #####
RQ_BUILDER_FILES = \
	http/RequestBuilder.cpp \
	http/Request.cpp \
	utils/utils.cpp

RQ_BUILDER_TESTER_FILES = \
	tests_rq_builder.cpp

RQ_BUILDER_SRC = $(addprefix $(SOURCE_DIR)/,$(RQ_BUILDER_FILES))
RQ_BUILDER_OBJ = $(patsubst $(SOURCE_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(RQ_BUILDER_SRC))
RQ_BUILDER_DEP = $(RQ_BUILDER_OBJ:%.o=%.d)
RQ_BUILDER_LIB = $(BUILD_DIR)/librq_builder.a

$(RQ_BUILDER_LIB): $(RQ_BUILDER_OBJ)
	@echo "$(CYAN)Creating archive library $(RQ_BUILDER_LIB)$(DEF_COLOR)"
	@ar rcs $@ $^
	@echo "$(GREEN)$(RQ_BUILDER_LIB) created!$(DEF_COLOR)"

$(RQ_BUILDER_TESTER): $(RQ_BUILDER_LIB) $(LOGGER_LIB) $(RQ_BUILDER_TESTER_FILES)
	$(CXX17) $(CXXFLAGS) $(RQ_BUILDER_TESTER_FILES) $(RQ_BUILDER_LIB) $(LOGGER_LIB) -I$(HOME)/googletest/googletest/include -L$(HOME)/googletest/build/lib -lgtest -lgtest_main -pthread -o $(RQ_BUILDER_TESTER)
	@echo "$(GREEN)Executable $(RQ_BUILDER_TESTER) created!$(DEF_COLOR)"

# TODO: try to fix dependencies, so that changes in headerfiles are recognized
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX98) $(CXXFLAGS) $(IFLAGS) -MMD -c $< -o $@

-include $(DEP)

#### CLEAN ####
clean:
	rm -fr $(BUILD_DIR)

fclean: clean
	rm -f $(C_API_UTILS_TESTER) $(RQ_BUILDER_TESTER)

re: fclean all

.PHONY: all clean fclean re
