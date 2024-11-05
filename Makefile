NAME = webserv

CMAKE_ARGS = -DCMAKE_C_COMPILER=cc -DCMAKE_CXX_COMPILER=c++
CMAKE_BUILD_TYPE_ARG ?= Debug
CMAKE = cmake $(CMAKE_ARGS) -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE_ARG)

BUILD_DIR = ./build

all: $(NAME) bonus

$(NAME): build

bonus:
	@echo no bonus yet

build:
	$(CMAKE) -S . -B $(BUILD_DIR) && make -C $(BUILD_DIR)
	cp $(BUILD_DIR)/$(NAME) .

run: build
	./$(NAME) ./conf/webserv.conf

clean:
	rm -rf $(BUILD_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

test: build
	cd $(BUILD_DIR) && ctest

.PHONY: all bonus build run clean fclean re test
