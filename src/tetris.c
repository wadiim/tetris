#include "tetromino.h"
#include "tetris.h"
#include "term.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define TETROMINO_PREVIEW_WIDTH 8
#define TETROMINO_PREVIEW_HEIGHT 8

static char *BOX_CHARS[BOX_CHARS_SIZE] = {
	"\xE2\x94\xBC\xE2\x94\x80", /* 0b0000 -> "┼─" */
	"\xE2\x94\x9C\xE2\x94\x80", /* 0b0001 -> "├─" */
	"\xE2\x94\xB4\xE2\x94\x80", /* 0b0010 -> "┴─" */
	"\xE2\x94\x94\xE2\x94\x80", /* 0b0011 -> "└─" */
	"\xE2\x94\xA4 ",            /* 0b0100 -> "┤ " */
	"\xE2\x94\x82 ",            /* 0b0101 -> "│ " */
	"\xE2\x94\x98 ",            /* 0b0110 -> "┘ " */
	"",                         /* 0b0111 -> ""   */
	"\xE2\x94\xAC\xE2\x94\x80", /* 0b1000 -> "┬─" */
	"\xE2\x94\x8C\xE2\x94\x80", /* 0b1001 -> "┌─" */
	"\xE2\x94\x80\xE2\x94\x80", /* 0b1010 -> "──" */
	"",                         /* 0b1011 -> ""   */
	"\xE2\x94\x90 ",            /* 0b1100 -> "┐ " */
	"",                         /* 0b1101 -> ""   */
	"",                         /* 0b1110 -> ""   */
	"  "                        /* 0b1111 -> "  " */
};

static int *get_cell_neighbours(int idx, int width, int *cells);
static char *cell_to_box_char(int idx, int width, int *cells);
static int rotate_active_tetromino(Tetris *tetris, int (*rotate)(Tetromino *tetromino, int width, int *cells));
static int *generate_tetromino_preview_bitmap(Tetromino *tetromino);

void initialize_tetris(Tetris *tetris, int width, int height)
{
	int i, cells_size = width * height;

	tetris->width = width;
	tetris->height = height;
	tetris->active_tetromino = NULL;
	if ((tetris->cells = malloc((cells_size)*sizeof(int))) == NULL)
	{
		perror("Failed to initialize tetris");
		exit(errno);
	}

	/* Set all cells to 0 */
	memset(tetris->cells, 0, (cells_size)*sizeof(int));

	/* Add horizontal borders */
	for (i = 0; i < width; ++i)
	{
		tetris->cells[i] = 1;
		tetris->cells[cells_size - i - 1] = 1;
	}

	/* Add vertical borders */
	for (i = width; i < cells_size; i += width)
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
	initialize_tetromino(tetris->next_tetromino, tetris->width);
}

void terminate_tetris(Tetris *tetris)
{
	free(tetris->cells);
	free(tetris->active_tetromino);
}

char *tetris_to_str(Tetris *tetris)
{
	char *str;
	char *bchars;
	int i, j = 0, k, wrows, wcols, vmsize, hmsize;

	get_window_size(&wcols, &wrows);
	vmsize = (wrows - tetris->height) / 2;
	hmsize = (wcols - 2*tetris->width - TETROMINO_PREVIEW_WIDTH) / 2;
	if ((str = malloc(((tetris->width + vmsize)*(tetris->height + hmsize) + 2)*3*2*sizeof(char))) == NULL)
	{
		perror("Failed to generate string representation of tetris");
		exit(errno);
	}

	/* Center vertically */
	for (i = 0; i < vmsize; ++i)
	{
		memcpy(str + j, "\n\r", 2);
		j += 2;
	}

	for (i = tetris->width; i < tetris->width * tetris->height; ++i)
	{
		/* Center horizontally */
		if (i % tetris->width == 0)
		{
			for (k = 0; k < hmsize; ++k)
			{
				str[j++] = ' ';
			}
			continue;
		}

		/* Generate board */
		bchars = cell_to_box_char(i, tetris->width, tetris->cells);
		memcpy(str + j, bchars, strlen(bchars));
		j += strlen(bchars);
		if (i > 0 && (i + 1) % tetris->width == 0)
		{
			memcpy(str + j, "\n\r", 2);
			j += 2;
		}
	}

	str[j] = '\0';
	return str;
}

char *get_tetromino_preview_str(Tetris *tetris)
{
	char *bchars;
	int row, col, wrows, wcols, vmsize, hmsize, i = 0;

	int *bitmap = generate_tetromino_preview_bitmap(tetris->next_tetromino);
	char *preview_str = malloc((TETROMINO_PREVIEW_WIDTH + 8)*TETROMINO_PREVIEW_WIDTH*3*2*sizeof(char));

	if (preview_str == NULL)
	{
		perror("Failed to allocate string for tetromino preview");
		exit(errno);
	}

	get_window_size(&wcols, &wrows);
	vmsize = (wrows - tetris->height) / 2;
	hmsize = (wcols - 2*tetris->width - TETROMINO_PREVIEW_WIDTH) / 2 + tetris->width + TETROMINO_PREVIEW_WIDTH + 1;

	for (row = 1; row < TETROMINO_PREVIEW_HEIGHT; ++row)
	{
		i += sprintf(preview_str + i, "\x1b[%i;%iH", row + vmsize, hmsize + 1);
		for (col = 1; col < TETROMINO_PREVIEW_WIDTH; ++col)
		{
			bchars = cell_to_box_char(row*TETROMINO_PREVIEW_WIDTH + col, TETROMINO_PREVIEW_WIDTH, bitmap);
			memcpy(preview_str + i, bchars, strlen(bchars));
			i += strlen(bchars);
		}
	}
	preview_str[i] = '\0';
	free(bitmap);

	return preview_str;
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
	initialize_tetromino(tetris->next_tetromino, tetris->width);
	return insert_tetromino(tetris->active_tetromino, tetris->width, tetris->cells);
}

