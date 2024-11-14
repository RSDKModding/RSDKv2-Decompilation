#include "RetroEngine.hpp"

struct GifDecoder {
    int depth;
    int clearCode;
    int eofCode;
    int runningCode;
    int runningBits;
    int prevCode;
    int currentCode;
    int maxCodePlusOne;
    int stackPtr;
    int shiftState;
    int fileState;
    int position;
    int bufferSize;
    uint shiftData;
    uint pixelCount;
    byte buffer[256];
    byte stack[4096];
    byte suffix[4096];
    uint prefix[4096];
};

const int LOADING_IMAGE = 0;
const int LOAD_COMPLETE = 1;
const int LZ_MAX_CODE   = 4095;
const int LZ_BITS       = 12;
const int FIRST_CODE    = 4097;
const int NO_SUCH_CODE  = 4098;

struct GifDecoder GDecoder;
int codeMasks[] = { 0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095 };

int ReadGifCode(void);
byte ReadGifByte(void);
byte TraceGifPrefix(uint *prefix, int code, int clearCode);

void InitGifDecoder() {
    byte val = 0;
    FileRead(&val, 1);
    GDecoder.fileState      = LOADING_IMAGE;
    GDecoder.position       = 0;
    GDecoder.bufferSize     = 0;
    GDecoder.buffer[0]      = 0;
    GDecoder.depth          = val;
    GDecoder.clearCode      = 1 << val;
    GDecoder.eofCode        = GDecoder.clearCode + 1;
    GDecoder.runningCode    = GDecoder.eofCode + 1;
    GDecoder.runningBits    = val + 1;
    GDecoder.maxCodePlusOne = 1 << GDecoder.runningBits;
    GDecoder.stackPtr       = 0;
    GDecoder.prevCode       = NO_SUCH_CODE;
    GDecoder.shiftState     = 0;
    GDecoder.shiftData      = 0u;
    for (int i = 0; i <= LZ_MAX_CODE; ++i) GDecoder.prefix[i] = (byte)NO_SUCH_CODE;
}
void ReadGifLine(byte *line, int length, int offset) {
    int i         = 0;
    int stackPtr  = GDecoder.stackPtr;
    int eofCode   = GDecoder.eofCode;
    int clearCode = GDecoder.clearCode;
    int prevCode  = GDecoder.prevCode;
    if (stackPtr != 0) {
        while (stackPtr != 0) {
            if (i >= length) {
                break;
            }
            line[offset++] = GDecoder.stack[--stackPtr];
            i++;
        }
    }
    while (i < length) {
        int gifCode = ReadGifCode();
        if (gifCode == eofCode) {
            if (i != length - 1 | GDecoder.pixelCount != 0u) {
                return;
            }
            i++;
        } else {
            if (gifCode == clearCode) {
                for (int j = 0; j <= LZ_MAX_CODE; j++) {
                    GDecoder.prefix[j] = NO_SUCH_CODE;
                }
                GDecoder.runningCode    = GDecoder.eofCode + 1;
                GDecoder.runningBits    = GDecoder.depth + 1;
                GDecoder.maxCodePlusOne = 1 << GDecoder.runningBits;
                prevCode                  = (GDecoder.prevCode = NO_SUCH_CODE);
            } else {
                if (gifCode < clearCode) {
                    line[offset] = (byte)gifCode;
                    offset++;
                    i++;
                } else {
                    if (gifCode < 0 | gifCode > LZ_MAX_CODE) {
                        return;
                    }
                    int code;
                    if (GDecoder.prefix[gifCode] == NO_SUCH_CODE) {
                        if (gifCode != GDecoder.runningCode - 2) {
                            return;
                        }
                        code = prevCode;
                        GDecoder.suffix[GDecoder.runningCode - 2] =
                            (GDecoder.stack[stackPtr++] = TraceGifPrefix(GDecoder.prefix, prevCode, clearCode));
                    } else {
                        code = gifCode;
                    }
                    int c = 0;
                    while (c++ <= LZ_MAX_CODE && code > clearCode && code <= LZ_MAX_CODE) {
                        GDecoder.stack[stackPtr++] = GDecoder.suffix[code];
                        code                         = GDecoder.prefix[code];
                    }
                    if (c >= LZ_MAX_CODE | code > LZ_MAX_CODE) {
                        return;
                    }
                    GDecoder.stack[stackPtr++] = (byte)code;
                    while (stackPtr != 0 && i++ < length) {
                        line[offset++] = GDecoder.stack[--stackPtr];
                    }
                }
                if (prevCode != NO_SUCH_CODE) {
                    if (GDecoder.runningCode < 2 | GDecoder.runningCode > FIRST_CODE) {
                        return;
                    }
                    GDecoder.prefix[GDecoder.runningCode - 2] = prevCode;
                    if (gifCode == GDecoder.runningCode - 2) {
                        GDecoder.suffix[GDecoder.runningCode - 2] = TraceGifPrefix(GDecoder.prefix, prevCode, clearCode);
                    } else {
                        GDecoder.suffix[GDecoder.runningCode - 2] = TraceGifPrefix(GDecoder.prefix, gifCode, clearCode);
                    }
                }
                prevCode = gifCode;
            }
        }
    }
    GDecoder.prevCode = prevCode;
    GDecoder.stackPtr = stackPtr;
}

