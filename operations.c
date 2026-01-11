#include <stdlib.h>
#include <time.h>
#include <windows.h>
#define MINIAUDIO_IMPLEMENTATION
#include "include/miniaudio.h"
#include "include/globals.h"


DWORD WINAPI run(LPVOID lpParam) {
    ma_engine engine;
    ma_engine_init(NULL, &engine);
    srand(time(NULL));
    while (appRunning) {
        int delay = rand() % 60;

        Sleep(delay * 1000);
        ma_engine_play_sound(&engine, "sound.wav", NULL);
        Sleep(3000);
    }

    ma_engine_uninit(&engine);
    return 0;
}
