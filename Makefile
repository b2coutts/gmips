CC = gcc
CFLAGS = -std=c99 -Wall

bin/assemble: obj/assemble.o obj/parse.o obj/avl.o
	@mkdir -p bin
	$(CC) $(CFLAGS) obj/assemble.o obj/avl.o obj/parse.o -o bin/assemble

obj/%.o: %.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -rf obj/*.o bin/*
