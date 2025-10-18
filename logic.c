#include "logic.h"
#include <stdlib.h>
#include <stdio.h>

int inb(Board *b, int r, int c){ return (r>=0 && r<b->height && c>=0 && c<b->width); }

void board_init_random(Board *b, int num_types){
    for(int r=0;r<b->height;r++)
        for(int c=0;c<b->width;c++)
            b->cells[r][c] = rand() % num_types;
}

int mark_matches(Board *b, int mark[16][16], int min_len){
    int found=0; int dr[4]={0,1,1,1}, dc[4]={1,0,1,-1};
    for(int r=0;r<b->height;r++) for(int c=0;c<b->width;c++) mark[r][c]=0;
    for(int r=0;r<b->height;r++){
        for(int c=0;c<b->width;c++){
            int t=b->cells[r][c]; if(t<0) continue;
            for(int d=0; d<4; d++){
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

void apply_erase(Board *b, GameConfig *cfg, int mark[16][16]){
    for(int r=0;r<b->height;r++)
        for(int c=0;c<b->width;c++)
            if(mark[r][c]){ erase_cell(b,cfg,r,c); b->cells[r][c]=-1; }
}

void gravity_down(Board *b, GameConfig *cfg){
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

int has_empty(Board *b){
    for(int r=0;r<b->height;r++)
        for(int c=0;c<b->width;c++)
            if(b->cells[r][c]<0) return 1;
    return 0;
}

void refill_new_from_top(Board *b, GameConfig *cfg){
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

void resolve_all_chains(Board *b, GameConfig *cfg, int match_min){
    int mark[16][16];
    for(;;){
        int changed = mark_matches(b, mark, match_min);
        if(!changed) break;
        apply_erase(b, cfg, mark);
        msleep_x(cfg->frame_ms*3);
        gravity_down(b, cfg);
        msleep_x(cfg->frame_ms*3);
        while(has_empty(b)){
            refill_new_from_top(b, cfg);
            if(mark_matches(b, mark, match_min)){
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

