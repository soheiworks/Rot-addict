// 05_fps_loop.c
// 60FPS相当の固定フレーム雛型（Windows / POSIX 両対応）

#define _POSIX_C_SOURCE 199309L  /* nanosleep を有効化（必ず time.h より前） */

#include <stdio.h>
#include <time.h>

#ifdef _WIN32
  #include <windows.h>
#else
  #include <unistd.h>
#endif

static void sleep_ms(int ms){
#ifdef _WIN32
    if(ms < 0) ms = 0;
    Sleep(ms);
#else
    if(ms < 0) ms = 0;
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
#endif
}

int main(void){
    const int frame_ms = 1000 / 60;  /* 約60FPS */
    for(int frame = 0; frame < 180; ++frame){
        clock_t start = clock();

        /* ここに update()/render() を入れる想定 */
        printf("\rframe=%d", frame);
        fflush(stdout);

        long elapsed = (long)((clock() - start) * 1000 / CLOCKS_PER_SEC);
        int wait = frame_ms - (int)elapsed;
        if(wait > 0) sleep_ms(wait);
    }
    puts("\nend");
    return 0;
}

