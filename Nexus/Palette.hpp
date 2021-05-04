#ifndef PALETTE_H
#define PALETTE_H

#define PALETTE_SIZE  (0x100)

struct Colour {
    byte r;
    byte g;
    byte b;
    byte a;
};

struct PaletteEntry {
    byte r;
    byte g;
    byte b;
};

// Palettes (as RGB565 Colours)
extern PaletteEntry fullPalette32[PALETTE_SIZE];
extern ushort fullPalette[PALETTE_SIZE];

extern int fadeMode;
extern byte fadeA;
extern byte fadeR;
extern byte fadeG;
extern byte fadeB;

extern int paletteMode;

#define RGB888_TO_RGB565(r, g, b)  ((b) >> 3) | (((g) >> 2) << 5) | (((r) >> 3) << 11)

#define PACK_RGB888(r, g, b) RGB888_TO_RGB565(r, g, b)

void LoadPalette(const char *filePath, int startPaletteIndex, int startIndex, int endIndex);

inline void SetPaletteEntry(byte index, byte r, byte g, byte b)
{
    fullPalette[index]     = PACK_RGB888(r, g, b);
    fullPalette32[index].r = r;
    fullPalette32[index].g = g;
    fullPalette32[index].b = b;
}

inline void RotatePalette(byte startIndex, byte endIndex, bool right)
{
    if (right) {
        ushort startClr         = fullPalette[endIndex];
        PaletteEntry startClr32 = fullPalette32[startIndex];
        for (int i = endIndex; i > startIndex; --i) {
            fullPalette[i]   = fullPalette[i - 1];
            fullPalette32[i] = fullPalette32[i - 1];
        }
        fullPalette[startIndex] = startClr;
        fullPalette32[endIndex] = startClr32;
    }
    else {
        ushort startClr         = fullPalette[startIndex];
        PaletteEntry startClr32 = fullPalette32[startIndex];
        for (int i = startIndex; i < endIndex; ++i) {
            fullPalette[i]   = fullPalette[i + 1];
            fullPalette32[i] = fullPalette32[i + 1];
        }
        fullPalette[endIndex]   = startClr;
        fullPalette32[endIndex] = startClr32;
    }
}

inline void SetFade(byte R, byte G, byte B, ushort A)
{
    fadeMode = 1;
    fadeR    = R;
    fadeG    = G;
    fadeB    = B;
    fadeA    = A > 0xFF ? 0xFF : A;
}

#endif // !PALETTE_H
