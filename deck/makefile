CC=clang
HOME=/home/b
INC=$(HOME)/.guix-profile/include/c++:$(HOME)/.guix-profile/include/c++/x86_64-unknown-linux-gnu
LIB=-L$(HOME)/.guix-profile/lib -lm -lstdc++
OPT= #-v
DEBUG=-g
OUT=deck
all: build
build: main.cpp test.cpp
	CPLUS_INCLUDE_PATH=$(INC) $(CC) $(DEBUG) $(LIB) $(OPT) -o $(OUT) main.cpp
	CPLUS_INCLUDE_PATH=$(INC) $(CC) $(DEBUG) $(LIB) $(OPT) -fPIC -shared -o deck.so main.cpp
test:
	CPLUS_INCLUDE_PATH=$(INC) $(CC) $(DEBUG) -L. $(LIB) $(OPT) -l:./deck.so -o test test.cpp
run:
	./$(OUT)
clean:
	rm $(OUT)
	rm test

