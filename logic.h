#pragma once
#include "board.h"

int inb(Board *b, int r, int c);
void board_init_random(Board *b, int num_types);

int mark_matches(Board *b, int mark[16][16], int min_len);
void apply_erase(Board *b, GameConfig *cfg, int mark[16][16]);
void gravity_down(Board *b, GameConfig *cfg);
int  has_empty(Board *b);
void refill_new_from_top(Board *b, GameConfig *cfg);
void resolve_all_chains(Board *b, GameConfig *cfg, int match_min);

