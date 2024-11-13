#include "RetroEngine.hpp"

int CurrentVideoFrame = 0;
int VideoFrameCount   = 0;
int VideoWidth        = 0;
int VideoHeight       = 0;
int VideoSurface      = 0;
int VideoFilePos      = 0;
bool VideoPlaying     = false;

void UpdateVideoFrame() {
    if (VideoPlaying) {
        if (VideoFrameCount <= CurrentVideoFrame) {
            VideoPlaying = false;
            CloseFile();
        } else {
            GFXSurface *surface = &GfxSurface[VideoSurface];
            byte fileBuffer     = 0;
            ushort fileBuffer2  = 0;
            FileRead(&fileBuffer, 1);
            VideoFilePos += fileBuffer;
            FileRead(&fileBuffer, 1);
            VideoFilePos += fileBuffer << 8;
            FileRead(&fileBuffer, 1);
            VideoFilePos += fileBuffer << 16;
            FileRead(&fileBuffer, 1);
            VideoFilePos += fileBuffer << 24;

            byte clr[3];
            for (int i = 0; i < 0x80; ++i) {
                FileRead(&clr, 3);
                SetPaletteEntry(i, clr[0], clr[1], clr[2]);
            }
            SetPaletteEntry(0, 0x00, 0x00, 0x00);

            FileRead(&fileBuffer, 1);
            while (fileBuffer != ',') FileRead(&fileBuffer, 1); // gif image start identifier

            FileRead(&fileBuffer2, 2); // IMAGE LEFT
            FileRead(&fileBuffer2, 2); // IMAGE TOP
            FileRead(&fileBuffer2, 2); // IMAGE WIDTH
            FileRead(&fileBuffer2, 2); // IMAGE HEIGHT
            FileRead(&fileBuffer, 1);  // PaletteType
            bool interlaced = (fileBuffer & 0x40) >> 6;
            if (fileBuffer >> 7 == 1) {
                int c = 0x80;
                do {
                    ++c;
                    FileRead(&fileBuffer, 1);
                    FileRead(&fileBuffer, 1);
                    FileRead(&fileBuffer, 1);
                } while (c != 0x100);
            }
            ReadGifPictureData(surface->width, surface->height, interlaced, GraphicData, surface->dataPosition);

            SetFilePosition(VideoFilePos);
            ++CurrentVideoFrame;
        }
    }
}