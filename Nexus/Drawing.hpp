#ifndef DRAWING_H
#define DRAWING_H

#define SPRITESHEETS_MAX (16)
#define SURFACE_MAX      (24)
#define GFXDATA_MAX      (0x400000)

#define DRAWLAYER_COUNT (0x7)

enum FlipFlags { FLIP_NONE, FLIP_X, FLIP_Y, FLIP_XY };
enum InkFlags { INK_NONE, INK_BLEND, INK_TINT };
enum DrawFXFlags { FX_SCALE, FX_ROTATE, FX_INK, FX_TINT };

struct DrawListEntry {
    int entityRefs[ENTITY_COUNT];
    int listSize;
};

struct GFXSurface {
    char fileName[0x40];
    int height;
    int width;
    int dataPosition;
};

extern int SCREEN_XSIZE;
extern int SCREEN_CENTERX;

extern byte blendLookupTable[0x100 * 0x100];

extern byte tintTable0[0x100];
extern byte tintTable1[0x100];
extern byte tintTable2[0x100];
extern byte tintTable3[0x100];

extern DrawListEntry drawListEntries[DRAWLAYER_COUNT];

extern int gfxDataPosition;
extern GFXSurface gfxSurface[SURFACE_MAX];
extern byte graphicData[GFXDATA_MAX];

int InitRenderDevice();
void FlipScreen();
void ReleaseRenderDevice();

inline void ClearGraphicsData()
{
    for (int i = 0; i < SURFACE_MAX; ++i) StrCopy(gfxSurface[i].fileName, "");
    gfxDataPosition = 0;
}
void ClearScreen(byte index);

void SetScreenSize(int width, int lineSize);

void SetBlendTable(ushort alpha, byte type, byte a3, byte a4);
void SetTintTable(short alpha, short a2, byte type, byte a4, byte a5, byte tableID);

// Layer Drawing
void DrawObjectList(int layer);
void DrawStageGFX();

// TileLayer Drawing
void DrawHLineScrollLayer(int layerID);
void DrawVLineScrollLayer(int layerID);
void Draw3DCloudLayer(int layerID);

// Shape Drawing
void DrawTintRectangle(int XPos, int YPos, int width, int height, byte tintID);
void DrawScaledTintMask(int direction, int XPos, int YPos, int pivotX, int pivotY, int scaleX, int scaleY, int width, int height, int sprX, int sprY,
                        int tintID, int sheetID);

// Sprite Drawing
void DrawSprite(int XPos, int YPos, int width, int height, int sprX, int sprY, int sheetID);
void DrawSpriteScaled(int direction, int XPos, int YPos, int pivotX, int pivotY, int scaleX, int scaleY, int width, int height, int sprX, int sprY,
                      int sheetID);
void DrawSpriteRotated(int direction, int XPos, int YPos, int pivotX, int pivotY, int sprX, int sprY, int width, int height, int rotation,
                       int sheetID);
void DrawBlendedSprite(int XPos, int YPos, int width, int height, int sprX, int sprY, int sheetID);

//Text Menus
void DrawTextMenuEntry(void *menu, int rowID, int XPos, int YPos, int textHighlight);
void DrawStageTextEntry(void *menu, int rowID, int XPos, int YPos, int textHighlight);
void DrawTextMenu(void *menu, int XPos, int YPos);

#endif // !DRAWING_H
