#include "tetromino.h"
#include "tetris.h"
#include "term.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define CELLS_SIZE BOARD_ROWS*BOARD_COLS
#define START_POS BOARD_COLS + BOARD_COLS/2 - TETROMINO_BITMAP_WIDTH/2

static int is_colliding(Tetris *tetris, int new_pos);
static int move_active_tetromino(Tetris *tetris, int step);
static int insert_tetromino(Tetris *tetris, Tetromino *tetromino);
static void remove_tetromino(Tetris *tetris, Tetromino *tetromino);
static void overwrite_tetromino(Tetris *tetris, Tetromino *tetromino, int val);
static int rotate_active_tetromino(Tetris *tetris,
		void (*rotate)(Tetromino *tetromino),
		void (*undo)(Tetromino *tetromino));

void initialize_tetris(Tetris *tetris)
{
	int i;

	tetris->active_tetromino = NULL;

	if ((tetris->cells = calloc(CELLS_SIZE, sizeof(int))) == NULL)
	{
		perror("Failed to initialize tetris");
		exit(errno);
	}

	/* Add horizontal borders */
	for (i = 0; i < BOARD_COLS; ++i)
	{
		tetris->cells[i] = 1;
		tetris->cells[CELLS_SIZE - i - 1] = 1;
	}

	/* Add vertical borders */
	for (i = BOARD_COLS; i < CELLS_SIZE; i += BOARD_COLS)
	{
		tetris->cells[i-1] = 1;
		tetris->cells[i] = 1;
	}

	/* Initialize next tetromino */
	if ((tetris->next_tetromino = malloc(sizeof(Tetromino))) == NULL)
	{
		perror("Failed to initialize next tetromino");
		exit(errno);
	}
	initialize_tetromino(tetris->next_tetromino, START_POS);
}

void terminate_tetris(Tetris *tetris)
{
	free(tetris->cells);
	free(tetris->active_tetromino);
}

int add_new_tetromino(Tetris *tetris)
{
	free(tetris->active_tetromino);
	tetris->active_tetromino = tetris->next_tetromino;
	if ((tetris->next_tetromino = malloc(sizeof(Tetromino))) == NULL)
	{
		perror("Failed to add new tetromino");
		exit(errno);
	}
	initialize_tetromino(tetris->next_tetromino, START_POS);
	return insert_tetromino(tetris, tetris->active_tetromino);
}

int move_active_tetromino_left(Tetris *tetris)
{
	return move_active_tetromino(tetris, -1);
}

int move_active_tetromino_right(Tetris *tetris)
{
	return move_active_tetromino(tetris, 1);
}

int move_active_tetromino_down(Tetris *tetris)
{
	return move_active_tetromino(tetris, BOARD_COLS);
}

int rotate_active_tetromino_clockwise(Tetris *tetris)
{
	return rotate_active_tetromino(tetris,
			rotate_tetromino_clockwise,
			rotate_tetromino_anticlockwise);
}

int rotate_active_tetromino_anticlockwise(Tetris *tetris)
{
	return rotate_active_tetromino(tetris,
			rotate_tetromino_anticlockwise,
			rotate_tetromino_clockwise);
}

void drop_active_tetromino(Tetris *tetris)
{
	while (move_active_tetromino_down(tetris))
	{
	}
}

int remove_full_rows(Tetris *tetris)
{
	int row, col, is_full, num_of_rows_removed = 0;
	int start = tetris->active_tetromino->pos / BOARD_COLS;
	int end = (start + TETROMINO_BITMAP_HEIGHT > BOARD_ROWS - 1) ? BOARD_ROWS - 1 : start + TETROMINO_BITMAP_HEIGHT;

	for (row = start; row < end; ++row)
	{
		is_full = 1;
		for (col = 1; col < BOARD_COLS - 1; ++col)
		{
			if (tetris->cells[col + row*BOARD_COLS] == 0)
			{
				is_full = 0;
				break;
			}
		}

		if (is_full == 1)
		{
			for (col = 1; col < BOARD_COLS - 1; ++col)
			{
				tetris->cells[col + row*BOARD_COLS] = 0;
			}
			++num_of_rows_removed;
		}
	}

	return num_of_rows_removed;
}

void remove_empty_rows(Tetris *tetris)
{
	int row, tmp_row, col, is_empty;
	int start = tetris->active_tetromino->pos / BOARD_COLS;

	for (row = BOARD_ROWS - 2; row >= start; --row)
	{
		is_empty = 1;
		for (col = 1; col < BOARD_COLS - 1; ++col)
		{
			if (tetris->cells[col + row*BOARD_COLS] != 0)
			{
				is_empty = 0;
				break;
			}
		}

		if (is_empty == 1)
		{
			for (col = 1; col < BOARD_COLS - 1; ++col)
			{
				for (tmp_row = row; tmp_row > 1; --tmp_row)
				{
					tetris->cells[col + tmp_row*BOARD_COLS] = tetris->cells[col + (tmp_row - 1)*BOARD_COLS];
				}
			}
			++row;
			++start;
		}
	}
}

static int is_colliding(Tetris *tetris, int new_pos)
{
	int x, y;
	for (y = 0; y < TETROMINO_BITMAP_WIDTH; ++y)
	{
		for (x = 0; x < TETROMINO_BITMAP_WIDTH; ++x)
		{
			if (tetris->active_tetromino->bitmap[x + y*TETROMINO_BITMAP_WIDTH] == 1
				&& tetris->cells[new_pos + x + y*BOARD_COLS] != 0
				&& tetris->cells[new_pos + x + y*BOARD_COLS] != tetris->active_tetromino->id)
			{
				return 1;
			}
		}
	}
	return 0;
}

int move_active_tetromino(Tetris *tetris, int step)
{
	if (is_colliding(tetris, tetris->active_tetromino->pos + step))
	{
		return 0;
	}
	remove_tetromino(tetris, tetris->active_tetromino);
	tetris->active_tetromino->pos += step;
	insert_tetromino(tetris, tetris->active_tetromino);
	return 1;
}

int insert_tetromino(Tetris *tetris, Tetromino *tetromino)
{
	if (is_colliding(tetris, tetromino->pos))
	{
		return 0;
	}
	overwrite_tetromino(tetris, tetromino, tetromino->id);
	return 1;
}

void remove_tetromino(Tetris *tetris, Tetromino *tetromino)
{
	overwrite_tetromino(tetris, tetromino, 0);
}

void overwrite_tetromino(Tetris *tetris, Tetromino *tetromino, int val)
{
	int x, y;
	for (y = 0; y < TETROMINO_BITMAP_WIDTH; ++y)
	{
		for (x = 0; x < TETROMINO_BITMAP_WIDTH; ++x)
		{
			if (tetromino->bitmap[x + y*TETROMINO_BITMAP_WIDTH] == 1)
			{
				tetris->cells[tetromino->pos + x + y*BOARD_COLS] = val;
			}
		}
	}
}

static int rotate_active_tetromino(Tetris *tetris, void (*rotate)(Tetromino *tetromino), void (*undo)(Tetromino *tetromino))
{
	int ret;
	remove_tetromino(tetris, tetris->active_tetromino);
	rotate(tetris->active_tetromino);
	if (is_colliding(tetris, tetris->active_tetromino->pos))
	{
		undo(tetris->active_tetromino);
		ret = 0;
	}
	else
	{
		ret = 1;
	}
	insert_tetromino(tetris, tetris->active_tetromino);
	return ret;
}
