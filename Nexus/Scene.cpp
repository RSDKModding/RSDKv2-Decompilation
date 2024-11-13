#include "RetroEngine.hpp"
#include <stdlib.h>

int stageListCount[STAGELIST_MAX];
char stageListNames[STAGELIST_MAX][0x20] = {
    "Presentation Stages",
    "Regular Stages",
    "Bonus Stages",
    "Special Stages",
};
SceneInfo stageList[STAGELIST_MAX][0x100];

int StageMode = STAGEMODE_LOAD;

int CameraStyle   = CAMERASTYLE_FOLLOW;
int CameraEnabled = false;
int cameraAdjustY = 0;
int XScrollOffset = 0;
int YScrollOffset = 0;
int YScrollA      = 0;
int YScrollB      = SCREEN_YSIZE;
int XScrollA      = 0;
int XScrollB      = SCREEN_XSIZE;
int YScrollMove   = 0;
int EarthquakeX   = 0;
int EarthquakeY   = 0;
int XScrollMove   = 0;

int XBoundary1    = 0;
int NewXBoundary1 = 0;
int YBoundary1    = 0;
int NewYBoundary1 = 0;
int XBoundary2    = 0;
int YBoundary2    = 0;
int WaterLevel    = 0x7FFFFFF;
int waterDrawPos  = SCREEN_YSIZE;
int NewXBoundary2 = 0;
int NewYBoundary2 = 0;

int SCREEN_SCROLL_LEFT  = SCREEN_CENTERX - 8;
int SCREEN_SCROLL_RIGHT = SCREEN_CENTERX + 8;

int LastYSize = -1;
int LastXSize = -1;

bool PauseEnabled = true;
bool TimeEnabled  = true;
bool debugMode    = false;
int FrameCounter  = 0;
int MilliSeconds  = 0;
int Seconds       = 0;
int Minutes       = 0;

// Category and Scene IDs
int ActiveStageList   = 0;
int StageListPosition = 0;
char CurrentStageFolder[0x100];
int ActNumber = 0;

char titleCardText[0x100];
byte titleCardWord2 = 0;

byte activeTileLayers[4];
byte tLayerMidPoint;
TileLayer StageLayouts[LAYER_COUNT];

int BGDeformationData1[DEFORM_COUNT];
int BGDeformationData2[DEFORM_COUNT];
int BGDeformationData3[DEFORM_COUNT];
int BGDeformationData4[DEFORM_COUNT];

int DeformationPos1  = 0;
int DeformationPos2 = 0;
int DeformationPos3  = 0;
int DeformationPos4 = 0;

LineScroll HParallax;
LineScroll VParallax;

Tiles128x128 StageTiles;
CollisionMasks TileCollisions[2];

byte TileGfx[TILESET_SIZE];

