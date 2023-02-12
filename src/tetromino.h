#ifndef TETROMINO_H
#define TETROMINO_H

#define TETROMINO_BITMAP_WIDTH 4
#define TETROMINO_BITMAP_HEIGHT 4
#define TETROMINO_BITMAP_SIZE 16

typedef struct Tetromino
{
	int id;
	int pos;
	int bitmap[TETROMINO_BITMAP_SIZE];
} Tetromino;

void initialize_tetromino(Tetromino *tetromino, int pos);
void rotate_tetromino_clockwise(Tetromino *tetromino);
void rotate_tetromino_anticlockwise(Tetromino *tetromino);

#endif
