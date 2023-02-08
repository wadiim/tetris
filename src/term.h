#ifndef TERM_H
#define TERM_H

enum KEY
{
	ENTER = 13,
	ESC = 27,
	ARROW_LEFT = 164,
	ARROW_UP,
	ARROW_RIGHT,
	ARROW_DOWN
};

void switch_to_alternate_buffer(void);
void switch_to_normal_buffer(void);
void hide_cursor(void);
void show_cursor(void);
void set_cursor_position(int x, int y);
void set_window_title(const char *title);
void get_window_size(int *x, int *y);
void switch_to_raw_mode(void);
void switch_to_cooked_mode(void);
int get_input(void);
void clear_screen(void);
void init_sigaction(void);
void create_signal_handler(int signal, void (*handler)(int));
void destroy_signal_handler(int signal);

#endif
