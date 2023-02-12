#include "tetromino.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define NUM_OF_TETROMINO_TYPES 7

static int TETROMINO_BITMAPS[NUM_OF_TETROMINO_TYPES][TETROMINO_BITMAP_SIZE] = {
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

static int map_index_clockwise(int x, int y);
static int map_index_anticlockwise(int x, int y);
static void swap_bitmaps(int (*b1)[TETROMINO_BITMAP_SIZE], int (*b2)[TETROMINO_BITMAP_SIZE]);
static void rotate_tetromino(Tetromino *tetromino, int (*index_map)(int, int));

void initialize_tetromino(Tetromino *tetromino, int pos)
{
	int i, bitmap_id;
	static int id = 2;
	tetromino->id = id++;
	tetromino->pos = pos;
	bitmap_id = rand() % NUM_OF_TETROMINO_TYPES;
	for (i = 0; i < TETROMINO_BITMAP_SIZE; ++i)
	{
		tetromino->bitmap[i] = TETROMINO_BITMAPS[bitmap_id][i];
	}
}

void rotate_tetromino_clockwise(Tetromino *tetromino)
{
	rotate_tetromino(tetromino, map_index_clockwise);
}

void rotate_tetromino_anticlockwise(Tetromino *tetromino)
{
	rotate_tetromino(tetromino, map_index_anticlockwise);
}

static int map_index_clockwise(int x, int y)
{
	return y + (TETROMINO_BITMAP_WIDTH - x - 1)*TETROMINO_BITMAP_WIDTH;
}

static int map_index_anticlockwise(int x, int y)
{
	return (TETROMINO_BITMAP_WIDTH - y - 1) + x*TETROMINO_BITMAP_WIDTH;
}

void swap_bitmaps(int (*b1)[TETROMINO_BITMAP_SIZE], int (*b2)[TETROMINO_BITMAP_SIZE])
{
	int i, tmp;
	for (i = 0; i < TETROMINO_BITMAP_SIZE; ++i)
	{
		tmp = (*b1)[i];
		(*b1)[i] = (*b2)[i];
		(*b2)[i] = tmp;
	}
}

void rotate_tetromino(Tetromino *tetromino, int (*index_map)(int, int))
{
	int x, y;
	int bitmap[TETROMINO_BITMAP_SIZE];

	for (y = 0; y < TETROMINO_BITMAP_HEIGHT; ++y)
	{
		for (x = 0; x < TETROMINO_BITMAP_WIDTH; ++x)
		{
			bitmap[x + y*TETROMINO_BITMAP_WIDTH] = tetromino->bitmap[index_map(x, y)];
		}
	}

	swap_bitmaps(&tetromino->bitmap, &bitmap);
}
