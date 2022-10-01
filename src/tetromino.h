#ifndef TETROMINO_H
#define TETROMINO_H

#define BITMAP_SIZE 16
#define BITMAP_WIDTH 4
#define BOX_CHARS_SIZE 16
#define NUM_OF_TETROMINOS 7

typedef struct Tetromino
{
	int id;
	int pos;
	int bitmap[BITMAP_SIZE];
} Tetromino;

void initialize_tetromino(Tetromino *tetromino, int width);
int insert_tetromino(Tetromino *tetromino, int width, int *cells);
void remove_tetromino(Tetromino *tetromino, int width, int *cells);
int move_tetromino(Tetromino *tetromino, int step, int width, int *cells);

#endif
