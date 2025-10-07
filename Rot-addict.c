#include <stdio.h>
#ifdef _WIN32
 #include <windows.h>
#else
 #include <unistd.h>
#endif

int main(void){
    int piece;
    int row;
    int rows;
    int col;
    int frame_ms;

    /* 追加の変数（関数先頭で宣言） */
    int height;
    int width;
    int top;
    int left;
    int p;
    int target_r;
    int target_c;
    int cur;
    int prev_drawn;
    int prev_r;
    int prev_c;

    /* 設定 */
    height   = 8;
    width    = 8;
    top      = 2;     /* 描画開始行 */
    left     = 10;    /* 描画開始列（全角対策で2刻みを使う） */
    frame_ms = 120;   /* 1フレーム待ち */
    rows     = 12;    /* もう使わないが残しておくならOK */
    col      = 10;    /* 同上 */

    /* 初期化：画面を最初だけクリアしてカーソル非表示 */
    printf("\x1b[2J");
    printf("\x1b[H");
    printf("\x1b[?25l");
    fflush(stdout);

    /* 8x8 = 64個を、左上→右→次の段…の順に落とす */
    for(p = 0; p < width * height; p++){
        target_c = p % width;    /* 0..7 （左→右） */
        target_r = p / width;    /* 0..7 （上→下の段番号：0が最上段になるように見えるが、
                                    下から数える論理に合わせて描画変換で吸収） */

        /* 論理r=0を最下段、r=height-1を最上段とみなす。
           画面行 = top + (height - 1 - r)
           ここでは「上から落ちてくる」ので cur を height-1 から target_r まで減らす。 */
        prev_drawn = 0;
        prev_r = 0;
        prev_c = 0;

        for(cur = height - 1; cur >= target_r; cur--){
            /* 直前位置を消す（差分） */
            if(prev_drawn){
                printf("\x1b[%d;%dH",
                       top + (height - 1 - prev_r),
                       left + prev_c * 2);
                printf("  "); /* 全角の消去：半角2つ */
            }

            /* 新しい位置に描く */
            printf("\x1b[%d;%dH",
                   top + (height - 1 - cur),
                   left + target_c * 2);
            printf("■");

            fflush(stdout);

            /* 次フレームまで待機 */
            #ifdef _WIN32
                Sleep(frame_ms);
            #else
                usleep(frame_ms * 1000);
            #endif

            /* 現在位置を記録 */
            prev_drawn = 1;
            prev_r = cur;
            prev_c = target_c;
        }

        /* 着地後はそのまま残す（再描画しない） */
    }

    /* カーソル再表示＋終了待ち */
    printf("\x1b[?25h");
    printf("\n落下デモ終了。Enterで終了します。\n");
    fflush(stdout);
    piece = getchar();
    (void)piece;

    return 0;
}

