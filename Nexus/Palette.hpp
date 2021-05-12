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
extern uint palette32[PALETTE_SIZE];
extern uint palette32W[PALETTE_SIZE];
extern uint palette32F[PALETTE_SIZE];
extern uint palette32WF[PALETTE_SIZE];

// Palettes (as RGB565 Colours)
extern ushort palette16[PALETTE_SIZE];
extern ushort palette16W[PALETTE_SIZE];
extern ushort palette16F[PALETTE_SIZE];
extern ushort palette16WF[PALETTE_SIZE];

// Palettes (as RGB888 Colours)
extern Colour palette8[PALETTE_SIZE];
extern Colour palette8W[PALETTE_SIZE];
extern Colour palette8F[PALETTE_SIZE];
extern Colour palette8WF[PALETTE_SIZE];

extern int fadeMode;

#define RGB888_TO_RGB565(r, g, b) ((b) >> 3) | (((g) >> 2) << 5) | (((r) >> 3) << 11)

#define PACK_RGB888(r, g, b) ((0xFF << 24) | ((r) << 16) | ((g) << 8) | ((b) << 0))

void LoadPalette(const char *filePath, int startIndex, int endIndex);

inline void SetPaletteEntry(byte index, byte r, byte g, byte b)
{
    palette32[index]  = PACK_RGB888(r, g, b);
    palette16[index]  = RGB888_TO_RGB565(r, g, b);
    palette8[index].r = r;
    palette8[index].g = g;
    palette8[index].b = b;
}

inline void RotatePalette(byte startIndex, byte endIndex, bool right)
{
    if (right) {
        Colour startClr8  = palette8[endIndex];
        ushort startClr16 = palette16[endIndex];
        uint startClr32   = palette32[endIndex];
        for (int i = endIndex; i > startIndex; --i) {
            palette8[i] = palette8[i - 1];
            palette16[i] = palette16[i - 1];
            palette32[i] = palette32[i - 1];
        }
        palette8[startIndex]  = startClr8;
        palette16[startIndex] = startClr16;
        palette32[startIndex] = startClr32;
    }
    else {
        Colour startClr8    = palette8[startIndex];
        ushort startClr16   = palette16[startIndex];
        uint startClr32   = palette32[startIndex];
        for (int i = startIndex; i < endIndex; ++i) {
            palette8[i] = palette8[i + 1];
            palette16[i] = palette16[i + 1];
            palette32[i] = palette32[i + 1];
        }
        palette8[endIndex] = startClr8;
        palette16[endIndex] = startClr16;
        palette32[endIndex] = startClr32;
    }
}

void SetFade(byte r, byte g, byte b, ushort a, int start, int end);

void SetWaterColour(byte r, byte g, byte b, ushort a);
void WaterFlash();

#endif // !PALETTE_H