int ReadGifCode() {
    while (GDecoder.shiftState < GDecoder.runningBits) {
        byte b = ReadGifByte();
        GDecoder.shiftData |= (uint)((uint)b << GDecoder.shiftState);
        GDecoder.shiftState += 8;
    }
    int result = (int)((unsigned long)GDecoder.shiftData & (unsigned long)(codeMasks[GDecoder.runningBits]));
    GDecoder.shiftData >>= GDecoder.runningBits;
    GDecoder.shiftState -= GDecoder.runningBits;
    if (++GDecoder.runningCode > GDecoder.maxCodePlusOne && GDecoder.runningBits < LZ_BITS) {
        GDecoder.maxCodePlusOne <<= 1;
        GDecoder.runningBits++;
    }
    return result;
}

byte ReadGifByte() {
    char c = '\0';
    if (GDecoder.fileState == LOAD_COMPLETE)
        return c;

    byte b;
    if (GDecoder.position == GDecoder.bufferSize) {
        FileRead(&b, 1);
        GDecoder.bufferSize = (int)b;
        if (GDecoder.bufferSize == 0) {
            GDecoder.fileState = LOAD_COMPLETE;
            return c;
        }
        FileRead(GDecoder.buffer, GDecoder.bufferSize);
        b                   = GDecoder.buffer[0];
        GDecoder.position = 1;
    } else {
        b = GDecoder.buffer[GDecoder.position++];
    }
    return b;
}

byte TraceGifPrefix(uint *prefix, int code, int clearCode) {
    int i = 0;
    while (code > clearCode && i++ <= LZ_MAX_CODE) code = prefix[code];

    return code;
}
void ReadGifPictureData(int width, int height, bool interlaced, byte *gfxData, int offset) {
    int array[]  = { 0, 4, 2, 1 };
    int array2[] = { 8, 8, 4, 2 };
    InitGifDecoder();
    if (interlaced) {
        for (int i = 0; i < 4; ++i) {
            for (int j = array[i]; j < height; j += array2[i]) {
                ReadGifLine(gfxData, width, j * width + offset);
            }
        }
        return;
    }
    for (int h = 0; h < height; ++h) ReadGifLine(gfxData, width, h * width + offset);
}

