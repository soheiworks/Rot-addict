#include <stdio.h> // 標準入出力ライブラリ
              
int main(void){
    int piece; 

    printf("■");
    fflush(stdout);

piece = getchar();
printf("\x1b[G");
printf("\x1b[2K");

printf("\x1b[F");
printf("\x1b[2K");
printf("\x1b[G");

fflush(stdout);
printf("　");
fflush(stdout);
getchar();
return 0;
}
