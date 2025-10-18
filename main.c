#define _POSIX_C_SOURCE 199309L
#include <stdlib.h>
#include <time.h>
#include <locale.h>
#include <stdio.h>
#include "board.h"
#include "logic.h"

#ifdef _WIN32
  #include <windows.h>
  #include <conio.h>
#else
  #include <unistd.h>
  #include <termios.h>
#endif

#define HSIZE 8
#define WSIZE 8
#define MATCH_MIN 4
#define SPEED_MULT 8
static const char* PIECES[] = { "[]", "##", "()", "<>" };
static const int   NUM_TYPES = 4;

enum Key { K_NONE=0, K_UP, K_DOWN, K_LEFT, K_RIGHT, K_SPACE, K_Q, K_H, K_J, K_K, K_L };

#ifndef _WIN32
static struct termios oldt, newt;
static void enable_raw_mode(void){ tcgetattr(STDIN_FILENO,&oldt); newt=oldt; newt.c_lflag &= ~(ICANON|ECHO); tcsetattr(STDIN_FILENO,TCSANOW,&newt); }
static void disable_raw_mode(void){ tcsetattr(STDIN_FILENO,TCSANOW,&oldt); }
static int read_byte(void){ unsigned char ch=0; if(read(STDIN_FILENO,&ch,1)==1) return (int)ch; return -1; }
#else
static int read_byte(void){ while(!_kbhit()) Sleep(1); return _getch(); }
#endif

static enum Key read_key(void){
    int a,b,ch=read_byte(); if(ch==-1) return K_NONE;
    if(ch==0x1b){ a=read_byte(); if(a=='['){ b=read_byte();
        if(b=='A')return K_UP; if(b=='B')return K_DOWN; if(b=='C')return K_RIGHT; if(b=='D')return K_LEFT; } return K_NONE; }
#ifdef _WIN32
    if(ch==0xE0||ch==0x00){ b=read_byte(); if(b==72)return K_UP; if(b==80)return K_DOWN; if(b==75)return K_LEFT; if(b==77)return K_RIGHT; return K_NONE; }
#endif
    if(ch=='q'||ch=='Q')return K_Q; if(ch==' ')return K_SPACE;
    if(ch=='w'||ch=='W')return K_UP; if(ch=='s'||ch=='S')return K_DOWN; if(ch=='a'||ch=='A')return K_LEFT; if(ch=='d'||ch=='D')return K_RIGHT;
    if(ch=='h'||ch=='H')return K_H; if(ch=='j'||ch=='J')return K_J; if(ch=='k'||ch=='K')return K_K; if(ch=='l'||ch=='L')return K_L;
    return K_NONE;
}

static int try_swap_and_resolve(Board *b, GameConfig *cfg, int r, int c, int r2, int c2){
    if(!inb(b,r,c)||!inb(b,r2,c2)) return 0;
    if(r==r2 && c==c2) return 0;
    int tmp = b->cells[r][c];
    b->cells[r][c]   = b->cells[r2][c2];
    b->cells[r2][c2] = tmp;
    draw_cell(b,cfg,r,c); draw_cell(b,cfg,r2,c2);
    int mark[16][16];
    if(mark_matches(b,mark,MATCH_MIN)){
        resolve_all_chains(b,cfg,MATCH_MIN); return 1;
    }else{
        tmp = b->cells[r][c];
        b->cells[r][c]   = b->cells[r2][c2];
        b->cells[r2][c2] = tmp;
        draw_cell(b,cfg,r,c); draw_cell(b,cfg,r2,c2);
        return 0;
    }
}

int main(void){
    setlocale(LC_ALL,"");
    srand((unsigned)time(NULL));

    Board board = { .height=HSIZE, .width=WSIZE };
    GameConfig cfg = { .top=2, .left=10, .frame_ms=4/SPEED_MULT, .pieces=PIECES, .num_types=NUM_TYPES };
    if(cfg.frame_ms < 10) cfg.frame_ms = 10;

#ifndef _WIN32
    enable_raw_mode();
#endif

    board_init_random(&board, NUM_TYPES);
    term_clear(); term_show_cursor(0);
    redraw_board(&board,&cfg,0,0,0);
    printf("\x1b[%d;%dH操作: 矢印/WASD=移動, H/J/K/L=スワップ, Space=選択→方向, q=終了",
           cfg.top + board.height + 2, cfg.left);

    int cur_r=0, cur_c=0, selecting=0, sel_r=0, sel_c=0;

    for(;;){
        enum Key k = read_key(); if(k==K_Q) break;

        int nr=cur_r, nc=cur_c;
        if(k==K_UP) nr=(cur_r+1<board.height)?cur_r+1:cur_r;
        if(k==K_DOWN) nr=(cur_r-1>=0)?cur_r-1:cur_r;
        if(k==K_LEFT) nc=(cur_c-1>=0)?cur_c-1:cur_c;
        if(k==K_RIGHT) nc=(cur_c+1<board.width)?cur_c+1:cur_c;
        if(nr!=cur_r || nc!=cur_c){ cur_r=nr; cur_c=nc; redraw_board(&board,&cfg,cur_r,cur_c,selecting); }

        if(k==K_SPACE){ selecting=!selecting; if(selecting){ sel_r=cur_r; sel_c=cur_c; } redraw_board(&board,&cfg,cur_r,cur_c,selecting); }

        if(k==K_H||k==K_J||k==K_K||k==K_L){
            int tr=cur_r, tc=cur_c;
            if(k==K_H) tc=cur_c-1; if(k==K_L) tc=cur_c+1; if(k==K_J) tr=cur_r-1; if(k==K_K) tr=cur_r+1;
            if(inb(&board,tr,tc)){ (void)try_swap_and_resolve(&board,&cfg,cur_r,cur_c,tr,tc); redraw_board(&board,&cfg,cur_r,cur_c,selecting); }
        }

        if(selecting && (k==K_UP||k==K_DOWN||k==K_LEFT||k==K_RIGHT)){
            int tr=sel_r, tc=sel_c;
            if(k==K_LEFT) tc=sel_c-1; if(k==K_RIGHT) tc=sel_c+1; if(k==K_DOWN) tr=sel_r-1; if(k==K_UP) tr=sel_r+1;
            if(inb(&board,tr,tc)){
                if(try_swap_and_resolve(&board,&cfg,sel_r,sel_c,tr,tc)){ selecting=0; cur_r=tr; cur_c=tc; }
                redraw_board(&board,&cfg,cur_r,cur_c,selecting);
            }
        }
    }

    term_show_cursor(1);
#ifndef _WIN32
    disable_raw_mode();
#endif
    printf("\n");
    return 0;
}