int move_active_tetromino_left(Tetris *tetris)
{
	return move_tetromino(tetris->active_tetromino,
			-1,
			tetris->width,
			tetris->cells);
}

int move_active_tetromino_right(Tetris *tetris)
{
	return move_tetromino(tetris->active_tetromino,
			1,
			tetris->width,
			tetris->cells);
}

int move_active_tetromino_down(Tetris *tetris)
{
	return move_tetromino(tetris->active_tetromino,
			tetris->width,
			tetris->width,
			tetris->cells);
}

int rotate_active_tetromino_clockwise(Tetris *tetris)
{
	return rotate_active_tetromino(tetris, rotate_tetromino_clockwise);
}

int rotate_active_tetromino_anticlockwise(Tetris *tetris)
{
	return rotate_active_tetromino(tetris, rotate_tetromino_anticlockwise);
}

void drop_active_tetromino(Tetris *tetris)
{
	while (move_active_tetromino_down(tetris))
	{
	}
}

int remove_full_rows(Tetris *tetris)
{
	int row, col, is_full, first_removed = -1;
	for (row = 1; row < tetris->height - 1; ++row)
	{
		is_full = 1;
		for (col = 1; col < tetris->width - 1; ++col)
		{
			if (tetris->cells[col + row*tetris->width] == 0)
			{
				is_full = 0;
				break;
			}
		}

		if (is_full == 1)
		{
			if (first_removed == -1)
			{
				first_removed = row;
			}
			for (col = 1; col < tetris->width - 1; ++col)
			{
				tetris->cells[col + row*tetris->width] = 0;
			}
		}
	}
	return first_removed;
}

void remove_empty_rows(Tetris *tetris, int start)
{
	int row, tmp_row, col, is_empty;
	for (row = tetris->height - 2; row >= start; --row)
	{
		is_empty = 1;
		for (col = 1; col < tetris->width - 1; ++col)
		{
			if (tetris->cells[col + row*tetris->width] != 0)
			{
				is_empty = 0;
				break;
			}
		}

		if (is_empty == 1)
		{
			for (col = 1; col < tetris->width - 1; ++col)
			{
				for (tmp_row = row; tmp_row > 1; --tmp_row)
				{
					tetris->cells[col + tmp_row*tetris->width] = tetris->cells[col + (tmp_row - 1)*tetris->width];
				}
			}
			++row;
			++start;
		}
	}
}

int *get_cell_neighbours(int idx, int width, int *cells)
{
	int *n = malloc(4*sizeof(int));
	if (n == NULL)
	{
		perror("Failed to get cell's neighbours");
		exit(errno);
	}
	n[0] = (idx - width - 1 < 0) ? 0 : cells[idx - width - 1];
	n[1] = (idx - width < 0) ? 0 : cells[idx - width];
	n[2] = (idx - 1 < 0) ? 0 : cells[idx - 1];
	n[3] = cells[idx];
	return n;
}

char *cell_to_box_char(int idx, int width, int *cells)
{
	int bchar_idx = 0;
	int *n = get_cell_neighbours(idx, width, cells);
	if (n[0] == n[2]) bchar_idx += 1;
	if (n[2] == n[3]) bchar_idx += 2;
	if (n[3] == n[1]) bchar_idx += 4;
	if (n[1] == n[0]) bchar_idx += 8;
	free(n);
	return BOX_CHARS[bchar_idx];
}

static int rotate_active_tetromino(Tetris *tetris, int (*rotate)(Tetromino *tetromino, int width, int *cells))
{
	int ret;
	remove_tetromino(tetris->active_tetromino, tetris->width, tetris->cells);
	ret = rotate(tetris->active_tetromino, tetris->width, tetris->cells);
	insert_tetromino(tetris->active_tetromino, tetris->width, tetris->cells);
	return ret;
}

static int *generate_tetromino_preview_bitmap(Tetromino *tetromino)
{
	int row, col, i;
	int *bitmap = calloc(TETROMINO_PREVIEW_WIDTH*TETROMINO_PREVIEW_HEIGHT, sizeof(int));
	if (bitmap == NULL)
	{
		perror("Failed to generate tetromino preview bitmap");
		exit(errno);
	}

	for (i = 0; i < TETROMINO_PREVIEW_WIDTH; ++i)
	{
		bitmap[i] = bitmap[TETROMINO_PREVIEW_WIDTH*TETROMINO_PREVIEW_HEIGHT - i - 1] = bitmap[i*TETROMINO_PREVIEW_WIDTH] = bitmap[i*TETROMINO_PREVIEW_WIDTH + (TETROMINO_PREVIEW_WIDTH - 1)] = 1;
	}

	for (col = 0; col < 4; ++col)
	{
		for (row = 0; row < 4; ++row)
		{
			bitmap[(row + 2)*TETROMINO_PREVIEW_WIDTH + (col + 2)] = tetromino->bitmap[row*4 + col];
		}
	}

	return bitmap;
}
