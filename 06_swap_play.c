// 06_swap_play.c
// 8x8 盤でカーソル移動＋隣接スワップ→連鎖解決（単体実行用）
// 操作: 矢印/WASDで移動, H/J/K/Lで隣とスワップ, Spaceで選択→方向, q終了
// 文字幅対策: ASCIIピース使用（[] ## () <>）

#define _POSIX_C_SOURCE 199309L  /* nanosleep 用。必ず time.h より前 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>

#ifdef _WIN32
  #include <windows.h>
  #include <conio.h>
#else
  #include <unistd.h>
  #include <termios.h>
  #include <fcntl.h>
#endif

/* ===== ユーザ設定 ===== */
#define HSIZE 8
#define WSIZE 8
#define MATCH_MIN 4
#define SPEED_MULT 8

/* ===== ピース定義（ANSIなしのASCII。全角崩れ回避）===== */
static const char* PIECES[] = { "[]", "##", "()", "<>" };
static const int   NUM_TYPES = 4;

/* ===== 構造体 ===== */
typedef struct {
    int height;
    int width;
    int cells[16][16]; /* -1: empty, >=0: type */
} Board;

typedef struct {
    int top;
    int left;
    int frame_ms;
    const char **pieces;
    int num_types;
} GameConfig;

/* ===== 端末IO（カーソル/色） ===== */
static void term_clear(void){
    printf("\x1b[2J\x1b[H");
}
static void term_show_cursor(int show){
    printf("%s", show ? "\x1b[?25h" : "\x1b[?25l");
}
static void msleep_x(int ms){
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

/* ===== 入力（単キー） ===== */
enum Key {
    K_NONE=0, K_UP, K_DOWN, K_LEFT, K_RIGHT, K_SPACE, K_Q, K_H, K_J, K_K, K_L
};

#ifndef _WIN32
static struct termios oldt, newt;
static void enable_raw_mode(void){
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}
static void disable_raw_mode(void){
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}
#endif

static int read_byte(void){
#ifdef _WIN32
    int ch;
    while(!_kbhit()) { Sleep(1); }
    ch = _getch();
    return ch;
#else
    unsigned char ch=0;
    if(read(STDIN_FILENO, &ch, 1) == 1) return (int)ch;
    return -1;
#endif
}

static enum Key read_key(void){
    int a, b, ch;
    ch = read_byte();
    if(ch == -1) return K_NONE;

    /* 矢印キー: ESC [ A/B/C/D */
    if(ch == 0x1b){
        a = read_byte();
        if(a == '['){
            b = read_byte();
            if(b == 'A') return K_UP;
            if(b == 'B') return K_DOWN;
            if(b == 'C') return K_RIGHT;
            if(b == 'D') return K_LEFT;
        }
        return K_NONE;
    }

#ifdef _WIN32
    if(ch == 0xE0 || ch == 0x00){
        b = read_byte();
        if(b == 72) return K_UP;
        if(b == 80) return K_DOWN;
        if(b == 75) return K_LEFT;
        if(b == 77) return K_RIGHT;
        return K_NONE;
    }
#endif

    if(ch == 'q' || ch == 'Q') return K_Q;
    if(ch == ' ') return K_SPACE;
    if(ch == 'w' || ch == 'W') return K_UP;
    if(ch == 's' || ch == 'S') return K_DOWN;
    if(ch == 'a' || ch == 'A') return K_LEFT;
    if(ch == 'd' || ch == 'D') return K_RIGHT;
    if(ch == 'h' || ch == 'H') return K_H;
    if(ch == 'j' || ch == 'J') return K_J;
    if(ch == 'k' || ch == 'K') return K_K;
    if(ch == 'l' || ch == 'L') return K_L;
    return K_NONE;
}

/* ===== 盤面操作 ===== */
static void board_init_random(Board *b, int num_types){
    int r, c;
    for(r = 0; r < b->height; r++){
        for(c = 0; c < b->width; c++){
            b->cells[r][c] = rand() % num_types;
        }
    }
}
static int inb(Board *b, int r, int c){
    return (r >= 0 && r < b->height && c >= 0 && c < b->width);
}

/* ===== 描画 ===== */
static void draw_cell(Board *b, GameConfig *cfg, int r, int c){
    int tr = cfg->top + (b->height - 1 - r);
    int tc = cfg->left + c * 2;
    printf("\x1b[%d;%dH  ", tr, tc);
    if(b->cells[r][c] >= 0){
        printf("\x1b[%d;%dH%s", tr, tc, cfg->pieces[b->cells[r][c]]);
    }
}
static void erase_cell(Board *b, GameConfig *cfg, int r, int c){
    int tr = cfg->top + (b->height - 1 - r);
    int tc = cfg->left + c * 2;
    printf("\x1b[%d;%dH  ", tr, tc);
}
static void redraw_board(Board *b, GameConfig *cfg, int cur_r, int cur_c, int selecting){
    int r, c, tr, tc;
    for(r = 0; r < b->height; r++){
        for(c = 0; c < b->width; c++){
            draw_cell(b, cfg, r, c);
        }
    }
    tr = cfg->top + (b->height - 1 - cur_r);
    tc = cfg->left + cur_c * 2;

    /* ←ここを修正：選択中は反転＋下線、通常は反転のみ */
    if (selecting) {
        printf("\x1b[%d;%dH\x1b[7;4m%s\x1b[0m", tr, tc,
               (b->cells[cur_r][cur_c] >= 0) ? cfg->pieces[b->cells[cur_r][cur_c]] : "  ");
    } else {
        printf("\x1b[%d;%dH\x1b[7m%s\x1b[0m", tr, tc,
               (b->cells[cur_r][cur_c] >= 0) ? cfg->pieces[b->cells[cur_r][cur_c]] : "  ");
    }
}

/* ===== マッチ検出→消去→重力→補充 ===== */
static int mark_matches(Board *b, int mark[16][16], int min_len){
    int r, c, d, found = 0;
    int dr[4]={0,1,1,1}, dc[4]={1,0,1,-1};

    for(r = 0; r < b->height; r++)
        for(c = 0; c < b->width; c++)
            mark[r][c] = 0;

    for(r = 0; r < b->height; r++){
        for(c = 0; c < b->width; c++){
            int t = b->cells[r][c];
            if(t < 0) continue;
            for(d = 0; d < 4; d++){
                int pr=r-dr[d], pc=c-dc[d];
                if(inb(b,pr,pc) && b->cells[pr][pc]==t) continue;
                int cnt=0, rr=r, cc=c;
                while(inb(b,rr,cc) && b->cells[rr][cc]==t){ cnt++; rr+=dr[d]; cc+=dc[d]; }
                if(cnt>=min_len){
                    found=1; rr=r; cc=c;
                    for(int k=0;k<cnt;k++){ mark[rr][cc]=1; rr+=dr[d]; cc+=dc[d]; }
                }
            }
        }
    }
    return found;
}

static void apply_erase(Board *b, GameConfig *cfg, int mark[16][16]){
    for(int r=0;r<b->height;r++)
        for(int c=0;c<b->width;c++)
            if(mark[r][c]){ erase_cell(b,cfg,r,c); b->cells[r][c]=-1; }
}

static void gravity_down(Board *b, GameConfig *cfg){
    for(int c=0;c<b->width;c++){
        int tmp[16]; for(int r=0;r<b->height;r++) tmp[r]=-1;
        int write_idx=0;
        for(int r=0;r<b->height;r++) if(b->cells[r][c]>=0) tmp[write_idx++]=b->cells[r][c];
        for(int r=0;r<b->height;r++){
            int val = (r<write_idx)? tmp[r]: -1;
            if(b->cells[r][c]!=val){
                if(b->cells[r][c]>=0) erase_cell(b,cfg,r,c);
                b->cells[r][c]=val;
                if(val>=0) draw_cell(b,cfg,r,c);
                msleep_x(cfg->frame_ms);
            }
        }
    }
}

static int has_empty(Board *b){
    for(int r=0;r<b->height;r++)
        for(int c=0;c<b->width;c++)
            if(b->cells[r][c]<0) return 1;
    return 0;
}

static void refill_new_from_top(Board *b, GameConfig *cfg){
    for(int c=0;c<b->width;c++){
        int filled=0;
        for(int r=0;r<b->height;r++) if(b->cells[r][c]>=0) filled++;
        for(int r=filled;r<b->height;r++){
            int tkind = rand()%cfg->num_types;
            const char* pstr = cfg->pieces[tkind];
            int prev_drawn=0, prev_r=0;
            for(int cur=b->height-1; cur>=r; cur--){
                int tr = cfg->top + (b->height - 1 - cur);
                int tc = cfg->left + c * 2;
                if(prev_drawn){
                    int pr = cfg->top + (b->height - 1 - prev_r);
                    printf("\x1b[%d;%dH  ", pr, tc);
                }
                printf("\x1b[%d;%dH  ", tr, tc);
                printf("\x1b[%d;%dH%s", tr, tc, pstr);
                msleep_x(cfg->frame_ms);
                prev_drawn=1; prev_r=cur;
            }
            b->cells[r][c]=tkind;
        }
    }
}

static void resolve_all_chains(Board *b, GameConfig *cfg){
    int mark[16][16];
    for(;;){
        int changed = mark_matches(b, mark, MATCH_MIN);
        if(!changed) break;
        apply_erase(b, cfg, mark);
        msleep_x(cfg->frame_ms*3);
        gravity_down(b, cfg);
        msleep_x(cfg->frame_ms*3);
        while(has_empty(b)){
            refill_new_from_top(b, cfg);
            if(mark_matches(b, mark, MATCH_MIN)){
                apply_erase(b, cfg, mark);
                msleep_x(cfg->frame_ms*3);
                gravity_down(b, cfg);
                msleep_x(cfg->frame_ms*3);
            }else{
                break;
            }
        }
    }
}

/* ===== スワップ処理 ===== */
static int inb2(Board *b,int r,int c){ return inb(b,r,c); }

static int try_swap_and_resolve(Board *b, GameConfig *cfg, int r, int c, int r2, int c2){
    if(!inb2(b,r,c)||!inb2(b,r2,c2)) return 0;
    if(r==r2 && c==c2) return 0;

    int tmp = b->cells[r][c];
    b->cells[r][c]   = b->cells[r2][c2];
    b->cells[r2][c2] = tmp;

    draw_cell(b,cfg,r,c);
    draw_cell(b,cfg,r2,c2);

    int mark[16][16];
    if(mark_matches(b,mark,MATCH_MIN)){
        resolve_all_chains(b,cfg);
        return 1;
    }else{
        tmp = b->cells[r][c];
        b->cells[r][c]   = b->cells[r2][c2];
        b->cells[r2][c2] = tmp;
        draw_cell(b,cfg,r,c);
        draw_cell(b,cfg,r2,c2);
        return 0;
    }
}

/* ===== メイン ===== */
int main(void){
    setlocale(LC_ALL, "");

    Board board = { .height=HSIZE, .width=WSIZE };
    GameConfig cfg;
    cfg.top = 2; cfg.left = 10;
    cfg.frame_ms = 4 / SPEED_MULT; if(cfg.frame_ms < 10) cfg.frame_ms = 10;
    cfg.pieces = PIECES; cfg.num_types = NUM_TYPES;

    srand((unsigned)time(NULL));
    board_init_random(&board, NUM_TYPES);

#ifndef _WIN32
    enable_raw_mode();
#endif

    term_clear();
    term_show_cursor(0);
    redraw_board(&board, &cfg, 0, 0, 0);
    printf("\x1b[%d;%dH操作: 矢印/WASD=移動, H/J/K/L=隣とスワップ, Space=選択→方向, q=終了",
           cfg.top + board.height + 2, cfg.left);

    int cur_r=0, cur_c=0;
    int selecting=0, sel_r=0, sel_c=0;

    for(;;){
        enum Key k = read_key();
        if(k == K_Q) break;

        int nr=cur_r, nc=cur_c;
        if(k == K_UP)    nr = (cur_r + 1 < board.height) ? cur_r + 1 : cur_r;
        if(k == K_DOWN)  nr = (cur_r - 1 >= 0)           ? cur_r - 1 : cur_r;
        if(k == K_LEFT)  nc = (cur_c - 1 >= 0)           ? cur_c - 1 : cur_c;
        if(k == K_RIGHT) nc = (cur_c + 1 < board.width)  ? cur_c + 1 : cur_c;

        if(nr!=cur_r || nc!=cur_c){ cur_r=nr; cur_c=nc; redraw_board(&board,&cfg,cur_r,cur_c,selecting); }

        if(k == K_SPACE){
            selecting = !selecting;
            if(selecting){ sel_r=cur_r; sel_c=cur_c; }
            redraw_board(&board,&cfg,cur_r,cur_c,selecting);
        }

        if(k == K_H || k == K_J || k == K_K || k == K_L){
            int tr=cur_r, tc=cur_c;
            if(k==K_H) tc=cur_c-1;
            if(k==K_L) tc=cur_c+1;
            if(k==K_J) tr=cur_r-1;
            if(k==K_K) tr=cur_r+1;
            if(inb(&board,tr,tc)){
                (void)try_swap_and_resolve(&board,&cfg,cur_r,cur_c,tr,tc);
                redraw_board(&board,&cfg,cur_r,cur_c,selecting);
            }
        }

        if(selecting && (k==K_UP||k==K_DOWN||k==K_LEFT||k==K_RIGHT)){
            int tr=sel_r, tc=sel_c;
            if(k==K_LEFT)  tc=sel_c-1;
            if(k==K_RIGHT) tc=sel_c+1;
            if(k==K_DOWN)  tr=sel_r-1;
            if(k==K_UP)    tr=sel_r+1;
            if(inb(&board,tr,tc)){
                if(try_swap_and_resolve(&board,&cfg,sel_r,sel_c,tr,tc)){
                    selecting=0; cur_r=tr; cur_c=tc;
                }
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

