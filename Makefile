NAME = ./build/webserv

CMAKE_ARGS = -DCMAKE_C_COMPILER=cc -DCMAKE_CXX_COMPILER=c++
CMAKE_BUILD_TYPE_ARG ?= Debug
CMAKE = cmake $(CMAKE_ARGS) -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE_ARG)

all: $(NAME) bonus

$(NAME): build

bonus:
	@echo no bonus yet

build:
	$(CMAKE) -S . -B ./build && make -C ./build

run: build
	./build/webserv ./conf/webserv.conf

clean:
	mv $(NAME) ./ws_tmp
	rm -rf ./build
	mkdir -p build
	mv ./ws_tmp $(NAME)

fclean:
	rm -rf ./build

re: fclean all

test: build
	cd build && ctest

.PHONY: all bonus build run clean fclean re test
