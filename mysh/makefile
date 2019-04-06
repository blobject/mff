CC=clang
OPT=-DUNIX -ledit
DEBUG=-g
ME=mysh
all: $(ME)
$(ME): $(ME).c main.c
	$(CC) $(OPT) $(DEBUG) $(ME).c main.c -o $(ME)
test: $(ME).c test.c
	$(CC) $(OPT) $(DEBUG) $(ME).c test.c -o test
run:
	./$(ME)
clean:
	if test -e $(ME); then rm $(ME); fi
	if test -e test; then rm test; fi
