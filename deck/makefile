CC=gcc
L=-lm -lstdc++
O=-g
ME=deck

all: build

build: src/*.hpp src/main.cpp
	CPLUS_INCLUDE_PATH=$(I) $(CC) $(L) $(O) -o $(ME) src/main.cpp

run:
	./$(ME)

clean:
	if test -f $(ME); then rm $(ME); fi

