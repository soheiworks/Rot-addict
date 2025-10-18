#define _POSIX_C_SOURCE 199309L
#include "board.h"
#include <time.h>

#ifdef _WIN32
  #include <windows.h>
#else
  #include <unistd.h>
#endif

void term_clear(void){ printf("\x1b[2J\x1b[H"); }
void term_show_cursor(int show){ printf("%s", show? "\x1b[?25h":"\x1b[?25l"); }

void msleep_x(int ms){
#ifdef _WIN32
    if(ms<0) ms=0; Sleep(ms);
#else
    if(ms<0) ms=0;
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
#endif
}

void draw_cell(Board *b, GameConfig *cfg, int r, int c){
    int tr = cfg->top + (b->height - 1 - r);
    int tc = cfg->left + c * 2;
    printf("\x1b[%d;%dH  ", tr, tc);
    if(b->cells[r][c] >= 0){
        printf("\x1b[%d;%dH%s", tr, tc, cfg->pieces[b->cells[r][c]]);
    }
}

void erase_cell(Board *b, GameConfig *cfg, int r, int c){
    int tr = cfg->top + (b->height - 1 - r);
    int tc = cfg->left + c * 2;
    printf("\x1b[%d;%dH  ", tr, tc);
}

void redraw_board(Board *b, GameConfig *cfg, int cur_r, int cur_c, int selecting){
    for(int r=0;r<b->height;r++){
        for(int c=0;c<b->width;c++){
            draw_cell(b, cfg, r, c);
        }
    }
    int tr = cfg->top + (b->height - 1 - cur_r);
    int tc = cfg->left + cur_c * 2;
    if (selecting) {
        printf("\x1b[%d;%dH\x1b[7;4m%s\x1b[0m", tr, tc,
               (b->cells[cur_r][cur_c] >= 0) ? cfg->pieces[b->cells[cur_r][cur_c]] : "  ");
    } else {
        printf("\x1b[%d;%dH\x1b[7m%s\x1b[0m", tr, tc,
               (b->cells[cur_r][cur_c] >= 0) ? cfg->pieces[b->cells[cur_r][cur_c]] : "  ");
    }
}

