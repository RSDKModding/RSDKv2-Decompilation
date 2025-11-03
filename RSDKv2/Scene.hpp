#ifndef SCENE_H
#define SCENE_H

#define LAYER_COUNT    (9)
#define DEFORM_STORE   (0x100)
#define DEFORM_SIZE    (320)
#define DEFORM_COUNT   (DEFORM_STORE + DEFORM_SIZE)
#define PARALLAX_COUNT (0x100)

#define TILE_COUNT    (0x400)
#define TILE_SIZE     (0x10)
#define CHUNK_SIZE     (0x80)
#define TILE_DATASIZE (TILE_SIZE * TILE_SIZE)
#define TILESET_SIZE  (TILE_COUNT * TILE_DATASIZE)

#define TILELAYER_CHUNK_W  (0x100)
#define TILELAYER_CHUNK_H  (0x100)
#define TILELAYER_CHUNK_MAX  (TILELAYER_CHUNK_W * TILELAYER_CHUNK_H)
#define TILELAYER_SCROLL_MAX (TILELAYER_CHUNK_H * CHUNK_SIZE)

#define CHUNKTILE_COUNT (0x200 * (8 * 8))

#define CPATH_COUNT (2)

enum StageListNames {
    STAGELIST_PRESENTATION = 0,
    STAGELIST_REGULAR      = 1,
    STAGELIST_BONUS        = 2,
    STAGELIST_SPECIAL      = 3,
    STAGELIST_MAX, //StageList size
};

enum TileLayerTypes {
    LAYER_NOSCROLL = 0,
    LAYER_HSCROLL  = 1,
    LAYER_VSCROLL  = 2,
    LAYER_3DCLOUD  = 3,
    LAYER_3DSKY    = 4,
};

enum StageModes {
    STAGEMODE_LOAD   = 0,
    STAGEMODE_NORMAL = 1,
    STAGEMODE_PAUSED = 2,
};

enum TileInfo {
    TILEINFO_INDEX       = 0,
    TILEINFO_DIRECTION   = 1,
    TILEINFO_VISUALPLANE = 2,
    TILEINFO_SOLIDITYA   = 3,
    TILEINFO_SOLIDITYB   = 4,
    TILEINFO_FLAGSA      = 5,
    TILEINFO_ANGLEA      = 6,
    TILEINFO_FLAGSB      = 7,
    TILEINFO_ANGLEB      = 8,
};

enum DeformationModes {
    DEFORM_FG       = 0,
    DEFORM_FG_WATER = 1,
    DEFORM_BG       = 2,
    DEFORM_BG_WATER = 3,
};

enum CameraStyles {
    CAMERASTYLE_FOLLOW,
    CAMERASTYLE_EXTENDED,
};

struct SceneInfo {
    char name[0x40];
    char folder[0x40];
    char id[0x40];
    bool highlighted;
};

struct CollisionMasks {
    sbyte floorMasks[TILE_COUNT * TILE_SIZE];
    sbyte lWallMasks[TILE_COUNT * TILE_SIZE];
    sbyte rWallMasks[TILE_COUNT * TILE_SIZE];
    sbyte roofMasks[TILE_COUNT * TILE_SIZE];
    int angles[TILE_COUNT];
    byte flags[TILE_COUNT];
};

struct TileLayer {
    ushort tiles[TILELAYER_CHUNK_MAX];
    byte lineScroll[TILELAYER_SCROLL_MAX];
    int parallaxFactor;
    int scrollSpeed;
    int scrollPos;
    int angle;
    int XPos;
    int YPos;
    int ZPos;
    byte type;
    byte xsize;
    byte ysize;
};

struct LineScroll {
    int parallaxFactor[PARALLAX_COUNT];
    int scrollSpeed[PARALLAX_COUNT];
    int scrollPos[PARALLAX_COUNT];
    int linePos[PARALLAX_COUNT];
    byte deform[PARALLAX_COUNT];
    byte entryCount;
};

struct Tiles128x128 {
    int gfxDataPos[CHUNKTILE_COUNT];
    ushort tileIndex[CHUNKTILE_COUNT];
    byte direction[CHUNKTILE_COUNT];
    byte visualPlane[CHUNKTILE_COUNT];
    byte collisionFlags[CPATH_COUNT][CHUNKTILE_COUNT];
};

extern int stageListCount[STAGELIST_MAX];
extern char stageListNames[STAGELIST_MAX][0x20];
extern SceneInfo stageList[STAGELIST_MAX][0x100];

extern int StageMode;

extern int CameraStyle;
extern int CameraEnabled;
extern int cameraAdjustY;
extern int XScrollOffset;
extern int YScrollOffset;
extern int YScrollA;
extern int YScrollB;
extern int XScrollA;
extern int XScrollB;
extern int YScrollMove;
extern int EarthquakeX;
extern int EarthquakeY;
extern int XScrollMove;

extern int XBoundary1;
extern int NewXBoundary1;
extern int YBoundary1;
extern int NewYBoundary1;
extern int XBoundary2;
extern int YBoundary2;
extern int WaterLevel;
extern int waterDrawPos;
extern int NewXBoundary2;
extern int NewYBoundary2;

extern int SCREEN_SCROLL_LEFT;
extern int SCREEN_SCROLL_RIGHT;
#define SCREEN_SCROLL_UP   ((SCREEN_YSIZE / 2) - 16)
#define SCREEN_SCROLL_DOWN ((SCREEN_YSIZE / 2) + 16)

extern int LastXSize;
extern int LastYSize;

extern bool PauseEnabled;
extern bool TimeEnabled;
extern bool debugMode;
extern int FrameCounter;
extern int MilliSeconds;
extern int Seconds;
extern int Minutes;

// Category and Scene IDs
extern int ActiveStageList;
extern int StageListPosition;
extern char CurrentStageFolder[0x100];
extern int ActNumber;

extern char titleCardText[0x100];
extern byte titleCardWord2;

extern byte activeTileLayers[4];
extern byte tLayerMidPoint;
extern TileLayer StageLayouts[LAYER_COUNT];

extern int BGDeformationData1[DEFORM_COUNT];
extern int BGDeformationData2[DEFORM_COUNT];
extern int BGDeformationData3[DEFORM_COUNT];
extern int BGDeformationData4[DEFORM_COUNT];

extern int DeformationPos1;
extern int DeformationPos2;
extern int DeformationPos3;
extern int DeformationPos4;

extern LineScroll HParallax;
extern LineScroll VParallax;

extern Tiles128x128 StageTiles;
extern CollisionMasks TileCollisions[2];

extern byte TileGfx[TILESET_SIZE];

void ProcessStage();

void ResetBackgroundSettings();
inline void ResetCurrentStageFolder() { strcpy(CurrentStageFolder, ""); }
inline bool CheckCurrentStageFolder(int stage)
{
    if (strcmp(CurrentStageFolder, stageList[ActiveStageList][stage].folder) == 0) {
        return true;
    }
    else {
        strcpy(CurrentStageFolder, stageList[ActiveStageList][stage].folder);
        return false;
    }
}

void LoadStageFiles();
int LoadActFile(const char *ext, int stageID, FileInfo *info);
int LoadStageFile(const char *filePath, int stageID, FileInfo *info);

void LoadActLayout();
void LoadStageBackground();
void Load128x128Mappings();
void LoadStageCollisions();
void LoadStageGIFFile(int stageID);
void LoadStageGFXFile(int stageID);
void SetPlayerScreenPosition(Player *player);
void SetPlayerScreenPositionCDStyle(Player *player);
void SetPlayerLockedScreenPosition(Player *player);

#endif // !SCENE_H
