CC=clang
HOME=/home/b
INC=$(HOME)/.guix-profile/include/c++:$(HOME)/.guix-profile/include/c++/x86_64-unknown-linux-gnu
LIB=-L$(HOME)/.guix-profile/lib -lm -lstdc++
OPT= -Wall -Wextra #-v
DEBUG=-g
all: build
build: main.cpp
	CPLUS_INCLUDE_PATH=$(INC) $(CC) $(DEBUG) $(LIB) $(OPT) main.cpp
run:
	./a.out
clean:
	rm a.out

