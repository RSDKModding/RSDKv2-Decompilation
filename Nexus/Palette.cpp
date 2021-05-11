#include "RetroEngine.hpp"

// Palettes (as RGB888 Colours)
uint palette32[PALETTE_SIZE];
uint palette32W[PALETTE_SIZE];
uint palette32F[PALETTE_SIZE];
uint palette32WF[PALETTE_SIZE];

// Palettes (as RGB565 Colours)
ushort palette16[PALETTE_SIZE];
ushort palette16W[PALETTE_SIZE];
ushort palette16F[PALETTE_SIZE];
ushort palette16WF[PALETTE_SIZE];

// Palettes (as RGB888 Colours)
Colour palette8[PALETTE_SIZE];
Colour palette8W[PALETTE_SIZE];
Colour palette8F[PALETTE_SIZE];
Colour palette8WF[PALETTE_SIZE];

int fadeMode = 0;

void LoadPalette(const char *filePath, int startIndex, int endIndex)
{
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

void SetFade(byte r, byte g, byte b, ushort a, int start, int end)
{
    fadeMode = 1;
    if (a > 255)
        a = 255;
    if (end < 256)
        ++end;
    for (int i = start; i < end; ++i) {
        byte red     = (ushort)(r * a + (0xFF - a) * palette8[i].r) >> 8;
        byte green   = (ushort)(g * a + (0xFF - a) * palette8[i].g) >> 8;
        byte blue    = (ushort)(b * a + (0xFF - a) * palette8[i].b) >> 8;
        palette16F[i]  = RGB888_TO_RGB565(red, green, blue);
        palette32F[i]  = PACK_RGB888(red, green, blue);
        palette8F[i].r = red;
        palette8F[i].g = green;
        palette8F[i].b = blue;

        red             = (ushort)(r * a + (0xFF - a) * palette8W[i].r) >> 8;
        green           = (ushort)(g * a + (0xFF - a) * palette8W[i].g) >> 8;
        blue            = (ushort)(b * a + (0xFF - a) * palette8W[i].b) >> 8;
        palette16WF[i]  = RGB888_TO_RGB565(red, green, blue);
        palette32WF[i]  = PACK_RGB888(red, green, blue);
        palette8WF[i].r = red;
        palette8WF[i].g = green;
        palette8WF[i].b = blue;
    }
}

void SetWaterColour(byte r, byte g, byte b, ushort a)
{
    fadeMode = 1;
    if (a > 255)
        a = 255;
    for (int i = 0; i < 0x100; ++i) {
        byte red     = (ushort)(r * a + (0xFF - a) * palette8[i].r) >> 8;
        byte green   = (ushort)(g * a + (0xFF - a) * palette8[i].g) >> 8;
        byte blue    = (ushort)(b * a + (0xFF - a) * palette8[i].b) >> 8;
        palette16W[i]  = RGB888_TO_RGB565(red, green, blue);
        palette32W[i]  = PACK_RGB888(red, green, blue);
        palette8W[i].r = red;
        palette8W[i].g = green;
        palette8W[i].b = blue;
    }
}

void WaterFlash()
{
    fadeMode = 5;
    for (int i = 0; i < 0x100; ++i) {
        palette16WF[i]  = RGB888_TO_RGB565(0xFF, 0xFF, 0xFF);
        palette32WF[i]  = PACK_RGB888(0xFF, 0xFF, 0xFF);
        palette8WF[i].r = 0xFF;
        palette8WF[i].g = 0xFF;
        palette8WF[i].b = 0xFF;
    }
}