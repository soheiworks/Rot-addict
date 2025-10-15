// 02_match_generic.c
#include <stdio.h>

typedef struct { int h,w; int a[16][16]; } Board;

static int inb(Board *b,int r,int c){ return (r>=0&&r<b->h&&c>=0&&c<b->w); }

int mark_matches(Board *b, int mark[16][16], int min_len){
    int found=0; 
    int dr[4]={0,1,1,1}, dc[4]={1,0,1,-1};
    for(int r=0;r<b->h;r++) for(int c=0;c<b->w;c++) mark[r][c]=0;

    for(int r=0;r<b->h;r++){
        for(int c=0;c<b->w;c++){
            int t=b->a[r][c]; if(t<0) continue;
            for(int d=0; d<4; d++){
                int pr=r-dr[d], pc=c-dc[d];
                if(inb(b,pr,pc) && b->a[pr][pc]==t) continue; // 起点でない
                int cnt=0, rr=r, cc=c;
                while(inb(b,rr,cc) && b->a[rr][cc]==t){ cnt++; rr+=dr[d]; cc+=dc[d]; }
                if(cnt>=min_len){
                    found=1; rr=r; cc=c;
                    for(int k=0;k<cnt;k++){ mark[rr][cc]=1; rr+=dr[d]; cc+=dc[d]; }
                }
            }
        }
    }
    return found;
}

static void print_board(Board *b){
    for(int r=b->h-1;r>=0;r--){ for(int c=0;c<b->w;c++) printf("%2d ", b->a[r][c]); puts(""); }
    puts("----");
}

static void print_mark(Board *b,int mark[16][16]){
    for(int r=b->h-1;r>=0;r--){ for(int c=0;c<b->w;c++) printf("%2d ", mark[r][c]); puts(""); }
    puts("----");
}

int main(void){
    Board B={.h=6,.w=7};
    // テスト配置：横/縦/斜めすべて検出できるよう適宜編集可
    for(int r=0;r<B.h;r++)for(int c=0;c<B.w;c++) B.a[r][c]=-1;
    for(int c=1;c<=4;c++) B.a[0][c]=2;        // 横4
    for(int r=1;r<=4;r++) B.a[r][0]=3;        // 縦4
    for(int i=0;i<4;i++) B.a[i][i+2]=5;       // 斜め4

    int mark[16][16];
    puts("BOARD:"); print_board(&B);
    int f=mark_matches(&B,mark,4);
    printf("found=%d\nMARKS:\n",f); print_mark(&B,mark);
    return 0;
}

