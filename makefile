CFLAGS = -std=c89 -Wall -Wextra -Wpedantic

all: release

debug: CFLAGS += -DDEBUG -g3
debug: tetris

release: CFLAGS += -O3
release: tetris

tetris: main.o game.o tetris.o tetromino.o term.o utils.o
	mkdir -p bin
	$(CC) $(CFLAGS) \
		build/main.o \
		build/game.o \
		build/tetris.o \
		build/tetromino.o \
		build/term.o \
		build/utils.o \
		-o bin/tetris

main.o: src/main.c
	mkdir -p build
	$(CC) $(CFLAGS) -c src/main.c -o build/main.o

game.o: src/game.c src/game.h
	$(CC) $(CFLAGS) -c src/game.c -o build/game.o

tetris.o: src/tetris.c src/tetris.h
	$(CC) $(CFLAGS) -c src/tetris.c -o build/tetris.o

tetromino.o: src/tetromino.c src/tetromino.h
	$(CC) $(CFLAGS) -c src/tetromino.c -o build/tetromino.o

term.o: src/term.c src/term.h
	$(CC) $(CFLAGS) -c src/term.c -o build/term.o

utils.o: src/utils.c src/utils.h
	$(CC) $(CFLAGS) -c src/utils.c -o build/utils.o

clean:
	rm -rf bin/ build/
