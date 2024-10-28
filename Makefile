NAME=webserv

all: webserv bonus

$(NAME): build

bonus:
	@echo no bonus yet

build:
	mkdir -p build
	cd build && cmake .. && make

run:
	./build/webserv ./conf/webserv.conf

fclean:
	rm -rf ./build

re: fclean all

test: build
	cd build && ctest
