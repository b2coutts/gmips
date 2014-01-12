CC = gcc
CFLAGS = -std=c99 -Wall

all: bin/emulate bin/assemble


bin/emulate: obj/emulate.o obj/machine.o obj/parse.o obj/avl.o
	@mkdir -p bin
	$(CC) $(CFLAGS) obj/emulate.o obj/avl.o obj/parse.o obj/machine.o -o bin/emulate

bin/assemble: obj/assemble.o obj/parse.o obj/avl.o
	@mkdir -p bin
	$(CC) $(CFLAGS) obj/assemble.o obj/avl.o obj/parse.o -o bin/assemble

obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -rf obj/*.o bin/*

.PHONY: clean, all