int AddGraphicsFile(const char *filePath) {
    char sheetPath[!RETRO_USE_ORIGINAL_CODE ? 0x100 : 0x40];

    StrCopy(sheetPath, "Data/Sprites/");
    StrAdd(sheetPath, filePath);
    int sheetID = 0;
    while (StrLength(GfxSurface[sheetID].fileName) > 0) {
        if (StrComp(GfxSurface[sheetID].fileName, sheetPath))
            return sheetID;
        // NOTE
        // This uses SPRITESHEETS_MAX (16), while RemoveGraphicsFile, uses SURFACE_MAX (24)...
        if (++sheetID >= SPRITESHEETS_MAX)
            return 0;
    }
    byte fileExtension = (byte)sheetPath[(StrLength(sheetPath) - 1) & 0xFF];
    switch (fileExtension) {
        case 'f': LoadGIFFile(sheetPath, sheetID); break;
        case 'p': LoadBMPFile(sheetPath, sheetID); break;
        case 'v': LoadRSVFile(sheetPath, sheetID); break;
        case 'x': LoadGFXFile(sheetPath, sheetID); break;
    }

    return sheetID;
}
void RemoveGraphicsFile(const char *filePath, int sheetID) {
    if (sheetID < 0) {
        for (int i = 0; i < SURFACE_MAX; ++i) {
            if (StrLength(GfxSurface[i].fileName) > 0 && StrComp(GfxSurface[i].fileName, filePath))
                sheetID = i;
        }
    }

    if (sheetID >= 0 && StrLength(GfxSurface[sheetID].fileName)) {
        StrCopy(GfxSurface[sheetID].fileName, "");
        int dataPosStart = GfxSurface[sheetID].dataPosition;
        int dataPosEnd   = GfxSurface[sheetID].dataPosition + GfxSurface[sheetID].height * GfxSurface[sheetID].width;
        for (int i = 0x200000 - dataPosEnd; i > 0; --i) GraphicData[dataPosStart++] = GraphicData[dataPosEnd++];
        GfxDataPosition -= GfxSurface[sheetID].height * GfxSurface[sheetID].width;
        for (int i = 0; i < SURFACE_MAX; ++i) {
            if (GfxSurface[i].dataPosition > GfxSurface[sheetID].dataPosition)
                GfxSurface[i].dataPosition -= GfxSurface[sheetID].height * GfxSurface[sheetID].width;
        }
    }
}

