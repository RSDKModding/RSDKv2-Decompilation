#include "RetroEngine.hpp"
#include <math.h>
#include <time.h>

#ifndef RSDK_PI
#define RSDK_PI 3.1415927f
#endif

int SinValue512[512];
int CosValue512[512];

int SinValue256[256];
int CosValue256[256];

void CalculateTrigAngles() {
#if !RETRO_USE_ORIGINAL_CODE
    srand(time(NULL));
#endif

    for (int i = 0; i < 0x200; ++i) {
        float Val      = sinf(((float)i / 256) * RSDK_PI);
        SinValue512[i] = (signed int)(Val * 512.0);
        Val            = cosf(((float)i / 256) * RSDK_PI);
        CosValue512[i] = (signed int)(Val * 512.0);
    }

    CosValue512[0]   = 0x200;
    CosValue512[128] = 0;
    CosValue512[256] = -0x200;
    CosValue512[384] = 0;
    SinValue512[0]   = 0;
    SinValue512[128] = 0x200;
    SinValue512[256] = 0;
    SinValue512[384] = -0x200;

    for (int i = 0; i < 0x100; ++i) {
        SinValue256[i] = (SinValue512[i * 2] >> 1);
        CosValue256[i] = (CosValue512[i * 2] >> 1);
    }
}