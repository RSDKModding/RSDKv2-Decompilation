#include "RetroEngine.hpp"

int currentVideoFrame = 0;
int videoFrameCount   = 0;
int videoWidth        = 0;
int videoHeight       = 0;
int videoData         = 0;
int videoFilePos      = 0;
bool videoPlaying     = 0;

void UpdateVideoFrame()
{
    if (videoPlaying) {
        if (videoFrameCount > currentVideoFrame) {
            GFXSurface *surface = &gfxSurface[videoData];
            byte fileBuffer      = 0;
            FileRead(&fileBuffer, 1);
            videoFilePos += fileBuffer;
            FileRead(&fileBuffer, 1);
            videoFilePos += fileBuffer << 8;
            FileRead(&fileBuffer, 1);
            videoFilePos += fileBuffer << 16;
            FileRead(&fileBuffer, 1);
            videoFilePos += fileBuffer << 24;

            byte clr[3];
            for (int i = 0; i < 0x80; ++i) {
                FileRead(&clr, 3);
                palette32[i].r = clr[0];
                palette32[i].g = clr[1];
                palette32[i].b = clr[2];
                palette16[i]     = ((ushort)(clr[0] >> 3) << 11) | 32 * (clr[1] >> 2) | (clr[2] >> 3);
            }

            FileRead(&fileBuffer, 1);
            while (fileBuffer != ',') FileRead(&fileBuffer, 1); // gif image start identifier

            FileRead(&fileBuffer, 2); // IMAGE LEFT
            FileRead(&fileBuffer, 2); // IMAGE TOP
            FileRead(&fileBuffer, 2); // IMAGE WIDTH
            FileRead(&fileBuffer, 2); // IMAGE HEIGHT
            FileRead(&fileBuffer, 1); // PaletteType
            bool interlaced = (fileBuffer & 0x40) >> 6;
            if (fileBuffer >> 7 == 1) {
                int c = 0x80;
                do {
                    ++c;
                    FileRead(&fileBuffer, 3);
                } while (c != 0x100);
            }
            ReadGifPictureData(surface->width, surface->height, interlaced, graphicData, surface->dataPosition);

            SetFilePosition(videoFilePos);
            ++currentVideoFrame;
        }
        else {
            videoPlaying = 0;
            CloseFile();
        }
    }
}