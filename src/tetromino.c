#include "tetromino.h"

#include <stdlib.h>

static int TETROMINO_BITMAPS[NUM_OF_TETROMINOS][BITMAP_SIZE] = {
	{	/*   I   */
		0, 0, 1, 0,
		0, 0, 1, 0,
		0, 0, 1, 0,
		0, 0, 1, 0
	},
	{	/*   J   */
		0, 0, 1, 0,
		0, 0, 1, 0,
		0, 1, 1, 0,
		0, 0, 0, 0
	},
	{	/*   L   */
		0, 1, 0, 0,
		0, 1, 0, 0,
		0, 1, 1, 0,
		0, 0, 0, 0
	},
	{	/*   O   */
		0, 0, 0, 0,
		0, 1, 1, 0,
		0, 1, 1, 0,
		0, 0, 0, 0
	},
	{	/*   S   */
		0, 0, 0, 0,
		0, 0, 1, 1,
		0, 1, 1, 0,
		0, 0, 0, 0
	},
	{	/*   Z   */
		0, 0, 0, 0,
		0, 1, 1, 0,
		0, 0, 1, 1,
		0, 0, 0, 0
	},
	{	/*   T   */
		0, 0, 0, 0,
		0, 1, 1, 1,
		0, 0, 1, 0,
		0, 0, 0, 0,
	},
};

static void remove_tetromino(Tetromino *tetromino, int width, int *cells);
static int is_colliding(Tetromino *tetromino, int new_pos, int width, int *cells);
static void overwrite_tetromino(Tetromino *tetromino, int val, int width, int *cells);

void initialize_tetromino(Tetromino *tetromino, int width)
{
	int i, bitmap_id;
	static int id = 2;
	tetromino->id = id++;
	tetromino->pos = width + (width / 2) - (BITMAP_WIDTH / 2);
	bitmap_id = rand() % NUM_OF_TETROMINOS;
	for (i = 0; i < BITMAP_SIZE; ++i)
	{
		tetromino->bitmap[i] = TETROMINO_BITMAPS[bitmap_id][i];
	}
}

int insert_tetromino(Tetromino *tetromino, int width, int *cells)
{
	if (is_colliding(tetromino, tetromino->pos, width, cells))
	{
		return 0;
	}
	overwrite_tetromino(tetromino, tetromino->id, width, cells);
	return 1;
}

int move_tetromino(Tetromino *tetromino, int step, int width, int *cells)
{
	if (is_colliding(tetromino, tetromino->pos + step, width, cells))
	{
		return 0;
	}
	remove_tetromino(tetromino, width, cells);
	tetromino->pos += step;
	insert_tetromino(tetromino, width, cells);
	return 1;
}

void remove_tetromino(Tetromino *tetromino, int width, int *cells)
{
	overwrite_tetromino(tetromino, 0, width, cells);
}

int is_colliding(Tetromino *tetromino, int new_pos, int width, int *cells)
{
	int x, y;
	for (y = 0; y < BITMAP_WIDTH; ++y)
	{
		for (x = 0; x < BITMAP_WIDTH; ++x)
		{
			if (tetromino->bitmap[x + y*BITMAP_WIDTH] == 1
				&& cells[new_pos + x + y*width] != 0
				&& cells[new_pos + x + y*width] != tetromino->id)
			{
				return 1;
			}
		}
	}
	return 0;
}

void overwrite_tetromino(Tetromino *tetromino, int val, int width, int *cells)
{
	int x, y; 
	for (y = 0; y < BITMAP_WIDTH; ++y)
	{
		for (x = 0; x < BITMAP_WIDTH; ++x)
		{
			if (tetromino->bitmap[x + y*BITMAP_WIDTH] == 1)
			{
				cells[tetromino->pos + x + y*width] = val;
			}
		}
	}
}
