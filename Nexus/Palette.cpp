#include "RetroEngine.hpp"

// Palettes (as RGB888 Colours)
PaletteEntry fullPalette32[PALETTE_SIZE];

// Palettes (as RGB565 Colours)
ushort fullPalette[PALETTE_SIZE];

int fadeMode = 0;
byte fadeA   = 0;
byte fadeR   = 0;
byte fadeG   = 0;
byte fadeB   = 0;

int paletteMode = 0;

void LoadPalette(const char *filePath, int startPaletteIndex, int startIndex, int endIndex)
{
    FileInfo info;
    char fullPath[0x80];

    StrCopy(fullPath, "Data/Palettes/");
    StrAdd(fullPath, filePath);

    if (LoadFile(fullPath, &info)) {
        SetFilePosition(3 * startIndex);

        byte colour[3];
        for (int i = startIndex; i < endIndex; ++i) {
            FileRead(&colour, 3);
            SetPaletteEntry(startPaletteIndex++, colour[0], colour[1], colour[2]);
        }
        CloseFile();
    }
}