#include "tetromino.h"
#include "tetris.h"
#include "term.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

void game_loop(Tetris *tetris);
void redraw_screen(Tetris *tetris);

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
		case ' ':
			move_active_tetromino_down(tetris);
			break;
		}
		redraw_screen(tetris);

		if (((float)(clock() - start) / CLOCKS_PER_SEC) > 0.0004)
		{
			if (move_active_tetromino_down(tetris) == 0
				&& add_new_tetromino(tetris) == 0)
			{
				return;
			}
			redraw_screen(tetris);
			start = clock();
		}
	}
}

void redraw_screen(Tetris *tetris)
{
		char *str = tetris_to_str(tetris);
		set_cursor_position(0, 0);
		write(STDOUT_FILENO, str, strlen(str));
		free(str);
}
