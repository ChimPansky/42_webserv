NAME = webserv

all: $(NAME)

SOURCE_DIR = src
BUILD_DIR = build

CXX = c++
CXXFLAGS = -Wall -Werror -Wextra -Wpedantic
# CXXFLAGS += -g -Og #-fsanitize=address,undefined,leak

IFLAGS = \
	-I./src/c_api \
	-I./src/config \
	-I./src/http \
	-I./src/server \
	-I./src/utils

# LFLAGS =

WS_FILENAMES = \
	main.cpp \
	c_api/multiplexers/EpollMultiplexer.cpp \
	c_api/multiplexers/PollMultiplexer.cpp \
	c_api/multiplexers/SelectMultiplexer.cpp \
	c_api/multiplexers/AMultiplexer.cpp \
	c_api/EventManager.cpp \
	c_api/ClientSocket.cpp \
	c_api/Socket.cpp \
	c_api/c_api_utils.cpp \
	c_api/ChildProcessesManager.cpp \
	c_api/MasterSocket.cpp \
	http/cgi/cgi.cpp \
	http/ResponseCodes.cpp \
	http/http.cpp \
	http/RqTarget.cpp \
	http/SyntaxChecker.cpp \
	http/RequestParser.cpp \
	http/RequestBuilder.cpp \
	http/Request.cpp \
	http/Response.cpp \
	utils/errors.cpp \
	utils/time_utils.cpp \
	utils/file_utils.cpp \
	utils/str_utils.cpp \
	utils/rand.cpp \
	utils/logger.cpp \
	config/ParsedConfig.cpp \
	config/InheritedSettings.cpp \
	config/Config.cpp \
	config/HttpConfig.cpp \
	config/ServerConfig.cpp \
	config/ServerConfigBuilder.cpp \
	config/HttpConfigBuilder.cpp \
	config/ConfigBuilder.cpp \
	config/LocationConfig.cpp \
	config/LocationConfigBuilder.cpp \
	server/response_processors/DirectoryProcessor.cpp \
	server/response_processors/ErrorProcessor.cpp \
	server/response_processors/FileProcessor.cpp \
	server/response_processors/RedirectProcessor.cpp \
	server/response_processors/AResponseProcessor.cpp \
	server/response_processors/CGIProcessor.cpp \
	server/utils/utils.cpp \
	server/Location.cpp \
	server/Server.cpp \
	ClientSession.cpp \
	ServerCluster.cpp

SRC = $(addprefix $(SOURCE_DIR)/,$(WS_FILENAMES))
OBJ = $(SRC:%.cpp=$(BUILD_DIR)/%.o)
DEP = $(OBJ:%.o=%.d)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME) $(LFLAGS)
	@echo "Executable $(NAME) created!"

$(BUILD_DIR)/%.o : %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(IFLAGS) -MMD -c $< -o $@

-include $(DEP)

clean:
	rm -fr $(BUILD_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re test
