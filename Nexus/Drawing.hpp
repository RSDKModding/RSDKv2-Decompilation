#ifndef DRAWING_H
#define DRAWING_H

#define SPRITESHEETS_MAX (16)
#define SURFACE_MAX      (24)
#define GFXDATA_MAX      (0x400000)

#define BLENDTABLE_YSIZE (0x100)
#define BLENDTABLE_XSIZE (0x20)
#define BLENDTABLE_SIZE  (BLENDTABLE_XSIZE * BLENDTABLE_YSIZE)
#define TINTTABLE_SIZE   (0x1000)

#define DRAWLAYER_COUNT (0x7)

enum FlipFlags { FLIP_NONE, FLIP_X, FLIP_Y, FLIP_XY };
enum InkFlags { INK_NONE, INK_BLEND};
enum DrawFXFlags { FX_SCALE, FX_ROTATE, FX_INK };

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


extern DrawListEntry drawListEntries[DRAWLAYER_COUNT];

extern int gfxDataPosition;
extern GFXSurface gfxSurface[SURFACE_MAX];
extern byte graphicData[GFXDATA_MAX];

int InitRenderDevice();
void FlipScreen();
void ReleaseRenderDevice();

void GenerateBlendLookupTable();

inline void ClearGraphicsData()
{
    for (int i = 0; i < SURFACE_MAX; ++i) StrCopy(gfxSurface[i].fileName, "");
    gfxDataPosition = 0;
}
void ClearScreen(byte index);

void SetScreenSize(int width, int height);

// Layer Drawing
void DrawObjectList(int layer);
void DrawStageGFX();

// TileLayer Drawing
void DrawHLineScrollLayer(int layerID);
void DrawVLineScrollLayer(int layerID);
void Draw3DCloudLayer(int layerID);
void Draw3DSkyLayer(int layerID);

// Shape Drawing
void DrawTintRectangle(int XPos, int YPos, int width, int height);
void DrawScaledTintMask(int direction, int XPos, int YPos, int pivotX, int pivotY, int scaleX, int scaleY, int width, int height, int sprX, int sprY,
                        int sheetID);

// Sprite Drawing
void DrawSprite(int XPos, int YPos, int width, int height, int sprX, int sprY, int sheetID);
void DrawSpriteScaled(int direction, int XPos, int YPos, int pivotX, int pivotY, int scaleX, int scaleY, int width, int height, int sprX, int sprY,
                      int sheetID);
void DrawSpriteRotated(int direction, int XPos, int YPos, int pivotX, int pivotY, int sprX, int sprY, int width, int height, int rotation,
                       int sheetID);
void DrawBlendedSprite(int XPos, int YPos, int width, int height, int sprX, int sprY, int sheetID);

#endif // !DRAWING_H
