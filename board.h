#pragma once
#include <stdio.h>

typedef struct {
    int height, width;
    int cells[16][16]; /* -1: empty, >=0: type */
} Board;

typedef struct {
    int top, left;
    int frame_ms;
    const char **pieces;
    int num_types;
} GameConfig;

/* 端末 */
void term_clear(void);
void term_show_cursor(int show);
void msleep_x(int ms);

/* 描画 */
void draw_cell(Board *b, GameConfig *cfg, int r, int c);
void erase_cell(Board *b, GameConfig *cfg, int r, int c);
void redraw_board(Board *b, GameConfig *cfg, int cur_r, int cur_c, int selecting);

