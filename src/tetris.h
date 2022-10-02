#ifndef TETRIS_H
#define TETRIS_H

#define WIDTH 12
#define HEIGHT 22

struct Tetromino;

typedef struct Tetris
{
	int width;
	int height;
	int *cells;
	struct Tetromino *active_tetromino;
} Tetris;

void initialize_tetris(Tetris *tetris, int width, int height);
void terminate_tetris(Tetris *tetris);
char *tetris_to_str(Tetris *tetris);

int add_new_tetromino(Tetris *tetris);
int move_active_tetromino_left(Tetris *tetris);
int move_active_tetromino_right(Tetris *tetris);
int move_active_tetromino_down(Tetris *tetris);
int rotate_active_tetromino_clockwise(Tetris *tetris);
int rotate_active_tetromino_anticlockwise(Tetris *tetris);

/*
 * Returns the index of the first row that was removed, or -1 if
 * none row was removed.
 */
int remove_full_rows(Tetris *tetris);

void remove_empty_lines(Tetris *tetris, int start);

#endif
