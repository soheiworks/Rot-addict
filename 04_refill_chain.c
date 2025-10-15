// 04_refill_chain.c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct { int h,w; int a[16][16]; } Board;

static int inb(Board *b,int r,int c){ return (r>=0&&r<b->h&&c>=0&&c<b->w); }

static int mark_matches(Board *b, int mark[16][16], int min_len){
    int found=0; int dr[4]={0,1,1,1}, dc[4]={1,0,1,-1};
    for(int r=0;r<b->h;r++) for(int c=0;c<b->w;c++) mark[r][c]=0;
    for(int r=0;r<b->h;r++)for(int c=0;c<b->w;c++){
        int t=b->a[r][c]; if(t<0) continue;
        for(int d=0; d<4; d++){
            int pr=r-dr[d], pc=c-dc[d];
            if(inb(b,pr,pc) && b->a[pr][pc]==t) continue;
            int cnt=0, rr=r, cc=c;
            while(inb(b,rr,cc)&&b->a[rr][cc]==t){ cnt++; rr+=dr[d]; cc+=dc[d]; }
            if(cnt>=min_len){ found=1; rr=r; cc=c; for(int k=0;k<cnt;k++){ mark[rr][cc]=1; rr+=dr[d]; cc+=dc[d]; } }
        }
    }
    return found;
}

static void apply_erase(Board *b,int mark[16][16]){
    for(int r=0;r<b->h;r++) for(int c=0;c<b->w;c++) if(mark[r][c]) b->a[r][c]=-1;
}
static void gravity_down(Board *b){
    for(int c=0;c<b->w;c++){
        int write=0;
        for(int r=0;r<b->h;r++){ if(b->a[r][c]>=0){ int v=b->a[r][c]; b->a[r][c]=-1; b->a[write++][c]=v; } }
    }
}
static int has_empty(Board *b){
    for(int r=0;r<b->h;r++) for(int c=0;c<b->w;c++) if(b->a[r][c]<0) return 1;
    return 0;
}
static void refill(Board *b,int num_types){
    for(int r=b->h-1;r>=0;r--) for(int c=0;c<b->w;c++) if(b->a[r][c]<0) b->a[r][c]=rand()%num_types;
}

static void print_board(Board *b){
    for(int r=b->h-1;r>=0;r--){ for(int c=0;c<b->w;c++) printf("%2d ", b->a[r][c]); puts(""); }
    puts("----");
}

int main(void){
    srand((unsigned)time(NULL));
    Board B={.h=8,.w=8};
    for(int r=0;r<B.h;r++)for(int c=0;c<B.w;c++) B.a[r][c]=rand()%4;
    puts("init:"); print_board(&B);

    int mark[16][16], changed;
    do{
        changed = mark_matches(&B,mark,4);
        if(!changed) break;
        apply_erase(&B,mark);
        gravity_down(&B);
        while(has_empty(&B)){ refill(&B,4); gravity_down(&B); }
        print_board(&B); // 中間状態確認
    }while(changed);

    puts("stable:"); print_board(&B);
    return 0;
}

