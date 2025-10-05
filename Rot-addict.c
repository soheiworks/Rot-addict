#include <stdio.h>
#ifdef _WIN32
 #include <windows.h>
#else
 #include <unistd.h>
#endif

int main(void){
    /* 変数は関数の先頭で宣言（ユーザー希望に合わせて） */
    int piece;
    int row;
    int col;
    int frame_ms;
    int height;
    int width;
    int top;      /* 盤面の表示開始行（上余白） */
    int left;     /* 盤面の表示開始列（左余白） */
    int p;        /* ピース番号 0..63 */
    int r;        /* 盤面の論理y（0=最下段, 7=最上段） */
    int c;        /* 盤面の論理x（0..7） */
    int cur;      /* 落下中の現在高さ(論理y) */
    int target_r; /* このピースの着地先（論理y） */
    int target_c; /* このピースの着地先（論理x） */
    int board[8][8]; /* 配置済みブロック 0/1 */
    int i, j;

    /* 設定 */
    height   = 8;
    width    = 8;
    top      = 2;    /* 画面上の少し下から描く */
    left     = 10;   /* 画面左から少し右に寄せる */
    frame_ms = 120;  /* フレーム間隔（お好みで） */

    /* 盤面初期化 */
    for(i = 0; i < height; i++){
        for(j = 0; j < width; j++){
            board[i][j] = 0;
        }
    }

    /* カーソル非表示 */
    printf("\x1b[?25l");
    fflush(stdout);

    /* ピースを左上→右へ、上の段が埋まったら次の段…の順で64個落とす */
    for(p = 0; p < width * height; p++){
        target_c = p % width;           /* 0..7（左→右） */
        target_r = p / width;           /* 0..7（下→上に積む） */

        /* 下から数える論理yを画面行へ変換する関数的な計算
           画面行 = top + (height - 1 - 論理y)
         */

        /* 落下アニメーション：上(論理y=0)から着地点(target_r)まで1マスずつ降ろす */
        for(cur = 0; cur <= target_r; cur++){
            /* 画面クリア＆カーソルホーム */
            printf("\x1b[2J");
            printf("\x1b[H");

            /* 既に着地済みのブロックを描画 */
            for(r = 0; r < height; r++){
                for(c = 0; c < width; c++){
                    if(board[r][c]){
                        /* 全角「■」は等幅2桁相当なので列は2ずつ進める */
                        printf("\x1b[%d;%dH", top + (height - 1 - r), left + c * 2);
                        printf("■");
                    }
                }
            }

            /* 落下中のブロックを描画 */
            printf("\x1b[%d;%dH",
                   top + (height - 1 - cur),
                   left + target_c * 2);
            printf("■");
            fflush(stdout);

            /* スリープ */
            #ifdef _WIN32
                Sleep(frame_ms);
            #else
                usleep(frame_ms * 1000);
            #endif
        }

        /* 着地確定 */
        board[target_r][target_c] = 1;
    }

    /* 最終表示を残してカーソル再表示 */
    printf("\x1b[?25h");

    printf("\n落下デモ終了。Enterで終了します。\n");
    fflush(stdout);
    piece = getchar();
    (void)piece;

    return 0;
}

