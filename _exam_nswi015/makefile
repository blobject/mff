CC=clang
OPT=-DUNIX -Wall -Wextra
DEBUG=-g
all:
	$(CC) $(OPT) $(DEBUG) main.c
run:
	cp example/{foo,bar,baz}[012] .
	./a.out foo0 bar0 baz0
	rm {foo,bar,baz}[012]
clean:
	if test -e a.out; then rm a.out; fi
