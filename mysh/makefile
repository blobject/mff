CC=clang
O=-DUNIX -Wno-builtin-requires-header # setjmp.h warning
L=-ledit
DEBUG=-g
ME=mysh

all: $(ME)

$(ME): obj/parse.o obj/repl.o obj/util.o src/main.c
	mkdir -p obj
	$(CC) $(O) $(L) $(DEBUG) src/main.c obj/parse.o obj/repl.o obj/util.o -o $(ME)

test: obj/parse.o obj/repl.o obj/util.o src/test.c
	mkdir -p obj
	$(CC) $(O) $(L) $(DEBUG) src/test.c obj/parse.o obj/repl.o obj/util.o -o test

obj/%.o: src/%.c
	mkdir -p obj
	$(CC) $(O) $(DEBUG) -c $< -o $@

run:
	./$(ME)

clean:
	rm -R obj/ test $(ME)