void ProcessStage(void) {
    switch (StageMode) {
        case STAGEMODE_LOAD:
            CameraEnabled = true;
            XScrollOffset = 0;
            YScrollOffset = 0;
            PauseEnabled  = false;
            TimeEnabled   = false;
            MilliSeconds  = 0;
            Seconds       = 0;
            Minutes       = 0;
            LoadStageFiles();
            for (int i = 0; i < PLAYER_COUNT; ++i) {
                PlayerList[i].visible           = true;
                PlayerList[i].state             = 0;
                PlayerList[i].collisionPlane    = 0;
                PlayerList[i].collisionMode     = 0;
                PlayerList[i].gravity           = 1; // Air
                PlayerList[i].YVelocity         = 0;
                PlayerList[i].XVelocity         = 0;
                PlayerList[i].speed             = 0;
                PlayerList[i].direction         = FLIP_NONE;
                PlayerList[i].tileCollisions    = true;
                PlayerList[i].objectInteraction = true;
            }
            StageMode = STAGEMODE_NORMAL;
            break;
        case STAGEMODE_NORMAL:
            if (PaletteMode > 0)
                PaletteMode--;

            LastXSize = -1;
            LastYSize = -1;
            CheckKeyDown(&GKeyDown, 0xFF);
            CheckKeyPress(&GKeyPress, 0xFF);
            if (PauseEnabled && GKeyPress.start) {
                StageMode = STAGEMODE_PAUSED;
                PauseSound();
            }

            if (TimeEnabled) {
#if !RETRO_USE_ORIGINAL_CODE
                if (++FrameCounter == Engine.refreshRate) {
#else
                if (++FrameCounter == 60) {
#endif
                    FrameCounter = 0;
                    if (++Seconds > 59) {
                        Seconds = 0;
                        if (++Minutes > 59)
                            Minutes = 0;
                    }
                }
                MilliSeconds = 100 * FrameCounter / (!RETRO_USE_ORIGINAL_CODE ? Engine.refreshRate : 60);
            }

            // Update
            ProcessObjects();

            if (ObjectEntityList[0].type == OBJ_TYPE_PLAYER) {
                if (CameraEnabled) {
                    switch (CameraStyle) {
                        case CAMERASTYLE_FOLLOW: SetPlayerScreenPosition(&PlayerList[0]); break;
                        case CAMERASTYLE_EXTENDED: SetPlayerScreenPositionCDStyle(&PlayerList[0]); break;
                        default: break;
                    }
                } else {
                    SetPlayerLockedScreenPosition(&PlayerList[0]);
                }
            }

            DrawStageGfx();
            FlipScreen();
            break;
        case STAGEMODE_PAUSED:
            if (PaletteMode)
                PaletteMode--;

            LastXSize = -1;
            LastYSize = -1;
            CheckKeyDown(&GKeyDown, 0xFF);
            CheckKeyPress(&GKeyPress, 0xFF);

            if (GKeyPress.C) {
                GKeyPress.C = false;
                if (TimeEnabled) {
#if !RETRO_USE_ORIGINAL_CODE
                    if (++FrameCounter == Engine.refreshRate) {
#else
                    if (++FrameCounter == 60) {
#endif
                        FrameCounter = 0;
                        if (++Seconds > 59) {
                            Seconds = 0;
                            if (++Minutes > 59)
                                Minutes = 0;
                        }
                    }
                    MilliSeconds = 100 * FrameCounter / (!RETRO_USE_ORIGINAL_CODE ? Engine.refreshRate : 60);
                }

                // Update
                ProcessObjects();

                if (ObjectEntityList[0].type == OBJ_TYPE_PLAYER) {
                    if (CameraEnabled) {
                        switch (CameraStyle) {
                            case CAMERASTYLE_FOLLOW: SetPlayerScreenPosition(&PlayerList[0]); break;
                            case CAMERASTYLE_EXTENDED: SetPlayerScreenPositionCDStyle(&PlayerList[0]); break;
                            default: break;
                        }
                    } else {
                        SetPlayerLockedScreenPosition(&PlayerList[0]);
                    }
                }
                DrawStageGfx();
            }

            if (GKeyPress.start) {
                StageMode = STAGEMODE_NORMAL;
                ResumeSound();
            }
            break;
    }
}

void LoadStageFiles(void) {
    StopAllSfx();
    FileInfo infoStore;
    FileInfo info;
    byte fileBuffer  = 0;
    byte fileBuffer2 = 0;
    int scriptID     = 2;
    char strBuffer[0x100];

    if (!CheckCurrentStageFolder(StageListPosition)) {
#if !RETRO_USE_ORIGINAL_CODE
        PrintLog("Loading Scene %s - %s", stageListNames[ActiveStageList], stageList[ActiveStageList][StageListPosition].name);
#endif
        ReleaseStageSfx();
        LoadPalette("Data/Palettes/MasterPalette.act", 0, 256);
        ClearScriptData();
        for (int i = SPRITESHEETS_MAX; i > 0; i--) RemoveGraphicsFile((char *)"", i - 1);

        bool loadGlobals = false;
        if (LoadStageFile("StageConfig.bin", StageListPosition, &info)) {
            FileRead(&loadGlobals, 1);
            CloseFile();
        }
        if (loadGlobals && LoadFile("Data/Game/GameConfig.bin", &info)) {
            FileRead(&fileBuffer, 1);
            FileRead(&strBuffer, fileBuffer);
            FileRead(&fileBuffer, 1);
            FileRead(&strBuffer, fileBuffer);
            FileRead(&fileBuffer, 1);
            FileRead(&strBuffer, fileBuffer);

            byte globalScriptCount = 0;
            FileRead(&globalScriptCount, 1);
            for (byte i = 0; i < globalScriptCount; ++i) {
                FileRead(&fileBuffer2, 1);
                FileRead(strBuffer, fileBuffer2);
                strBuffer[fileBuffer2] = 0;
                GetFileInfo(&infoStore);
                CloseFile();
                ParseScriptFile(strBuffer, scriptID++);
                SetFileInfo(&infoStore);
            }
            CloseFile();
        }

        if (LoadStageFile("StageConfig.bin", StageListPosition, &info)) {
            FileRead(&fileBuffer, 1); // Load Globals
            for (int i = 96; i < 128; ++i) {
                byte clr[3];
                FileRead(&clr, 3);
                SetPaletteEntry(i, clr[0], clr[1], clr[2]);
            }

            byte stageScriptCount = 0;
            FileRead(&stageScriptCount, 1);
            for (byte i = 0; i < stageScriptCount; ++i) {
                FileRead(&fileBuffer2, 1);
                FileRead(strBuffer, fileBuffer2);
                strBuffer[fileBuffer2] = 0;
                GetFileInfo(&infoStore);
                CloseFile();
                ParseScriptFile(strBuffer, scriptID + i);
                SetFileInfo(&infoStore);
            }

            FileRead(&fileBuffer2, 1);
            NoStageSFX = fileBuffer2;
            for (int i = 0; i < NoStageSFX; ++i) {
                FileRead(&fileBuffer2, 1);
                FileRead(strBuffer, fileBuffer2);
                strBuffer[fileBuffer2] = 0;
                GetFileInfo(&infoStore);
                CloseFile();
                LoadSfx(strBuffer, NoGlobalSFX + i);
                SetFileInfo(&infoStore);
            }
            CloseFile();
        }
        FileInfo info;
        for (int p = 0; p < PLAYER_COUNT; ++p) {
            if (PlayerScriptList[p].scriptPath[0])
                ParseScriptFile(PlayerScriptList[p].scriptPath, p);
        }

        LoadStageGIFFile(StageListPosition);
        Load128x128Mappings();
        LoadStageCollisions();
        LoadStageBackground();
#if !RETRO_USE_ORIGINAL_CODE
    } else {
        PrintLog("Reloading Scene %s - %s", stageListNames[ActiveStageList], stageList[ActiveStageList][StageListPosition].name);
    }
#endif

    for (int i = 0; i < TRACK_COUNT; ++i) SetMusicTrack((char *)"", i, false);
    for (int i = 0; i < ENTITY_COUNT; ++i) {
        MEM_ZERO(ObjectEntityList[i]);
        ObjectEntityList[i].drawOrder = 3;
        ObjectEntityList[i].scale     = 512;
    }
    LoadActLayout();
    ProcessStartupScripts();

    XScrollA = (PlayerList[0].XPos >> 16) - SCREEN_CENTERX;
    XScrollB = (PlayerList[0].XPos >> 16) - (SCREEN_CENTERX + SCREEN_XSIZE);
    YScrollA = (PlayerList[0].YPos >> 16) - SCREEN_SCROLL_UP;
    YScrollB = (PlayerList[0].YPos >> 16) - (SCREEN_SCROLL_UP + SCREEN_YSIZE);
}
int LoadActFile(const char *ext, int stageID, FileInfo *info) {
    char dest[0x40];

    StrCopy(dest, "Data/Stages/");
    StrAdd(dest, stageList[ActiveStageList][stageID].folder);
    StrAdd(dest, "/Act");
    StrAdd(dest, stageList[ActiveStageList][stageID].id);
    StrAdd(dest, ext);

    ConvertStringToInteger(stageList[ActiveStageList][stageID].id, &ActNumber);

    return LoadFile(dest, info);
}
int LoadStageFile(const char *filePath, int stageID, FileInfo *info) {
    char dest[0x40];

    StrCopy(dest, "Data/Stages/");
    StrAdd(dest, stageList[ActiveStageList][stageID].folder);
    StrAdd(dest, "/");
    StrAdd(dest, filePath);
    return LoadFile(dest, info);
}
void LoadActLayout() {
    FileInfo info;
    if (LoadActFile(".bin", StageListPosition, &info)) {
        byte length = 0;
        FileRead(&length, 1);
        titleCardWord2 = (byte)length;
        for (int i = 0; i < length; i++) {
            FileRead(&titleCardText[i], 1);
            if (titleCardText[i] == '-')
                titleCardWord2 = (byte)(i + 1);
        }
        titleCardText[length] = '\0';

        // READ TILELAYER
        FileRead(activeTileLayers, 4);
        FileRead(&tLayerMidPoint, 1);

        FileRead(&StageLayouts[0].xsize, 1);
        FileRead(&StageLayouts[0].ysize, 1);
        XBoundary1    = 0;
        NewXBoundary1 = 0;
        YBoundary1    = 0;
        NewYBoundary1 = 0;
        XBoundary2    = StageLayouts[0].xsize << 7;
        YBoundary2    = StageLayouts[0].ysize << 7;
        WaterLevel    = YBoundary2 + 128;
        NewXBoundary2 = StageLayouts[0].xsize << 7;
        NewYBoundary2 = StageLayouts[0].ysize << 7;

        for (int i = 0; i < 0x10000; ++i) StageLayouts[0].tiles[i] = 0;

        byte fileBuffer = 0;
        for (int y = 0; y < StageLayouts[0].ysize; ++y) {
            ushort *tiles = &StageLayouts[0].tiles[(y * 0x100)];
            for (int x = 0; x < StageLayouts[0].xsize; ++x) {
                FileRead(&fileBuffer, 1);
                tiles[x] = fileBuffer << 8;
                FileRead(&fileBuffer, 1);
                tiles[x] += fileBuffer;
            }
        }

        // READ TYPENAMES
        FileRead(&fileBuffer, 1);
        int typenameCnt = fileBuffer;
        if (fileBuffer) {
            for (int i = 0; i < typenameCnt; ++i) {
                FileRead(&fileBuffer, 1);
                int nameLen = fileBuffer;
                for (int l = 0; l < nameLen; ++l) FileRead(&fileBuffer, 1);
            }
        }

        // READ OBJECTS
        FileRead(&fileBuffer, 1);
        int objectCount = fileBuffer;
        FileRead(&fileBuffer, 1);
        objectCount    = (objectCount << 8) + fileBuffer;
        Entity *object = &ObjectEntityList[32];
        for (int i = 0; i < objectCount; ++i) {
            FileRead(&fileBuffer, 1);
            object->type = fileBuffer;

            FileRead(&fileBuffer, 1);
            object->propertyValue = fileBuffer;

            FileRead(&fileBuffer, 1);
            object->XPos = fileBuffer << 8;
            FileRead(&fileBuffer, 1);
            object->XPos += fileBuffer;
            object->XPos <<= 16;

            FileRead(&fileBuffer, 1);
            object->YPos = fileBuffer << 8;
            FileRead(&fileBuffer, 1);
            object->YPos += fileBuffer;
            object->YPos <<= 16;

            if (object->type == OBJ_TYPE_PLAYER && PlayerList[0].type == object->propertyValue) {
                Entity *player     = &ObjectEntityList[0];
                player->type       = OBJ_TYPE_PLAYER;
                player->drawOrder  = 4;
                player->priority   = true;
                PlayerList[0].XPos = object->XPos;
                PlayerList[0].YPos = object->YPos;
                SetMovementStats(&PlayerList[0].stats);
                PlayerList[0].walkingSpeed = PlayerScriptList[PlayerList[0].type].startWalkSpeed;
                PlayerList[0].runningSpeed = PlayerScriptList[PlayerList[0].type].startRunSpeed;
                PlayerList[0].jumpingSpeed = PlayerScriptList[PlayerList[0].type].startJumpSpeed;
                object->type               = OBJ_TYPE_BLANKOBJECT;
            }

            ++object;
        }
        StageLayouts[0].type = LAYER_HSCROLL;
        CloseFile();
    }
}
void LoadStageBackground() {
    for (int i = 0; i < LAYER_COUNT; ++i) {
        StageLayouts[i].type = LAYER_NOSCROLL;
    }
    for (int i = 0; i < PARALLAX_COUNT; ++i) {
        HParallax.scrollPos[i] = 0;
        VParallax.scrollPos[i] = 0;
    }

    FileInfo info;
    if (LoadStageFile("Backgrounds.bin", StageListPosition, &info)) {
        byte fileBuffer = 0;
        byte layerCount = 0;
        FileRead(&layerCount, 1);
        FileRead(&HParallax.entryCount, 1);
        for (int i = 0; i < HParallax.entryCount; ++i) {
            FileRead(&fileBuffer, 1);
            HParallax.parallaxFactor[i] = fileBuffer;

            FileRead(&fileBuffer, 1);
            HParallax.scrollSpeed[i] = fileBuffer << 10;

            HParallax.scrollPos[i] = 0;

            FileRead(&HParallax.deform[i], 1);
        }

        FileRead(&VParallax.entryCount, 1);
        for (int i = 0; i < VParallax.entryCount; ++i) {
            FileRead(&fileBuffer, 1);
            VParallax.parallaxFactor[i] = fileBuffer;

            FileRead(&fileBuffer, 1);
            VParallax.scrollSpeed[i] = fileBuffer << 10;

            VParallax.scrollPos[i] = 0;

            FileRead(&VParallax.deform[i], 1);
        }

        for (int i = 1; i < layerCount + 1; ++i) {
            FileRead(&fileBuffer, 1);
            StageLayouts[i].xsize = fileBuffer;
            FileRead(&fileBuffer, 1);
            StageLayouts[i].ysize = fileBuffer;
            FileRead(&fileBuffer, 1);
            StageLayouts[i].type = fileBuffer;
            FileRead(&fileBuffer, 1);
            StageLayouts[i].parallaxFactor = fileBuffer;
            FileRead(&fileBuffer, 1);
            StageLayouts[i].scrollSpeed = fileBuffer << 10;
            StageLayouts[i].scrollPos   = 0;

            memset(StageLayouts[i].tiles, 0, TILELAYER_CHUNK_MAX * sizeof(ushort));
            byte *lineScrollPtr = StageLayouts[i].lineScroll;
            memset(StageLayouts[i].lineScroll, 0, 0x7FFF);

            // Read Line Scroll
            byte buf[3];
            while (true) {
                FileRead(&buf[0], 1);
                if (buf[0] == 0xFF) {
                    FileRead(&buf[1], 1);
                    if (buf[1] == 0xFF) {
                        break;
                    } else {
                        FileRead(&buf[2], 1);
                        int val = buf[1];
                        int cnt = buf[2] - 1;
                        for (int c = 0; c < cnt; ++c) *lineScrollPtr++ = val;
                    }
                } else {
                    *lineScrollPtr++ = buf[0];
                }
            }

            // Read Layout
            for (int y = 0; y < StageLayouts[i].ysize; ++y) {
                ushort *chunks = &StageLayouts[i].tiles[y * 0x100];
                for (int x = 0; x < StageLayouts[i].xsize; ++x) {
                    FileRead(&fileBuffer, 1);
                    *chunks += fileBuffer;
                    ++chunks;
                }
            }
        }

        CloseFile();
    }
}
void Load128x128Mappings() {
    FileInfo info;
    byte entry[3];

    if (LoadStageFile("128x128Tiles.bin", StageListPosition, &info)) {
        for (int i = 0; i < CHUNKTILE_COUNT; ++i) {
            FileRead(&entry, 3);
            entry[0] -= (byte)((entry[0] >> 6) << 6);

            StageTiles.visualPlane[i] = (byte)(entry[0] >> 4);
            entry[0] -= 16 * (entry[0] >> 4);

            StageTiles.direction[i] = (byte)(entry[0] >> 2);
            entry[0] -= 4 * (entry[0] >> 2);

            StageTiles.tileIndex[i]  = entry[1] + (entry[0] << 8);
            StageTiles.gfxDataPos[i] = StageTiles.tileIndex[i] << 8;

            StageTiles.collisionFlags[0][i] = entry[2] >> 4;
            StageTiles.collisionFlags[1][i] = entry[2] - ((entry[2] >> 4) << 4);
        }
        CloseFile();
    }
}
void LoadStageCollisions() {
    FileInfo info;
    if (LoadStageFile("CollisionMasks.bin", StageListPosition, &info)) {

        byte fileBuffer = 0;
        int tileIndex   = 0;
        for (int t = 0; t < 1024; ++t) {
            for (int p = 0; p < 2; ++p) {
                FileRead(&fileBuffer, 1);
                bool isCeiling             = fileBuffer >> 4;
                TileCollisions[p].flags[t] = fileBuffer & 0xF;
                FileRead(&fileBuffer, 1);
                TileCollisions[p].angles[t] = fileBuffer;
                FileRead(&fileBuffer, 1);
                TileCollisions[p].angles[t] += fileBuffer << 8;
                FileRead(&fileBuffer, 1);
                TileCollisions[p].angles[t] += fileBuffer << 16;
                FileRead(&fileBuffer, 1);
                TileCollisions[p].angles[t] += fileBuffer << 24;

                if (isCeiling) // Ceiling Tile
                {
                    for (int c = 0; c < TILE_SIZE; c += 2) {
                        FileRead(&fileBuffer, 1);
                        TileCollisions[p].roofMasks[c + tileIndex]     = fileBuffer >> 4;
                        TileCollisions[p].roofMasks[c + tileIndex + 1] = fileBuffer & 0xF;
                    }

                    // Has Collision (Pt 1)
                    FileRead(&fileBuffer, 1);
                    int id = 1;
                    for (int c = 0; c < TILE_SIZE / 2; ++c) {
                        if (fileBuffer & id) {
                            TileCollisions[p].floorMasks[c + tileIndex + 8] = 0;
                        } else {
                            TileCollisions[p].floorMasks[c + tileIndex + 8] = 0x40;
                            TileCollisions[p].roofMasks[c + tileIndex + 8]  = -0x40;
                        }
                        id <<= 1;
                    }

                    // Has Collision (Pt 2)
                    FileRead(&fileBuffer, 1);
                    id = 1;
                    for (int c = 0; c < TILE_SIZE / 2; ++c) {
                        if (fileBuffer & id) {
                            TileCollisions[p].floorMasks[c + tileIndex] = 0;
                        } else {
                            TileCollisions[p].floorMasks[c + tileIndex] = 0x40;
                            TileCollisions[p].roofMasks[c + tileIndex]  = -0x40;
                        }
                        id <<= 1;
                    }

                    // LWall rotations
                    for (int c = 0; c < TILE_SIZE; ++c) {
                        int h = 0;
                        while (h > -1) {
                            if (h >= TILE_SIZE) {
                                TileCollisions[p].lWallMasks[c + tileIndex] = 0x40;
                                h                                           = -1;
                            } else if (c > TileCollisions[p].roofMasks[h + tileIndex]) {
                                ++h;
                            } else {
                                TileCollisions[p].lWallMasks[c + tileIndex] = h;
                                h                                           = -1;
                            }
                        }
                    }

                    // RWall rotations
                    for (int c = 0; c < TILE_SIZE; ++c) {
                        int h = TILE_SIZE - 1;
                        while (h < TILE_SIZE) {
                            if (h <= -1) {
                                TileCollisions[p].rWallMasks[c + tileIndex] = -0x40;
                                h                                           = TILE_SIZE;
                            } else if (c > TileCollisions[p].roofMasks[h + tileIndex]) {
                                --h;
                            } else {
                                TileCollisions[p].rWallMasks[c + tileIndex] = h;
                                h                                           = TILE_SIZE;
                            }
                        }
                    }
                } else // Regular Tile
                {
                    for (int c = 0; c < TILE_SIZE; c += 2) {
                        FileRead(&fileBuffer, 1);
                        TileCollisions[p].floorMasks[c + tileIndex]     = fileBuffer >> 4;
                        TileCollisions[p].floorMasks[c + tileIndex + 1] = fileBuffer & 0xF;
                    }
                    FileRead(&fileBuffer, 1);
                    int id = 1;
                    for (int c = 0; c < TILE_SIZE / 2; ++c) // HasCollision
                    {
                        if (fileBuffer & id) {
                            TileCollisions[p].roofMasks[c + tileIndex + 8] = 0xF;
                        } else {
                            TileCollisions[p].floorMasks[c + tileIndex + 8] = 0x40;
                            TileCollisions[p].roofMasks[c + tileIndex + 8]  = -0x40;
                        }
                        id <<= 1;
                    }

                    FileRead(&fileBuffer, 1);
                    id = 1;
                    for (int c = 0; c < TILE_SIZE / 2; ++c) // HasCollision (pt 2)
                    {
                        if (fileBuffer & id) {
                            TileCollisions[p].roofMasks[c + tileIndex] = 0xF;
                        } else {
                            TileCollisions[p].floorMasks[c + tileIndex] = 0x40;
                            TileCollisions[p].roofMasks[c + tileIndex]  = -0x40;
                        }
                        id <<= 1;
                    }

                    // LWall rotations
                    for (int c = 0; c < TILE_SIZE; ++c) {
                        int h = 0;
                        while (h > -1) {
                            if (h >= TILE_SIZE) {
                                TileCollisions[p].lWallMasks[c + tileIndex] = 0x40;
                                h                                           = -1;
                            } else if (c < TileCollisions[p].floorMasks[h + tileIndex]) {
                                ++h;
                            } else {
                                TileCollisions[p].lWallMasks[c + tileIndex] = h;
                                h                                           = -1;
                            }
                        }
                    }

                    // RWall rotations
                    for (int c = 0; c < TILE_SIZE; ++c) {
                        int h = TILE_SIZE - 1;
                        while (h < TILE_SIZE) {
                            if (h <= -1) {
                                TileCollisions[p].rWallMasks[c + tileIndex] = -0x40;
                                h                                           = TILE_SIZE;
                            } else if (c < TileCollisions[p].floorMasks[h + tileIndex]) {
                                --h;
                            } else {
                                TileCollisions[p].rWallMasks[c + tileIndex] = h;
                                h                                           = TILE_SIZE;
                            }
                        }
                    }
                }
            }
            tileIndex += 16;
        }
        CloseFile();
    }
}
void LoadStageGIFFile(int stageID) {
    FileInfo info;
    if (LoadStageFile("16x16Tiles.gif", stageID, &info)) {
        byte fileBuffer = 0;
        int fileBuffer2 = 0;

        SetFilePosition(6); // GIF89a
        FileRead(&fileBuffer, 1);
        int width = fileBuffer;
        FileRead(&fileBuffer, 1);
        width += (fileBuffer << 8);
        FileRead(&fileBuffer, 1);
        int height = fileBuffer;
        FileRead(&fileBuffer, 1);
        height += (fileBuffer << 8);

        FileRead(&fileBuffer, 1); // Palette Size
        int has_pallete  = (fileBuffer & 0x80) >> 7;
        int colors       = ((fileBuffer & 0x70) >> 4) + 1;
        int palette_size = (fileBuffer & 0x7) + 1;
        if (palette_size > 0)
            palette_size = 1 << palette_size;

        FileRead(&fileBuffer, 1); // BG Colour index (thrown away)
        FileRead(&fileBuffer, 1); // Pixel aspect ratio (thrown away)

        if (palette_size == 256) {
            byte clr[3];

            for (int c = 0; c < 0x80; ++c) FileRead(clr, 3);
            for (int c = 0x80; c < 0x100; ++c) {
                FileRead(clr, 3);
                SetPaletteEntry(c, clr[0], clr[1], clr[2]);
            }
        }

        FileRead(&fileBuffer, 1);
        while (fileBuffer != ',') FileRead(&fileBuffer, 1); // gif image start identifier

        FileRead(&fileBuffer2, 2);
        FileRead(&fileBuffer2, 2);
        FileRead(&fileBuffer2, 2);
        FileRead(&fileBuffer2, 2);
        FileRead(&fileBuffer, 1);
        bool interlaced = (fileBuffer & 0x40) >> 6;
        if ((unsigned int)fileBuffer >> 7 == 1) {
            int c = 128;
            do {
                ++c;
                FileRead(&fileBuffer2, 3);
            } while (c != 256);
        }

        ReadGifPictureData(width, height, interlaced, TileGfx, 0);

        byte transparent = TileGfx[0];
        for (int i = 0; i < 0x40000; ++i) {
            if (TileGfx[i] == transparent)
                TileGfx[i] = 0;
        }

        CloseFile();
    }
}
void LoadStageGFXFile(int stageID) {
    FileInfo info;
    if (LoadStageFile("16x16Tiles.gfx", stageID, &info)) {
        byte fileBuffer = 0;
        FileRead(&fileBuffer, 1);
        int width = fileBuffer << 8;
        FileRead(&fileBuffer, 1);
        width += fileBuffer;
        FileRead(&fileBuffer, 1);
        int height = fileBuffer << 8;
        FileRead(&fileBuffer, 1);
        height += fileBuffer;

        byte clr[3];
        for (int i = 0; i < 0x80; ++i) FileRead(&clr, 3); // Palette
        for (int c = 0x80; c < 0x100; ++c) {
            FileRead(clr, 3);
            SetPaletteEntry(c, clr[0], clr[1], clr[2]);
        }

        byte *gfxData = TileGfx;
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

        byte transparent = TileGfx[0];
        for (int i = 0; i < 0x40000; ++i) {
            if (TileGfx[i] == transparent)
                TileGfx[i] = 0;
        }

        CloseFile();
    }
}

void ResetBackgroundSettings() {
    for (int i = 0; i < LAYER_COUNT; ++i) {
        StageLayouts[i].scrollPos = 0;
    }

    for (int i = 0; i < PARALLAX_COUNT; ++i) {
        HParallax.scrollPos[i] = 0;
        VParallax.scrollPos[i] = 0;
    }
}

void SetPlayerScreenPosition(Player *player) {
    PlayerScript *script = &PlayerScriptList[player->type];
    int playerXPos       = player->XPos >> 16;
    int playerYPos       = player->YPos >> 16;
    if (NewYBoundary1 > YBoundary1) {
        if (YScrollOffset <= NewYBoundary1)
            YBoundary1 = YScrollOffset;
        else
            YBoundary1 = NewYBoundary1;
    }
    if (NewYBoundary1 < YBoundary1) {
        if (YScrollOffset <= YBoundary1)
            --YBoundary1;
        else
            YBoundary1 = NewYBoundary1;
    }
    if (NewYBoundary2 < YBoundary2) {
        if (YScrollOffset + SCREEN_YSIZE >= YBoundary2 || YScrollOffset + SCREEN_YSIZE <= NewYBoundary2)
            --YBoundary2;
        else
            YBoundary2 = YScrollOffset + SCREEN_YSIZE;
    }
    if (NewYBoundary2 > YBoundary2) {
        if (YScrollOffset + SCREEN_YSIZE >= YBoundary2)
            ++YBoundary2;
        else
            YBoundary2 = NewYBoundary2;
    }
    if (NewXBoundary1 > XBoundary1) {
        if (XScrollOffset <= NewXBoundary1)
            XBoundary1 = XScrollOffset;
        else
            XBoundary1 = NewXBoundary1;
    }
    if (NewXBoundary1 < XBoundary1) {
        if (XScrollOffset <= XBoundary1)
            --XBoundary1;
        else
            XBoundary1 = NewXBoundary1;
    }
    if (NewXBoundary2 < XBoundary2) {
        if (XScrollOffset + SCREEN_XSIZE >= XBoundary2)
            XBoundary2 = XScrollOffset + SCREEN_XSIZE;
        else
            XBoundary2 = NewXBoundary2;
    }
    if (NewXBoundary2 > XBoundary2) {
        if (XScrollOffset + SCREEN_XSIZE >= XBoundary2)
            ++XBoundary2;
        else
            XBoundary2 = NewXBoundary2;
    }
    int xscrollA     = XScrollA;
    int xscrollB     = XScrollB;
    int scrollAmount = playerXPos - (XScrollA + SCREEN_CENTERX);
    if (abs(scrollAmount) >= 25) {
        if (scrollAmount <= 0)
            xscrollA -= 16;
        else
            xscrollA += 16;
        xscrollB = xscrollA + SCREEN_XSIZE;
    } else {
        if (playerXPos > xscrollA + SCREEN_SCROLL_RIGHT) {
            xscrollA = playerXPos - SCREEN_SCROLL_RIGHT;
            xscrollB = playerXPos - SCREEN_SCROLL_RIGHT + SCREEN_XSIZE;
        }
        if (playerXPos < xscrollA + SCREEN_SCROLL_LEFT) {
            xscrollA = playerXPos - SCREEN_SCROLL_LEFT;
            xscrollB = playerXPos - SCREEN_SCROLL_LEFT + SCREEN_XSIZE;
        }
    }
    if (xscrollA < XBoundary1) {
        xscrollA = XBoundary1;
        xscrollB = XBoundary1 + SCREEN_XSIZE;
    }
    if (xscrollB > XBoundary2) {
        xscrollB = XBoundary2;
        xscrollA = XBoundary2 - SCREEN_XSIZE;
    }
    XScrollA = xscrollA;
    XScrollB = xscrollB;
    if (playerXPos <= xscrollA + SCREEN_CENTERX) {
        player->screenXPos = EarthquakeX + playerXPos - xscrollA;
        XScrollOffset      = xscrollA - EarthquakeX;
    } else {
        XScrollOffset      = playerXPos + EarthquakeX - SCREEN_CENTERX;
        player->screenXPos = SCREEN_CENTERX - EarthquakeX;
        if (playerXPos > xscrollB - SCREEN_CENTERX) {
            player->screenXPos = playerXPos - (xscrollB - SCREEN_CENTERX) + EarthquakeX + SCREEN_CENTERX;
            XScrollOffset      = xscrollB - SCREEN_XSIZE - EarthquakeX;
        }
    }
    int yscrollA     = YScrollA;
    int yscrollB     = YScrollB;
    int hitboxDiff   = PlayerCBoxes[0].bottom[0] - GetPlayerCBoxInstance(player, script)->bottom[0];
    int adjustYPos   = playerYPos - hitboxDiff;
    int adjustAmount = player->lookPos + adjustYPos - (YScrollA + SCREEN_SCROLL_UP);
    if (player->trackScroll) {
        YScrollMove = 32;
    } else {
        if (YScrollMove == 32) {
            YScrollMove = 2 * ((hitboxDiff + SCREEN_SCROLL_UP - player->screenYPos - player->lookPos) >> 1);
            if (YScrollMove > 32)
                YScrollMove = 32;
            if (YScrollMove < -32)
                YScrollMove = -32;
        }
        if (YScrollMove > 0)
            YScrollMove -= 6;
        YScrollMove += YScrollMove < 0 ? 6 : 0;
    }
    int absAdjust = abs(adjustAmount);
    if (absAdjust >= abs(YScrollMove) + 17) {
        if (adjustAmount <= 0)
            yscrollA -= 16;
        else
            yscrollA += 16;
        yscrollB = yscrollA + SCREEN_YSIZE;
    } else if (YScrollMove == 32) {
        if (player->lookPos + adjustYPos > yscrollA + YScrollMove + SCREEN_SCROLL_UP) {
            yscrollA = player->lookPos + adjustYPos - (YScrollMove + SCREEN_SCROLL_UP);
            yscrollB = yscrollA + SCREEN_YSIZE;
        }
        if (player->lookPos + adjustYPos < yscrollA + SCREEN_SCROLL_UP - YScrollMove) {
            yscrollA = player->lookPos + adjustYPos - (SCREEN_SCROLL_UP - YScrollMove);
            yscrollB = yscrollA + SCREEN_YSIZE;
        }
    } else {
        yscrollA = player->lookPos + adjustYPos + YScrollMove - SCREEN_SCROLL_UP;
        yscrollB = yscrollA + SCREEN_YSIZE;
    }

    if (yscrollA < YBoundary1) {
        yscrollA = YBoundary1;
        yscrollB = YBoundary1 + SCREEN_YSIZE;
    }
    if (yscrollB > YBoundary2) {
        yscrollB = YBoundary2;
        yscrollA = YBoundary2 - SCREEN_YSIZE;
    }
    YScrollA = yscrollA;
    YScrollB = yscrollB;

    if (EarthquakeY) {
        if (EarthquakeY <= 0)
            EarthquakeY = ~EarthquakeY;
        else
            EarthquakeY = -EarthquakeY;
    }

    if (player->lookPos + adjustYPos <= yscrollA + SCREEN_SCROLL_UP) {
        player->screenYPos = adjustYPos - yscrollA - EarthquakeY;
        YScrollOffset      = EarthquakeY + yscrollA;
    } else {
        YScrollOffset      = EarthquakeY + adjustYPos + player->lookPos - SCREEN_SCROLL_UP;
        player->screenYPos = SCREEN_SCROLL_UP - player->lookPos - EarthquakeY;
        if (player->lookPos + adjustYPos > yscrollB - SCREEN_SCROLL_DOWN) {
            player->screenYPos = adjustYPos - (yscrollB - SCREEN_SCROLL_DOWN) + EarthquakeY + SCREEN_SCROLL_UP;
            YScrollOffset      = yscrollB - SCREEN_YSIZE - EarthquakeY;
        }
    }
    player->screenYPos += hitboxDiff;
}
void SetPlayerScreenPositionCDStyle(Player *player) {
    PlayerScript *script = &PlayerScriptList[player->type];
    int playerXPos       = player->XPos >> 16;
    int playerYPos       = player->YPos >> 16;
    if (NewYBoundary1 > YBoundary1) {
        if (YScrollOffset <= NewYBoundary1)
            YBoundary1 = YScrollOffset;
        else
            YBoundary1 = NewYBoundary1;
    }
    if (NewYBoundary1 < YBoundary1) {
        if (YScrollOffset <= YBoundary1)
            --YBoundary1;
        else
            YBoundary1 = NewYBoundary1;
    }
    if (NewYBoundary2 < YBoundary2) {
        if (YScrollOffset + SCREEN_YSIZE >= YBoundary2 || YScrollOffset + SCREEN_YSIZE <= NewYBoundary2)
            --YBoundary2;
        else
            YBoundary2 = YScrollOffset + SCREEN_YSIZE;
    }
    if (NewYBoundary2 > YBoundary2) {
        if (YScrollOffset + SCREEN_YSIZE >= YBoundary2)
            ++YBoundary2;
        else
            YBoundary2 = NewYBoundary2;
    }
    if (NewXBoundary1 > XBoundary1) {
        if (XScrollOffset <= NewXBoundary1)
            XBoundary1 = XScrollOffset;
        else
            XBoundary1 = NewXBoundary1;
    }
    if (NewXBoundary1 < XBoundary1) {
        if (XScrollOffset <= XBoundary1)
            --XBoundary1;
        else
            XBoundary1 = NewXBoundary1;
    }
    if (NewXBoundary2 < XBoundary2) {
        if (XScrollOffset + SCREEN_XSIZE >= XBoundary2)
            XBoundary2 = XScrollOffset + SCREEN_XSIZE;
        else
            XBoundary2 = NewXBoundary2;
    }
    if (NewXBoundary2 > XBoundary2) {
        if (XScrollOffset + SCREEN_XSIZE >= XBoundary2)
            ++XBoundary2;
        else
            XBoundary2 = NewXBoundary2;
    }

    if (!player->gravity) {
        if (player->direction) {
            if (player->animation == ANI_PEELOUT || player->animation == ANI_SPINDASH || player->speed < -0x5F5C2) {
                if (XScrollMove < 64)
                    XScrollMove += 2;
            } else {
                XScrollMove += XScrollMove < 0 ? 2 : 0;
                if (XScrollMove > 0)
                    XScrollMove -= 2;
            }
        } else if (player->animation == ANI_PEELOUT || player->animation == ANI_SPINDASH || player->speed > 0x5F5C2) {
            if (XScrollMove > -64)
                XScrollMove -= 2;
        } else {
            XScrollMove += XScrollMove < 0 ? 2 : 0;
            if (XScrollMove > 0)
                XScrollMove -= 2;
        }
    }
    if (playerXPos <= XBoundary1 + XScrollMove + SCREEN_CENTERX) {
        player->screenXPos = EarthquakeX + playerXPos - XBoundary1;
        XScrollOffset      = XBoundary1 - EarthquakeX;
    } else {
        XScrollOffset      = playerXPos + EarthquakeX - XScrollMove - SCREEN_CENTERX;
        player->screenXPos = XScrollMove - EarthquakeX + SCREEN_CENTERX;
        if (playerXPos - XScrollMove > XBoundary2 - SCREEN_CENTERX) {
            player->screenXPos = EarthquakeX + playerXPos - XBoundary2 + SCREEN_XSIZE;
            XScrollOffset      = XBoundary2 - SCREEN_XSIZE - EarthquakeX;
        }
    }

    XScrollA = XScrollOffset;
    XScrollB = (XScrollA + SCREEN_XSIZE);

    int yscrollA     = YScrollA;
    int yscrollB     = YScrollB;
    int hitboxDiff   = PlayerCBoxes[0].bottom[0] - GetPlayerCBoxInstance(player, script)->bottom[0];
    int adjustY      = playerYPos - hitboxDiff;
    int adjustOffset = player->lookPos + adjustY - (YScrollA + SCREEN_SCROLL_UP);
    if (player->trackScroll) {
        YScrollMove = 32;
    } else {
        if (YScrollMove == 32) {
            YScrollMove = ((hitboxDiff + SCREEN_SCROLL_UP - player->screenYPos - player->lookPos) >> 1) << 1;
            if (YScrollMove > 32)
                YScrollMove = 32;
            if (YScrollMove < -32)
                YScrollMove = -32;
        }
        if (YScrollMove > 0)
            YScrollMove -= 6;
        YScrollMove += YScrollMove < 0 ? 6 : 0;
    }
    int absAdjust = abs(adjustOffset);
    if (absAdjust >= abs(YScrollMove) + 17) {
        if (adjustOffset <= 0)
            yscrollA -= 16;
        else
            yscrollA += 16;
        yscrollB = yscrollA + SCREEN_YSIZE;
    } else if (YScrollMove == 32) {
        if (player->lookPos + adjustY > yscrollA + YScrollMove + SCREEN_SCROLL_UP) {
            yscrollA = player->lookPos + adjustY - (YScrollMove + SCREEN_SCROLL_UP);
            yscrollB = yscrollA + SCREEN_YSIZE;
        }
        if (player->lookPos + adjustY < yscrollA + SCREEN_SCROLL_UP - YScrollMove) {
            yscrollA = player->lookPos + adjustY - (SCREEN_SCROLL_UP - YScrollMove);
            yscrollB = yscrollA + SCREEN_YSIZE;
        }
    } else {
        yscrollA = player->lookPos + adjustY + YScrollMove - SCREEN_SCROLL_UP;
        yscrollB = yscrollA + SCREEN_YSIZE;
    }
    if (yscrollA < YBoundary1) {
        yscrollA = YBoundary1;
        yscrollB = YBoundary1 + SCREEN_YSIZE;
    }
    if (yscrollB > YBoundary2) {
        yscrollB = YBoundary2;
        yscrollA = YBoundary2 - SCREEN_YSIZE;
    }

    YScrollA = yscrollA;
    YScrollB = yscrollB;

    if (EarthquakeY) {
        if (EarthquakeY <= 0)
            EarthquakeY = ~EarthquakeY;

        else
            EarthquakeY = -EarthquakeY;
    }

    if (player->lookPos + adjustY <= yscrollA + SCREEN_SCROLL_UP) {
        player->screenYPos = adjustY - yscrollA - EarthquakeY;
        YScrollOffset      = EarthquakeY + yscrollA;
    } else {
        YScrollOffset      = EarthquakeY + adjustY + player->lookPos - SCREEN_SCROLL_UP;
        player->screenYPos = SCREEN_SCROLL_UP - player->lookPos - EarthquakeY;
        if (player->lookPos + adjustY > yscrollB - SCREEN_SCROLL_DOWN) {
            player->screenYPos = adjustY - (yscrollB - SCREEN_SCROLL_DOWN) + EarthquakeY + SCREEN_SCROLL_UP;
            YScrollOffset      = yscrollB - SCREEN_YSIZE - EarthquakeY;
        }
    }

    player->screenYPos += hitboxDiff;
}
void SetPlayerLockedScreenPosition(Player *player) {
    PlayerScript *script = &PlayerScriptList[player->type];
    int playerXPos       = player->XPos >> 16;
    int playerYPos       = player->YPos >> 16;
    switch (CameraStyle) {
        case CAMERASTYLE_FOLLOW: {
            if (playerXPos <= XBoundary1 + XScrollMove + SCREEN_CENTERX) {
                player->screenXPos = EarthquakeX + playerXPos - XBoundary1;
                XScrollOffset      = XBoundary1 - EarthquakeX;
            } else {
                XScrollOffset      = playerXPos + EarthquakeX - SCREEN_CENTERX - XScrollMove;
                player->screenXPos = XScrollMove + SCREEN_CENTERX - EarthquakeX;
                if (playerXPos > XBoundary2 + XScrollMove - SCREEN_CENTERX) {
                    player->screenXPos = XScrollMove + playerXPos - (XBoundary2 - SCREEN_CENTERX) + EarthquakeX + SCREEN_CENTERX;
                    XScrollOffset      = XBoundary2 - SCREEN_XSIZE - EarthquakeX - XScrollMove;
                }
            }
            break;
        }
        case CAMERASTYLE_EXTENDED: {
            int xscrollA = XScrollA;
            int xscrollB = XScrollB;
            if (playerXPos <= XScrollA + SCREEN_CENTERX) {
                player->screenXPos = EarthquakeX + playerXPos - XScrollA;
                XScrollOffset      = xscrollA - EarthquakeX;
            } else {
                XScrollOffset      = playerXPos + EarthquakeX - SCREEN_CENTERX;
                player->screenXPos = SCREEN_CENTERX - EarthquakeX;
                if (playerXPos > xscrollB - SCREEN_CENTERX) {
                    player->screenXPos = playerXPos - (xscrollB - SCREEN_CENTERX) + EarthquakeX + SCREEN_CENTERX;
                    XScrollOffset      = xscrollB - SCREEN_XSIZE - EarthquakeX;
                }
            }
            break;
        }
        default: break;
    }
    int yscrollA   = YScrollA;
    int yscrollB   = YScrollB;
    int hitboxDiff = PlayerCBoxes[0].bottom[0] - GetPlayerCBoxInstance(player, script)->bottom[0];
    int adjustY    = playerYPos - hitboxDiff;

    if (EarthquakeY) {
        if (EarthquakeY <= 0)
            EarthquakeY = ~EarthquakeY;
        else
            EarthquakeY = -EarthquakeY;
    }

    if (player->lookPos + adjustY <= YScrollA + SCREEN_SCROLL_UP) {
        player->screenYPos = adjustY - YScrollA - EarthquakeY;
        YScrollOffset      = EarthquakeY + yscrollA;
    } else {
        YScrollOffset      = EarthquakeY + adjustY + player->lookPos - SCREEN_SCROLL_UP;
        player->screenYPos = SCREEN_SCROLL_UP - player->lookPos - EarthquakeY;
        if (player->lookPos + adjustY > yscrollB - SCREEN_SCROLL_DOWN) {
            player->screenYPos = adjustY - (yscrollB - SCREEN_SCROLL_DOWN) + EarthquakeY + SCREEN_SCROLL_UP;
            YScrollOffset      = yscrollB - SCREEN_YSIZE - EarthquakeY;
        }
    }
    player->screenYPos += hitboxDiff;
}