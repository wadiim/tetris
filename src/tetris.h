#ifndef TETRIS_H
#define TETRIS_H

#define BOARD_ROWS 22
#define BOARD_COLS 12

struct Tetromino;

typedef struct Tetris
{
	int *cells;
	struct Tetromino *active_tetromino;
	struct Tetromino *next_tetromino;
} Tetris;

void initialize_tetris(Tetris *tetris);
void terminate_tetris(Tetris *tetris);

int add_new_tetromino(Tetris *tetris);
int move_active_tetromino_left(Tetris *tetris);
int move_active_tetromino_right(Tetris *tetris);
int move_active_tetromino_down(Tetris *tetris);
int rotate_active_tetromino_clockwise(Tetris *tetris);
int rotate_active_tetromino_anticlockwise(Tetris *tetris);
void drop_active_tetromino(Tetris *tetris);

/* Returns the number of rows removed */
int remove_full_rows(Tetris *tetris);
void remove_empty_rows(Tetris *tetris);

#endif
