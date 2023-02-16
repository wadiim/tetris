#include "term.h"
#include "utils.h"

#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

static struct termios orig_termios;
static struct sigaction sa;

static int get_char(void *c);
static void get_cursor_position(int *x, int *y);

void switch_to_alternate_buffer(void)
{
	if (write(STDOUT_FILENO, "\x1b[?1049h\x1b[H", 11) != 11)
	{
		die("Failed to switch to alternate buffer");
	}
}

void switch_to_normal_buffer(void)
{
	if (write(STDOUT_FILENO, "\x1b[?1049l", 8) != 8)
	{
		die("Failed to switch to normal buffer");
	}
}

void hide_cursor(void)
{
	if (write(STDOUT_FILENO, "\x1b[?25l", 6) != 6)
	{
		die("Failed to hide cursor");
	}
}

void show_cursor(void)
{
	if (write(STDOUT_FILENO, "\x1b[?25h", 6) != 6)
	{
		die("Failed to show cursor");
	}
}

void set_window_title(const char *title)
{
	char buf[64] = "\0";
	if (sprintf(buf, "\x1b]0;%s\x7", title) < 0
		|| write(STDOUT_FILENO, buf, sizeof(buf)) == -1)
	{
		die("Failed to set window title");
	}
}

void get_window_size(int *x, int *y)
{
	struct winsize ws;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1
			|| ws.ws_col == 0)
	{
		if (write(STDOUT_FILENO,
			"\x1b[999C\x1b[999B", 12) != 12)
		{
			die("Failed to get window size");
		}
		get_cursor_position(x, y);
		if (x != NULL) ++(*x);
		if (y != NULL) ++(*y);
	}
	else
	{
		if (x != NULL) *x = ws.ws_col;
		if (y != NULL) *y = ws.ws_row;
	}
}

void switch_to_raw_mode(void)
{
	struct termios raw = orig_termios;
	if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
	{
		die("Failed to switch to raw mode");
	}
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
	{
		die("Failed to switch to raw mode");
	}
}

void switch_to_cooked_mode(void)
{
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
	{
		die("Failed to switch to cooked mode");
	}
}

int get_input(void)
{
	unsigned char c, buf[2] = { 0 };
	if (get_char(&c) != 1) return -1;

	if (c == ESC)
	{
		if (get_char(&buf[0]) == -1) return ESC;
		if (get_char(&buf[1]) == -1) return ESC;

		switch (buf[1])
		{
			case 'A': return ARROW_UP;
			case 'B': return ARROW_DOWN;
			case 'C': return ARROW_RIGHT;
			case 'D': return ARROW_LEFT;
		}

		return ESC;
	}
	else
	{
		return c;
	}
}

int get_char(void *c)
{
	int nread = read(STDIN_FILENO, c, 1);
	if (nread == -1 && errno != EAGAIN)
	{
		die("Failed to get input");
	}
	return nread;
}

void get_cursor_position(int *x, int *y)
{
	unsigned i = 0;
	char buf[32];

	if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
	{
		die("Failed to get cursor position");
	}

	while (i < sizeof(buf) - 1)
	{
		if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
		if (buf[i] == 'R') break;
		++i;
	}
	buf[i] = '\0';
	if (x != NULL)
	{
		if (sscanf(&buf[4], "%i", x) != 1)
		{
			die("Failed to get cursor position");
		}
		--(*x);
	}
	if (y != NULL)
	{
		if (sscanf(&buf[2], "%i", y) != 1)
		{
			die("Failed to get cursor position");
		}
		--(*y);
	}
}

void clear_screen(void)
{
	if (write(STDOUT_FILENO, "\x1b[2J", 4) != 4
		|| write(STDOUT_FILENO, "\x1b[H", 3) != 3)
	{
		die("Failed to clear screen");
	}
}

void init_sigaction(void)
{
	sa.sa_flags = SA_RESTART;
	sigfillset(&sa.sa_mask);
}

void create_signal_handler(int signal, void (*handler)(int))
{
	sa.sa_handler = handler;
	if (sigaction(signal, &sa, NULL) == -1)
	{
		die("Failed to create signal handler");
	}
}

void destroy_signal_handler(int signal)
{
	sa.sa_handler = SIG_DFL;
	if (sigaction(signal, &sa, NULL) == -1)
	{
		die("Failed to destroy signal handler");
	}
}
