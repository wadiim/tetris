#include "term.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

struct termios orig_termios;

static int get_char(void *c);

void switch_to_alternate_buffer(void)
{
	if (write(STDOUT_FILENO, "\x1b[?1049h\x1b[H", 11) != 11)
	{
		perror("Failed to switch to alternate buffer");
		exit(errno);
	}
}

void switch_to_normal_buffer(void)
{
	if (write(STDOUT_FILENO, "\x1b[?1049l", 8) != 8)
	{
		perror("Failed to switch to normal buffer");
		exit(errno);
	}
}

void hide_cursor(void)
{
	if (write(STDOUT_FILENO, "\x1b[?25l", 6) != 6)
	{
		perror("Failed to hide cursor");
		exit(errno);
	}
}

void show_cursor(void)
{
	if (write(STDOUT_FILENO, "\x1b[?25h", 6) != 6)
	{
		perror("Failed to show cursor");
		exit(errno);
	}
}

void get_cursor_position(int *x, int *y)
{
	unsigned i = 0;
	char buf[32];

	if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
	{
		perror("Failed to get cursor position");
		exit(errno);
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
			perror("Failed to get cursor position");
			exit(errno);
		}
		--(*x);
	}
	if (y != NULL)
	{
		if (sscanf(&buf[2], "%i", y) != 1)
		{
			perror("Failed to get cursor position");
			exit(errno);
		}
		--(*y);
	}
}

void set_cursor_position(int x, int y)
{
	char buf[32] = "\0";
	if (sprintf(buf, "\x1b[%i;%iH", y + 1, x + 1) < 6
		|| write(STDOUT_FILENO, buf, sizeof(buf)) == -1)
	{
		perror("Failed to set cursor position");
		exit(errno);
	}
}

void set_window_title(const char *title)
{
	char buf[64] = "\0";
	if (sprintf(buf, "\x1b]0;%s\x7", title) < 0
		|| write(STDOUT_FILENO, buf, sizeof(buf)) == -1)
	{
		perror("Failed to set window title");
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
			perror("Failed to get window size");
			exit(errno);
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
		perror("Failed to switch to raw mode");
		exit(errno);
	}
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
	{
		perror("Failed to switch to raw mode");
		exit(errno);
	}
}

void switch_to_cooked_mode(void)
{
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
	{
		perror("Failed to switch to cooked mode");
		exit(errno);
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
		perror("Failed to get input");
		exit(errno);
	}
	return nread;
}
