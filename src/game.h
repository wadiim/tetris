#ifndef GAME_H
#define GAME_H

struct Tetris;

typedef struct Game
{
	int score;
	struct Tetris *tetris;
} Game;

void initialize_game(Game *game);
void terminate_game(Game *game);

void game_loop(Game *game);

#endif
