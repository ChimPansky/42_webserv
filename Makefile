ROOT_NAME = webserv
NAME = ./build/$(ROOT_NAME)
DEFAULT_CONF = ./conf/webserv.conf

CMAKE_ARGS = -DCMAKE_C_COMPILER=cc -DCMAKE_CXX_COMPILER=c++
CMAKE_BUILD_TYPE_ARG ?= Debug
CMAKE = cmake $(CMAKE_ARGS) -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE_ARG)

all: $(NAME) bonus

$(NAME): build

bonus:
	@echo no bonus yet

build:
	$(CMAKE) -S . -B ./build && make -j$(nproc) -C ./build
	ln -fs $(NAME) $(ROOT_NAME)

run: build
	$(NAME) $(DEFAULT_CONF)

clean:
	find ./build/ -mindepth 1 -maxdepth 1 ! -name '_deps' ! -name $(ROOT_NAME) -exec rm -rf {} +

fclean:
	rm -rf ./build $(ROOT_NAME)

re: clean all

test: build
	cd build && ctest

.PHONY: all bonus build run clean fclean re test
