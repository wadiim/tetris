#include "game.h"
#include "term.h"

#include <time.h>
#include <stdlib.h>
#include <signal.h>

static void handle_signal(int signal);

int main()
{
	Game game;

	srand(time(NULL));

	initialize_game(&game);

	switch_to_alternate_buffer();
	atexit(switch_to_normal_buffer);
	switch_to_raw_mode();
	atexit(switch_to_cooked_mode);
	hide_cursor();
	atexit(show_cursor);
	set_window_title("Tetris");
	init_sigaction();
	create_signal_handler(SIGWINCH, &handle_signal);

	game_loop(&game);

	terminate_game(&game);
	return 0;
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
