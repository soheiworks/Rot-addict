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

    rows = 12;
    col = 10;
    frame_ms = 120;

    printf("\x1b[?25l");
    fflush(stdout);

    for(row = 1; row <= rows; row++){

        printf("\x1b[2J");
        printf("\x1b[H");
        printf("\x1b[%d;%dH",row,col);
        printf("■");
        fflush(stdout);

        #ifdef _WIN32
            Sleep(frame_ms);
        #else
            usleep(frame_ms * 1000);
        #endif
   }

    printf("\x1b[2J");
    printf("\x1b[H");
    printf("\x1b[?25h");

    printf("落下デモ終了。Enterで終了します。\n");
    fflush(stdout);
    piece = getchar();
    (void)piece;

    return 0;

}

