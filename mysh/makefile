CC=clang
O=-DUNIX -Wno-builtin-requires-header # setjmp.h warning
L=-ledit
DEBUG=-g
ME=mysh

all: $(ME)

$(ME): obj/parse.o obj/repl.o obj/util.o
	mkdir -p bin
	mkdir -p obj
	$(CC) $(O) $(L) $(DEBUG) src/main.c obj/parse.o obj/repl.o obj/util.o -o bin/$(ME)

test: obj/parse.o obj/repl.o obj/util.o src/test.c
	mkdir -p bin
	mkdir -p obj
	$(CC) $(O) $(L) $(DEBUG) src/test.c obj/parse.o obj/repl.o obj/util.o -o bin/test

obj/%.o: src/%.c
	mkdir -p obj
	$(CC) $(O) $(DEBUG) -c $< -o $@

run:
	bin/$(ME)

clean:
	rm -R obj/ bin/ test