int LoadBMPFile(const char *filePath, byte sheetID) {
    FileInfo info;
    if (LoadFile(filePath, &info)) {
        GFXSurface *surface = &GfxSurface[sheetID];
        StrCopy(surface->fileName, filePath);

        byte fileBuffer = 0;

        SetFilePosition(18);
        FileRead(&fileBuffer, 1);
        surface->width = fileBuffer;
        FileRead(&fileBuffer, 1);
        surface->width += fileBuffer << 8;
        FileRead(&fileBuffer, 1);
        surface->width += fileBuffer << 16;
        FileRead(&fileBuffer, 1);
        surface->width += fileBuffer << 24;

        FileRead(&fileBuffer, 1);
        surface->height = fileBuffer;
        FileRead(&fileBuffer, 1);
        surface->height += fileBuffer << 8;
        FileRead(&fileBuffer, 1);
        surface->height += fileBuffer << 16;
        FileRead(&fileBuffer, 1);
        surface->height += fileBuffer << 24;

        SetFilePosition(info.fileSize - surface->height * surface->width);
        surface->dataPosition = GfxDataPosition;
        byte *gfxData         = &GraphicData[surface->dataPosition + surface->width * (surface->height - 1)];
        for (int y = 0; y < surface->height; ++y) {
            for (int x = 0; x < surface->width; ++x) {
                FileRead(&fileBuffer, 1);
                *gfxData++ = fileBuffer;
            }
            gfxData -= 2 * surface->width;
        }
        GfxDataPosition += surface->height * surface->width;

        if (GfxDataPosition >= GFXDATA_MAX) {
            GfxDataPosition = 0;
            PrintLog("WARNING: Exceeded max gfx size!");
        }

        CloseFile();
        return true;
    }
    return false;
}
int LoadGIFFile(const char *filePath, byte sheetID) {
    FileInfo info;
    if (LoadFile(filePath, &info)) {
        GFXSurface *surface = &GfxSurface[sheetID];
        StrCopy(surface->fileName, filePath);

        byte fileBuffer = 0;
        byte fileBuffer2[2];

        SetFilePosition(6); // GIF89a
        FileRead(&fileBuffer, 1);
        surface->width = fileBuffer;
        FileRead(&fileBuffer, 1);
        surface->width += (fileBuffer << 8);
        FileRead(&fileBuffer, 1);
        surface->height = fileBuffer;
        FileRead(&fileBuffer, 1);
        surface->height += (fileBuffer << 8);

        FileRead(&fileBuffer, 1); // Palette Size
        int has_pallete  = (fileBuffer & 0x80) >> 7;
        int colors       = ((fileBuffer & 0x70) >> 4) + 1;
        int palette_size = (fileBuffer & 0x7) + 1;
        if (palette_size > 0)
            palette_size = 1 << palette_size;

        FileRead(&fileBuffer, 1); // BG Colour index (thrown away)
        FileRead(&fileBuffer, 1); // idk actually (still thrown away)

        int c = 0;
        byte clr[3];
        do {
            ++c;
            FileRead(clr, 3);
        } while (c != palette_size);

        FileRead(&fileBuffer, 1);
        while (fileBuffer != ',') FileRead(&fileBuffer, 1); // gif image start identifier

        FileRead(fileBuffer2, 2);
        FileRead(fileBuffer2, 2);
        FileRead(fileBuffer2, 2);
        FileRead(fileBuffer2, 2);
        FileRead(&fileBuffer, 1);
        bool interlaced = (fileBuffer & 0x40) >> 6;
        if (fileBuffer >> 7 == 1) {
            int c = 128;
            do {
                ++c;
                FileRead(clr, 3);
            } while (c != 256);
        }

        surface->dataPosition = GfxDataPosition;

        GfxDataPosition += surface->width * surface->height;
        if (GfxDataPosition < GFXDATA_MAX) {
            ReadGifPictureData(surface->width, surface->height, interlaced, GraphicData, surface->dataPosition);
        } else {
            GfxDataPosition = 0;
            PrintLog("WARNING: Exceeded max gfx surface size!");
        }

        CloseFile();
        return true;
    }
    return false;
}
int LoadGFXFile(const char *filePath, byte sheetID) {
    FileInfo info;
    if (LoadFile(filePath, &info)) {
        GFXSurface *surface = &GfxSurface[sheetID];
        StrCopy(surface->fileName, filePath);

        byte fileBuffer = 0;
        FileRead(&fileBuffer, 1);
        surface->width = fileBuffer << 8;
        FileRead(&fileBuffer, 1);
        surface->width += fileBuffer;
        FileRead(&fileBuffer, 1);
        surface->height = fileBuffer << 8;
        FileRead(&fileBuffer, 1);
        surface->height += fileBuffer;

        byte clr[3];
        for (int i = 0; i < 0xFF; ++i) FileRead(&clr, 3); // Palette

        surface->dataPosition = GfxDataPosition;
        byte *gfxData         = &GraphicData[surface->dataPosition];
        byte buf[3];
        while (true) {
            FileRead(&buf[0], 1);
            if (buf[0] == 0xFF) {
                FileRead(&buf[1], 1);
                if (buf[1] == 0xFF) {
                    break;
                } else {
                    FileRead(&buf[2], 1);
                    for (int i = 0; i < buf[2]; ++i) *gfxData++ = buf[1];
                }
            } else {
                *gfxData++ = buf[0];
            }
        }

        GfxDataPosition += surface->height * surface->width;

        if (GfxDataPosition >= GFXDATA_MAX) {
            GfxDataPosition = 0;
            PrintLog("WARNING: Exceeded max gfx size!");
        }

        CloseFile();
        return true;
    }
    return false;
}
int LoadRSVFile(const char *filePath, byte sheetID) {
    FileInfo info;
    if (LoadFile(filePath, &info)) {
        GFXSurface *surface = &GfxSurface[sheetID];
        StrCopy(surface->fileName, filePath);

        VideoSurface      = sheetID;
        CurrentVideoFrame = 0;

        byte fileBuffer = 0;

        FileRead(&fileBuffer, 1);
        VideoFrameCount = fileBuffer;
        FileRead(&fileBuffer, 1);
        VideoFrameCount += fileBuffer << 8;

        FileRead(&fileBuffer, 1);
        VideoWidth = fileBuffer;
        FileRead(&fileBuffer, 1);
        VideoWidth += fileBuffer << 8;

        FileRead(&fileBuffer, 1);
        VideoHeight = fileBuffer;
        FileRead(&fileBuffer, 1);
        VideoHeight += fileBuffer << 8;

        VideoFilePos          = (int)GetFilePosition();
        VideoPlaying          = true;
        surface->width        = VideoWidth;
        surface->height       = VideoHeight;
        surface->dataPosition = GfxDataPosition;
        GfxDataPosition += surface->width * surface->height;

        if (GfxDataPosition >= GFXDATA_MAX) {
            GfxDataPosition = 0;
            PrintLog("WARNING: Exceeded max gfx size!");
        }

        return true;
    }
    return false;
}