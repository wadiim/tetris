#include "tetromino.h"
#include "tetris.h"
#include "term.h"

#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

static void game_loop(Tetris *tetris);
static void redraw_screen(Tetris *tetris);
static int handle_bottom_colision(Tetris *tetris);
static void handle_signal(int signal);

int main()
{
	Tetris tetris;

	srand(time(NULL));

	initialize_tetris(&tetris, WIDTH, HEIGHT);
	add_new_tetromino(&tetris);

	switch_to_alternate_buffer();
	atexit(switch_to_normal_buffer);
	switch_to_raw_mode();
	atexit(switch_to_cooked_mode);
	hide_cursor();
	atexit(show_cursor);
	set_window_title("Tetris");
	init_sigaction();
	create_signal_handler(SIGWINCH, &handle_signal);

	game_loop(&tetris);

	terminate_tetris(&tetris);
	return 0;
}

void game_loop(Tetris *tetris)
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
			move_active_tetromino_left(tetris);
			break;
		case 'l':
		case ARROW_RIGHT:
			move_active_tetromino_right(tetris);
			break;
		case 'j':
		case ARROW_DOWN:
			rotate_active_tetromino_clockwise(tetris);
			break;
		case 'k':
		case ARROW_UP:
			rotate_active_tetromino_anticlockwise(tetris);
			break;
		case ENTER:
			drop_active_tetromino(tetris);
			break;
		}

		if (input == ' ' || ((float)(clock() - start) / CLOCKS_PER_SEC) > 0.0004)
		{
			if (move_active_tetromino_down(tetris) == 0
				&& handle_bottom_colision(tetris) == 0)
			{
				return;
			}
			start = clock();
		}

		redraw_screen(tetris);
	}
}

void redraw_screen(Tetris *tetris)
{
	char *str = tetris_to_str(tetris);
	set_cursor_position(0, 0);
	write(STDOUT_FILENO, str, strlen(str));
	free(str);
}

void handle_signal(int signal)
{
	switch (signal)
	{
	case SIGWINCH:
		clear_screen();
		break;
	}
}

static int handle_bottom_colision(Tetris *tetris)
{
	int first_removed = remove_full_rows(tetris);
	if (first_removed != -1)
	{
		redraw_screen(tetris);
		usleep(400000);
		remove_empty_rows(tetris, first_removed);
	}
	return (add_new_tetromino(tetris) != 0);
}
