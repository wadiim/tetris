flags = -std=c89 -Wall -Wextra -Wpedantic -g3

tetris: main.o game.o tetris.o tetromino.o term.o
	mkdir -p bin
	gcc $(flags) \
		build/main.o \
		build/game.o \
		build/tetris.o \
		build/tetromino.o \
		build/term.o \
		-o bin/tetris

main.o: src/main.c
	mkdir -p build
	gcc $(flags) -c src/main.c -o build/main.o

game.o: src/game.c src/game.h
	gcc $(flags) -c src/game.c -o build/game.o

tetris.o: src/tetris.c src/tetris.h
	gcc $(flags) -c src/tetris.c -o build/tetris.o

tetromino.o: src/tetromino.c src/tetromino.h
	gcc $(flags) -c src/tetromino.c -o build/tetromino.o

term.o: src/term.c src/term.h
	gcc $(flags) -c src/term.c -o build/term.o

clean:
	rm -rf bin/ build/
