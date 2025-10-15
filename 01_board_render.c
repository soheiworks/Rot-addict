// 01_board_render.c
#include <stdio.h>
#include <locale.h>

typedef struct {
    int height, width;
    int cells[16][16]; // -1: empty, 0..: type
} Board;

typedef struct {
    int top, left, frame_ms;
    const char **pieces;
    int num_types;
} GameConfig;

static void draw_cell(Board *b, GameConfig *cfg, int r, int c) {
    int term_r = cfg->top + (b->height - 1 - r);
    int term_c = cfg->left + c * 2;
    printf("\x1b[%d;%dH  ", term_r, term_c);
    if (b->cells[r][c] >= 0) {
        printf("\x1b[%d;%dH%s", term_r, term_c, cfg->pieces[b->cells[r][c]]);
    }
}

static void redraw(Board *b, GameConfig *cfg){
    for(int r=0;r<b->height;r++){
        for(int c=0;c<b->width;c++) draw_cell(b,cfg,r,c);
    }
}

int main(void){
    setlocale(LC_ALL,"");
    const char* PIECES[] = { "[]", "##", "()", "<>" };
    Board board = { .height=8, .width=8 };
    GameConfig cfg = { .top=2, .left=10, .frame_ms=30, .pieces=PIECES, .num_types=4 };

    // 初期化：チェッカー模様
    for(int r=0;r<board.height;r++){
        for(int c=0;c<board.width;c++){
            board.cells[r][c] = ((r+c)%2)? (c%4) : -1;
        }
    }

    printf("\x1b[2J\x1b[H\x1b[?25l");
    redraw(&board,&cfg);
    printf("\x1b[%d;%dH", cfg.top+board.height+2, cfg.left);
    printf("表示のみデモ。Enterで終了。\n");
    getchar();
    printf("\x1b[?25h");
    return 0;
}

