#ifndef PALETTE_H
#define PALETTE_H

#define PALETTE_SIZE (0x100)

struct Colour {
    byte r;
    byte g;
    byte b;
    byte a;
};

// Palettes (as RGB888 Colours)
extern uint TilePalette32[PALETTE_SIZE];
extern uint TilePaletteW32[PALETTE_SIZE];
extern uint TilePalette32F[PALETTE_SIZE];
extern uint TilePaletteW32F[PALETTE_SIZE];

// Palettes (as RGB565 Colours)
extern ushort TilePalette16[PALETTE_SIZE];
extern ushort TilePaletteW16[PALETTE_SIZE];
extern ushort TilePalette16F[PALETTE_SIZE];
extern ushort TilePaletteW16F[PALETTE_SIZE];

// Water Flash Palettes (as RGB888 Colours)
extern Colour TilePalette[PALETTE_SIZE];
extern Colour TilePaletteW[PALETTE_SIZE];
extern Colour TilePaletteF[PALETTE_SIZE];
extern Colour TilePaletteWF[PALETTE_SIZE];

extern int PaletteMode;

#define RGB888_TO_RGB565(r, g, b) ((b) >> 3) | (((g) >> 2) << 5) | (((r) >> 3) << 11)

#define PACK_RGB888(r, g, b) ((0xFF << 24) | ((r) << 16) | ((g) << 8) | ((b) << 0))

void LoadPalette(const char *filePath, int startIndex, int endIndex);

inline void SetPaletteEntry(byte index, byte r, byte g, byte b) {
    TilePalette32[index] = PACK_RGB888(r, g, b);
    TilePalette16[index] = RGB888_TO_RGB565(r, g, b);
    TilePalette[index].r = r;
    TilePalette[index].g = g;
    TilePalette[index].b = b;
}

inline void RotatePalette(byte startIndex, byte endIndex, bool right) {
    if (right) {
        Colour startClr8  = TilePalette[endIndex];
        ushort startClr16 = TilePalette16[endIndex];
        uint startClr32   = TilePalette32[endIndex];
        for (int i = endIndex; i > startIndex; --i) {
            TilePalette[i]   = TilePalette[i - 1];
            TilePalette16[i] = TilePalette16[i - 1];
            TilePalette32[i] = TilePalette32[i - 1];
        }
        TilePalette[startIndex]   = startClr8;
        TilePalette16[startIndex] = startClr16;
        TilePalette32[startIndex] = startClr32;
    } else {
        Colour startClr8  = TilePalette[startIndex];
        ushort startClr16 = TilePalette16[startIndex];
        uint startClr32   = TilePalette32[startIndex];
        for (int i = startIndex; i < endIndex; ++i) {
            TilePalette[i]   = TilePalette[i + 1];
            TilePalette16[i] = TilePalette16[i + 1];
            TilePalette32[i] = TilePalette32[i + 1];
        }
        TilePalette[endIndex]   = startClr8;
        TilePalette16[endIndex] = startClr16;
        TilePalette32[endIndex] = startClr32;
    }
}

void SetFade(byte r, byte g, byte b, ushort a, int start, int end);

void SetWaterColour(byte r, byte g, byte b, ushort a);
void WaterFlash();

#endif // !PALETTE_H
