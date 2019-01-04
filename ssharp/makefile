CC=g++
OPT= #-v
DEBUG=-g
ME=transpile
all: build
build:
	$(CC) $(DEBUG) $(OPT) main.cpp -o $(ME)
run:
	./$(ME)
clean:
	if test -e $(ME); then rm $(ME); fi

