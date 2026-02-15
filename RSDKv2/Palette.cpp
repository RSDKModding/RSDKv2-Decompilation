#include "RetroEngine.hpp"

// Palettes (as RGB888 Colours)
uint TilePalette32[PALETTE_SIZE];
uint TilePaletteW32[PALETTE_SIZE];
uint TilePalette32F[PALETTE_SIZE];
uint TilePaletteW32F[PALETTE_SIZE];

// Palettes (as RGB565 Colours)
ushort TilePalette16[PALETTE_SIZE];
ushort TilePaletteW16[PALETTE_SIZE];
ushort TilePalette16F[PALETTE_SIZE];
ushort TilePaletteW16F[PALETTE_SIZE];

// Palettes (as RGB888 Colours)
Colour TilePalette[PALETTE_SIZE];
Colour TilePaletteW[PALETTE_SIZE];
Colour TilePaletteF[PALETTE_SIZE];
Colour TilePaletteWF[PALETTE_SIZE];

int PaletteMode = 0;

void LoadPalette(const char *filePath, int startIndex, int endIndex) {
    FileInfo info;

    if (LoadFile(filePath, &info)) {
        SetFilePosition(3 * startIndex);

        byte colour[3];
        for (int i = startIndex; i < endIndex; ++i) {
            FileRead(&colour, 3);
            SetPaletteEntry(i, colour[0], colour[1], colour[2]);
        }
        CloseFile();
    }
}

void SetFade(byte r, byte g, byte b, ushort a, int start, int end) {
    PaletteMode = 1;
    if (a > 0xFF)
        a = 0xFF;
    if (end <= 0xFF)
        ++end;
    for (int i = start; i < end; ++i) {
        byte red          = (ushort)(r * a + (0xFF - a) * TilePalette[i].r) >> 8;
        byte green        = (ushort)(g * a + (0xFF - a) * TilePalette[i].g) >> 8;
        byte blue         = (ushort)(b * a + (0xFF - a) * TilePalette[i].b) >> 8;
        TilePalette16F[i] = RGB888_TO_RGB565(red, green, blue);
        TilePalette32F[i] = PACK_RGB888(red, green, blue);
        TilePaletteF[i].r = red;
        TilePaletteF[i].g = green;
        TilePaletteF[i].b = blue;

        red                = (ushort)(r * a + (0xFF - a) * TilePaletteW[i].r) >> 8;
        green              = (ushort)(g * a + (0xFF - a) * TilePaletteW[i].g) >> 8;
        blue               = (ushort)(b * a + (0xFF - a) * TilePaletteW[i].b) >> 8;
        TilePaletteW16F[i] = RGB888_TO_RGB565(red, green, blue);
        TilePaletteW32F[i] = PACK_RGB888(red, green, blue);
        TilePaletteWF[i].r = red;
        TilePaletteWF[i].g = green;
        TilePaletteWF[i].b = blue;
    }
}

void SetWaterColour(byte r, byte g, byte b, ushort a) {
    PaletteMode = 1;
    if (a > 0xFF)
        a = 0xFF;
    for (int i = 0; i < PALETTE_SIZE; ++i) {
        byte red          = (ushort)(r * a + (0xFF - a) * TilePalette[i].r) >> 8;
        byte green        = (ushort)(g * a + (0xFF - a) * TilePalette[i].g) >> 8;
        byte blue         = (ushort)(b * a + (0xFF - a) * TilePalette[i].b) >> 8;
        TilePaletteW16[i] = RGB888_TO_RGB565(red, green, blue);
        TilePaletteW32[i] = PACK_RGB888(red, green, blue);
        TilePaletteW[i].r = red;
        TilePaletteW[i].g = green;
        TilePaletteW[i].b = blue;
    }
}

void WaterFlash() {
    PaletteMode = 5;
    for (int i = 0; i < PALETTE_SIZE; ++i) {
        TilePaletteW16F[i] = RGB888_TO_RGB565(0xFF, 0xFF, 0xFF);
        TilePaletteW32F[i] = PACK_RGB888(0xFF, 0xFF, 0xFF);
        TilePaletteWF[i].r = 0xFF;
        TilePaletteWF[i].g = 0xFF;
        TilePaletteWF[i].b = 0xFF;
    }
}
