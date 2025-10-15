// 03_gravity_param.c
#include <stdio.h>

typedef struct { int h,w; int a[16][16]; } Board;

static void gravity_down(Board *b){
    for(int c=0;c<b->w;c++){
        int write=0;
        for(int r=0;r<b->h;r++){ // 下(0)→上(h-1)の順に詰める
            if(b->a[r][c]>=0){ int v=b->a[r][c]; b->a[r][c]=-1; b->a[write++][c]=v; }
        }
    }
}

static void print_board(Board *b){
    for(int r=b->h-1;r>=0;r--){ for(int c=0;c<b->w;c++) printf("%2d ", b->a[r][c]); puts(""); }
    puts("---");
}

int main(void){
    Board B={.h=6,.w=6};
    for(int r=0;r<B.h;r++)for(int c=0;c<B.w;c++) B.a[r][c]=-1;
    // ばら撒き
    B.a[5][2]=1; B.a[4][2]=2; B.a[3][2]=-1; B.a[2][2]=3; B.a[1][2]=4;
    puts("before:"); print_board(&B);
    gravity_down(&B);
    puts("after:"); print_board(&B);
    return 0;
}

