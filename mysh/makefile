CC=clang
O=-DUNIX -Wno-builtin-requires-header # setjmp.h warning
L=-ledit
DEBUG=-g
ME=mysh

all: $(ME)

$(ME): parse.o repl.o util.o
	$(CC) $(O) $(L) $(DEBUG) main.c parse.o repl.o util.o -o $(ME)

test: parse.o repl.o util.o test.c
	$(CC) $(O) $(L) $(DEBUG) test.c parse.o repl.o util.o -o test

%.o: %.c
	$(CC) $(O) $(DEBUG) -c $<

run:
	./$(ME)

clean:
	rm $(ME) *.o *.gch test
