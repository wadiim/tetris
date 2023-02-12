#include "game.h"
#include "term.h"
#include "tetris.h"
#include "tetromino.h"

#define _DEFAULT_SOURCE

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TETROMINO_PREVIEW_ROWS 8
#define TETROMINO_PREVIEW_COLS 8

#define SCORE_VIEW_COLS 8

#define CELL_WIDTH_IN_BOX_SEQS 2
#define MAX_BOX_SEQ_LEN_IN_BYTES 6
#define MAX_ESC_SEQ_LEN_IN_BYTES 10

#define BOX_SEQS_SIZE 16

static char *BOX_SEQS[BOX_SEQS_SIZE] = {
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

static void update_screen(Game *game);
static int *get_cell_neighbours(int idx, int width, int *cells);
static char *cell_to_box_seq(int idx, int width, int *cells);
static char *get_board_string(Tetris *tetris, int start_x, int start_y);
static char *get_tetromino_preview_string(Tetris *tetris, int start_x, int start_y);
static int *get_tetromino_preview_bitmap(Tetromino *tetromino);
static char *get_score_view_string(Game *game, int start_x, int start_y);
static int handle_bottom_collision(Game *game);
static void update_score(Game *game, int num_of_rows_removed);

void initialize_game(Game *game)
{
	game->score = 0;
	game->tetris = malloc(sizeof(Tetris *));
	initialize_tetris(game->tetris);
	add_new_tetromino(game->tetris);
}

void terminate_game(Game *game)
{
	terminate_tetris(game->tetris);
	free(game->tetris);
}

void game_loop(Game *game)
{
	int input;
	clock_t start = clock();

	while ((input = get_input()) != 'q')
	{
		if (input == 0) continue;

		switch (input)
		{
		case 'h':
		case ARROW_LEFT:
			move_active_tetromino_left(game->tetris);
			break;
		case 'l':
		case ARROW_RIGHT:
			move_active_tetromino_right(game->tetris);
			break;
		case 'j':
		case ARROW_DOWN:
			rotate_active_tetromino_clockwise(game->tetris);
			break;
		case 'k':
		case ARROW_UP:
			rotate_active_tetromino_anticlockwise(game->tetris);
			break;
		case ENTER:
			drop_active_tetromino(game->tetris);
			break;
		}

		if (input == ' ' || ((float)(clock() - start) / CLOCKS_PER_SEC) > 0.0004)
		{
			if (move_active_tetromino_down(game->tetris) == 0
				&& handle_bottom_collision(game) == 0)
			{
				return;
			}
			start = clock();
		}

		update_screen(game);
	}
}

static void update_screen(Game *game)
{
	int wrows, wcols, start_x, start_y;

	char *board_string;
	char *preview_string;
	char *score_view_string;

	get_window_size(&wcols, &wrows);
	start_x = (wcols - CELL_WIDTH_IN_BOX_SEQS*BOARD_COLS - TETROMINO_PREVIEW_COLS) / 2;
	start_y = (wrows - BOARD_ROWS) / 2;

	board_string = get_board_string(game->tetris, start_x, start_y);
	start_x += (BOARD_COLS - 1)*CELL_WIDTH_IN_BOX_SEQS;
	preview_string = get_tetromino_preview_string(game->tetris, start_x, start_y);
	start_y += TETROMINO_PREVIEW_ROWS;
	score_view_string = get_score_view_string(game, start_x, start_y);

	write(STDOUT_FILENO, board_string, strlen(board_string));
	write(STDOUT_FILENO, preview_string, strlen(preview_string));
	write(STDOUT_FILENO, score_view_string, strlen(score_view_string));

	free(score_view_string);
	free(preview_string);
	free(board_string);
}

static int *get_cell_neighbours(int idx, int width, int *cells)
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

static char *cell_to_box_seq(int idx, int width, int *cells)
{
	int bchar_idx = 0;
	int *n = get_cell_neighbours(idx, width, cells);
	if (n[0] == n[2]) bchar_idx += 1;
	if (n[2] == n[3]) bchar_idx += 2;
	if (n[3] == n[1]) bchar_idx += 4;
	if (n[1] == n[0]) bchar_idx += 8;
	free(n);
	return BOX_SEQS[bchar_idx];
}

static char *get_board_string(Tetris *tetris, int start_x, int start_y)
{
	char *str;
	char *bchars;
	int i, str_pos = 0;

	if ((str = malloc(((CELL_WIDTH_IN_BOX_SEQS*BOARD_COLS*BOARD_ROWS)*MAX_BOX_SEQ_LEN_IN_BYTES + BOARD_ROWS*MAX_ESC_SEQ_LEN_IN_BYTES)*sizeof(char))) == NULL)
	{
		perror("Failed to generate string representation of tetris");
		exit(errno);
	}

	for (i = BOARD_COLS; i < BOARD_COLS*BOARD_ROWS; ++i)
	{
		if (i % BOARD_COLS == 0)
		{
			str_pos += sprintf(str + str_pos, "\x1b[%i;%iH", start_y + (i / BOARD_COLS), start_x);
			continue;
		}

		bchars = cell_to_box_seq(i, BOARD_COLS, tetris->cells);
		memcpy(str + str_pos, bchars, strlen(bchars));
		str_pos += strlen(bchars);
	}

	str[str_pos] = '\0';
	return str;
}

static char *get_tetromino_preview_string(Tetris *tetris, int start_x, int start_y)
{
	char *bchars;
	int row, col, str_pos = 0;

	int *bitmap = get_tetromino_preview_bitmap(tetris->next_tetromino);
	char *str = malloc((TETROMINO_PREVIEW_ROWS + MAX_ESC_SEQ_LEN_IN_BYTES)*TETROMINO_PREVIEW_COLS*CELL_WIDTH_IN_BOX_SEQS*MAX_BOX_SEQ_LEN_IN_BYTES*sizeof(char));

	if (str == NULL)
	{
		perror("Failed to allocate string for tetromino preview");
		exit(errno);
	}

	for (row = 1; row < TETROMINO_PREVIEW_ROWS; ++row)
	{
		str_pos += sprintf(str + str_pos, "\x1b[%i;%iH", row + start_y, start_x);
		for (col = 1; col < TETROMINO_PREVIEW_COLS; ++col)
		{
			bchars = cell_to_box_seq(row*TETROMINO_PREVIEW_COLS + col, TETROMINO_PREVIEW_COLS, bitmap);
			memcpy(str + str_pos, bchars, strlen(bchars));
			str_pos += strlen(bchars);
		}
	}
	str[str_pos] = '\0';
	free(bitmap);

	return str;
}

static int *get_tetromino_preview_bitmap(Tetromino *tetromino)
{
	int row, col, i;
	int *bitmap = calloc(TETROMINO_PREVIEW_COLS*TETROMINO_PREVIEW_ROWS, sizeof(int));
	if (bitmap == NULL)
	{
		perror("Failed to generate tetromino preview bitmap");
		exit(errno);
	}

	/* Generate borders */
	for (i = 0; i < TETROMINO_PREVIEW_COLS; ++i)
	{
		bitmap[i] = bitmap[TETROMINO_PREVIEW_COLS*TETROMINO_PREVIEW_ROWS - i - 1] = bitmap[i*TETROMINO_PREVIEW_COLS] = bitmap[i*TETROMINO_PREVIEW_COLS + (TETROMINO_PREVIEW_COLS - 1)] = 1;
	}

	for (col = 0; col < TETROMINO_BITMAP_WIDTH; ++col)
	{
		for (row = 0; row < TETROMINO_BITMAP_HEIGHT; ++row)
		{
			bitmap[(row + 2)*TETROMINO_PREVIEW_COLS + (col + 2)] = tetromino->bitmap[row*4 + col];
		}
	}

	return bitmap;
}

static char *get_score_view_string(Game *game, int start_x, int start_y)
{
	char *str = malloc(3*SCORE_VIEW_COLS*MAX_BOX_SEQ_LEN_IN_BYTES*sizeof(char));
	int str_pos = 0, i;

	if (str == NULL)
	{
		perror("Failed to generate score view string");
		exit(errno);
	}

	str_pos += sprintf(str + str_pos, "\x1b[%i;%iH%s", start_y, start_x, BOX_SEQS[9]);

	for (i = 0; i < (SCORE_VIEW_COLS / 2) + 1; ++i)
	{
		str_pos += sprintf(str + str_pos, "%s", BOX_SEQS[10]);
	}

	str_pos += sprintf(str + str_pos,
			"%s\x1b[%i;%iH%s%10i%s\x1b[%i;%iH%s",
			BOX_SEQS[12],
			start_y + 1,
			start_x,
			BOX_SEQS[5],
			game->score,
			BOX_SEQS[5],
			start_y + 2,
			start_x,
			BOX_SEQS[3]
			);

	for (i = 0; i < (SCORE_VIEW_COLS / 2) + 1; ++i)
	{
		str_pos += sprintf(str + str_pos, "%s", BOX_SEQS[10]);
	}

	str_pos += sprintf(str + str_pos, "%s", BOX_SEQS[6]);

	str[str_pos] = '\0';
	return str;
}

static int handle_bottom_collision(Game *game)
{
	int num_of_rows_removed = remove_full_rows(game->tetris);
	if (num_of_rows_removed)
	{
		update_score(game, num_of_rows_removed);
		update_screen(game);
		usleep(400000);
		remove_empty_rows(game->tetris);
	}
	return (add_new_tetromino(game->tetris) != 0);
}

static void update_score(Game *game, int num_of_rows_removed)
{
	switch (num_of_rows_removed)
	{
	case 1:
		game->score += 40;
		break;
	case 2:
		game->score += 100;
		break;
	case 3:
		game->score += 300;
		break;
	case 4:
		game->score += 1200;
		break;
	}
}
