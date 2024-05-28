#include "RetroEngine.hpp"

int SCREEN_XSIZE   = 320;
int SCREEN_CENTERX = 320 / 2;

byte blendLookupTable[0x100 * 0x100];

byte tintTable0[0x100];
byte tintTable1[0x100];
byte tintTable2[0x100];
byte tintTable3[0x100];

DrawListEntry drawListEntries[DRAWLAYER_COUNT];

int gfxDataPosition;
GFXSurface gfxSurface[SURFACE_MAX];
byte graphicData[GFXDATA_MAX];

int InitRenderDevice()
{
    char gameTitle[0x40];

    sprintf(gameTitle, "%s%s", Engine.gameWindowText, Engine.usingBinFile ? "" : " (Using Data Folder)");

    Engine.pixelBuffer = new byte[SCREEN_XSIZE * SCREEN_YSIZE];
    memset(Engine.pixelBuffer, 0, (SCREEN_XSIZE * SCREEN_YSIZE) * sizeof(byte));

#if RETRO_USING_SDL2
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, Engine.vsync ? "1" : "0");
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
    SDL_SetHint(SDL_HINT_WINRT_HANDLE_BACK_BUTTON, "1");

    byte flags      = 0;
    Engine.window   = SDL_CreateWindow(gameTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_XSIZE * Engine.windowScale,
                                     SCREEN_YSIZE * Engine.windowScale, SDL_WINDOW_ALLOW_HIGHDPI | flags);
    Engine.renderer = SDL_CreateRenderer(Engine.window, -1, SDL_RENDERER_ACCELERATED);

    if (!Engine.window) {
        printLog("ERROR: failed to create window!");
        Engine.gameMode = ENGINE_EXITGAME;
        return 0;
    }

    if (!Engine.renderer) {
        printLog("ERROR: failed to create renderer!");
        Engine.gameMode = ENGINE_EXITGAME;
        return 0;
    }

    SDL_RenderSetLogicalSize(Engine.renderer, SCREEN_XSIZE, SCREEN_YSIZE);
    SDL_SetRenderDrawBlendMode(Engine.renderer, SDL_BLENDMODE_BLEND);

    int colourMode = 0;
    if (Engine.colourMode == 1)
        colourMode = SDL_PIXELFORMAT_RGB565;
    else
        colourMode = SDL_PIXELFORMAT_RGB888;

    Engine.screenBuffer = SDL_CreateTexture(Engine.renderer, colourMode, SDL_TEXTUREACCESS_STREAMING, SCREEN_XSIZE, SCREEN_YSIZE);

    if (!Engine.screenBuffer) {
        printLog("ERROR: failed to create screen buffer!\nerror msg: %s", SDL_GetError());
        return 0;
    }

    if (Engine.startFullScreen) {
        SDL_RestoreWindow(Engine.window);
        SDL_SetWindowFullscreen(Engine.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        Engine.isFullScreen = true;
    }

    if (Engine.borderless) {
        SDL_RestoreWindow(Engine.window);
        SDL_SetWindowBordered(Engine.window, SDL_FALSE);
    }

    SDL_SetWindowPosition(Engine.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    SDL_DisplayMode disp;
    int winID = SDL_GetWindowDisplayIndex(Engine.window);
    if (SDL_GetCurrentDisplayMode(winID, &disp) == 0) {
        Engine.screenRefreshRate = disp.refresh_rate;
    }
    else {
        printf("error: %s", SDL_GetError());
    }

#if RETRO_PLATFORM == RETRO_iOS
    SDL_RestoreWindow(Engine.window);
    SDL_SetWindowFullscreen(Engine.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    Engine.isFullScreen = true;
#endif

#endif

#if RETRO_USING_SDL1
    SDL_Init(SDL_INIT_EVERYTHING);

    // SDL1.2 doesn't support hints it seems
    // SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    // SDL_SetHint(SDL_HINT_RENDER_VSYNC, Engine.vsync ? "1" : "0");
    // SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
    // SDL_SetHint(SDL_HINT_WINRT_HANDLE_BACK_BUTTON, "1");

    Engine.windowSurface = SDL_SetVideoMode(SCREEN_XSIZE * Engine.windowScale, SCREEN_YSIZE * Engine.windowScale, 32, SDL_SWSURFACE);
    if (!Engine.windowSurface) {
        printLog("ERROR: failed to create window!\nerror msg: %s", SDL_GetError());
        return 0;
    }
    // Set the window caption
    SDL_WM_SetCaption(gameTitle, NULL);

    Engine.screenBuffer =
        SDL_CreateRGBSurface(0, SCREEN_XSIZE * Engine.windowScale, SCREEN_YSIZE * Engine.windowScale, 16, 0xF800, 0x7E0, 0x1F, 0x00);

    if (!Engine.screenBuffer) {
        printLog("ERROR: failed to create screen buffer!\nerror msg: %s", SDL_GetError());
        return 0;
    }

    /*Engine.screenBuffer2x = SDL_SetVideoMode(SCREEN_XSIZE * 2, SCREEN_YSIZE * 2, 16, SDL_SWSURFACE);

    if (!Engine.screenBuffer2x) {
        printLog("ERROR: failed to create screen buffer HQ!\nerror msg: %s", SDL_GetError());
        return 0;
    }*/

    if (Engine.startFullScreen) {
        Engine.windowSurface =
            SDL_SetVideoMode(SCREEN_XSIZE * Engine.windowScale, SCREEN_YSIZE * Engine.windowScale, 16, SDL_SWSURFACE | SDL_FULLSCREEN);
        SDL_ShowCursor(SDL_FALSE);
        Engine.isFullScreen = true;
    }

    // TODO: not supported in 1.2?
    if (Engine.borderless) {
        // SDL_RestoreWindow(Engine.window);
        // SDL_SetWindowBordered(Engine.window, SDL_FALSE);
    }

    // SDL_SetWindowPosition(Engine.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    Engine.useHQModes = false; // disabled
    Engine.borderless = false; // disabled
#endif

    OBJECT_BORDER_X2 = SCREEN_XSIZE + 0x80;
    // OBJECT_BORDER_Y2 = SCREEN_YSIZE + 0x100;

    return 1;
}
void FlipScreen()
{
    if (Engine.gameMode == ENGINE_EXITGAME)
        return;

        // Clear the screen. This is needed to keep the
        // pillarboxes in fullscreen from displaying garbage data.
#if RETRO_USING_SDL2
    SDL_RenderClear(Engine.renderer);
#endif

    int pitch    = 0;
    void *pixels = NULL;

    switch (Engine.colourMode) {
        case 0: // 8-bit
        {
            uint *frameBuffer = new uint[SCREEN_XSIZE * SCREEN_YSIZE];
            memset(frameBuffer, 0, (SCREEN_XSIZE * SCREEN_YSIZE) * sizeof(uint));
            if (fadeMode) {
                for (int y = 0; y < waterDrawPos; ++y) {
                    for (int x = 0; x < SCREEN_XSIZE; ++x) {
                        Colour clr                          = palette8F[Engine.pixelBuffer[(y * SCREEN_XSIZE) + x]];
                        frameBuffer[(y * SCREEN_XSIZE) + x] = PACK_RGB888(clr.r, clr.g, clr.b);
                    }
                }

                for (int y = waterDrawPos; y < SCREEN_YSIZE; ++y) {
                    for (int x = 0; x < SCREEN_XSIZE; ++x) {
                        Colour clr                          = palette8WF[Engine.pixelBuffer[(y * SCREEN_XSIZE) + x]];
                        frameBuffer[(y * SCREEN_XSIZE) + x] = PACK_RGB888(clr.r, clr.g, clr.b);
                    }
                }
            }
            else {
                for (int y = 0; y < waterDrawPos; ++y) {
                    for (int x = 0; x < SCREEN_XSIZE; ++x) {
                        Colour clr                          = palette8[Engine.pixelBuffer[(y * SCREEN_XSIZE) + x]];
                        frameBuffer[(y * SCREEN_XSIZE) + x] = PACK_RGB888(clr.r, clr.g, clr.b);
                    }
                }

                for (int y = waterDrawPos; y < SCREEN_YSIZE; ++y) {
                    for (int x = 0; x < SCREEN_XSIZE; ++x) {
                        Colour clr                          = palette8W[Engine.pixelBuffer[(y * SCREEN_XSIZE) + x]];
                        frameBuffer[(y * SCREEN_XSIZE) + x] = PACK_RGB888(clr.r, clr.g, clr.b);
                    }
                }
            }

#if RETRO_USING_SDL2
            SDL_LockTexture(Engine.screenBuffer, NULL, &pixels, &pitch);
            memcpy(pixels, frameBuffer, pitch * SCREEN_YSIZE);
            SDL_UnlockTexture(Engine.screenBuffer);
#endif
            if (frameBuffer)
                delete[] frameBuffer;
            break;
        }
        case 1: // 16-bit
        {
            ushort *frameBuffer = new ushort[SCREEN_XSIZE * SCREEN_YSIZE];
            memset(frameBuffer, 0, (SCREEN_XSIZE * SCREEN_YSIZE) * sizeof(ushort));
            if (fadeMode) {
                for (int y = 0; y < waterDrawPos; ++y) {
                    for (int x = 0; x < SCREEN_XSIZE; ++x) {
                        frameBuffer[(y * SCREEN_XSIZE) + x] = palette16F[Engine.pixelBuffer[(y * SCREEN_XSIZE) + x]];
                    }
                }

                for (int y = waterDrawPos; y < SCREEN_YSIZE; ++y) {
                    for (int x = 0; x < SCREEN_XSIZE; ++x) {
                        frameBuffer[(y * SCREEN_XSIZE) + x] = palette16WF[Engine.pixelBuffer[(y * SCREEN_XSIZE) + x]];
                    }
                }
            }
            else {
                for (int y = 0; y < waterDrawPos; ++y) {
                    for (int x = 0; x < SCREEN_XSIZE; ++x) {
                        frameBuffer[(y * SCREEN_XSIZE) + x] = palette16[Engine.pixelBuffer[(y * SCREEN_XSIZE) + x]];
                    }
                }

                for (int y = waterDrawPos; y < SCREEN_YSIZE; ++y) {
                    for (int x = 0; x < SCREEN_XSIZE; ++x) {
                        frameBuffer[(y * SCREEN_XSIZE) + x] = palette16W[Engine.pixelBuffer[(y * SCREEN_XSIZE) + x]];
                    }
                }
            }

#if RETRO_USING_SDL2
            SDL_LockTexture(Engine.screenBuffer, NULL, &pixels, &pitch);
            memcpy(pixels, frameBuffer, pitch * SCREEN_YSIZE);
            SDL_UnlockTexture(Engine.screenBuffer);
#endif
            if (frameBuffer)
                delete[] frameBuffer;
            break;
        }
        case 2: // 32-bit
        {
            uint *frameBuffer = new uint[SCREEN_XSIZE * SCREEN_YSIZE];
            memset(frameBuffer, 0, (SCREEN_XSIZE * SCREEN_YSIZE) * sizeof(uint));
            if (fadeMode) {
                for (int y = 0; y < waterDrawPos; ++y) {
                    for (int x = 0; x < SCREEN_XSIZE; ++x) {
                        uint clr                            = palette32F[Engine.pixelBuffer[(y * SCREEN_XSIZE) + x]];
                        byte r                              = (clr >> 16) & 0xFF;
                        byte g                              = (clr >> 8) & 0xFF;
                        byte b                              = (clr >> 0) & 0xFF;
                        frameBuffer[(y * SCREEN_XSIZE) + x] = PACK_RGB888(r, g, b);
                    }
                }

                for (int y = waterDrawPos; y < SCREEN_YSIZE; ++y) {
                    for (int x = 0; x < SCREEN_XSIZE; ++x) {
                        uint clr                            = palette32WF[Engine.pixelBuffer[(y * SCREEN_XSIZE) + x]];
                        byte r                              = (clr >> 16) & 0xFF;
                        byte g                              = (clr >> 8) & 0xFF;
                        byte b                              = (clr >> 0) & 0xFF;
                        frameBuffer[(y * SCREEN_XSIZE) + x] = PACK_RGB888(r, g, b);
                    }
                }
            }
            else {
                for (int y = 0; y < waterDrawPos; ++y) {
                    for (int x = 0; x < SCREEN_XSIZE; ++x) {
                        uint clr                            = palette32[Engine.pixelBuffer[(y * SCREEN_XSIZE) + x]];
                        byte r                              = (clr >> 16) & 0xFF;
                        byte g                              = (clr >> 8) & 0xFF;
                        byte b                              = (clr >> 0) & 0xFF;
                        frameBuffer[(y * SCREEN_XSIZE) + x] = PACK_RGB888(r, g, b);
                    }
                }

                for (int y = waterDrawPos; y < SCREEN_YSIZE; ++y) {
                    for (int x = 0; x < SCREEN_XSIZE; ++x) {
                        uint clr                            = palette32W[Engine.pixelBuffer[(y * SCREEN_XSIZE) + x]];
                        byte r                              = (clr >> 16) & 0xFF;
                        byte g                              = (clr >> 8) & 0xFF;
                        byte b                              = (clr >> 0) & 0xFF;
                        frameBuffer[(y * SCREEN_XSIZE) + x] = PACK_RGB888(r, g, b);
                    }
                }
            }

#if RETRO_USING_SDL2
            SDL_LockTexture(Engine.screenBuffer, NULL, &pixels, &pitch);
            memcpy(pixels, frameBuffer, pitch * SCREEN_YSIZE);
            SDL_UnlockTexture(Engine.screenBuffer);
#endif
            if (frameBuffer)
                delete[] frameBuffer;
            break;
        }
    }

#if RETRO_USING_SDL2
    SDL_SetRenderTarget(Engine.renderer, NULL);
    SDL_RenderCopy(Engine.renderer, Engine.screenBuffer, NULL, NULL);

    SDL_RenderPresent(Engine.renderer);
#endif
}

void ReleaseRenderDevice()
{
    if (Engine.pixelBuffer)
        delete[] Engine.pixelBuffer;
#if RETRO_USING_SDL2
    SDL_DestroyTexture(Engine.screenBuffer);
    Engine.screenBuffer = NULL;
#endif

#if RETRO_USING_SDL1
    SDL_FreeSurface(Engine.screenBuffer);
#endif

#if RETRO_USING_SDL2
    SDL_DestroyRenderer(Engine.renderer);
    SDL_DestroyWindow(Engine.window);
#endif
}

void ClearScreen(byte index)
{
    byte *pixelBuffer = Engine.pixelBuffer;
    int cnt           = SCREEN_XSIZE * SCREEN_YSIZE;
    while (cnt--) {
        *pixelBuffer = index;
        ++pixelBuffer;
    }
}

void SetScreenSize(int width, int lineSize)
{
    SCREEN_XSIZE        = width;
    SCREEN_CENTERX      = (width / 2);
    SCREEN_SCROLL_LEFT  = SCREEN_CENTERX - 8;
    SCREEN_SCROLL_RIGHT = SCREEN_CENTERX + 8;
    OBJECT_BORDER_X2    = width + 0x80;
}

void DrawObjectList(int Layer)
{
    int size = drawListEntries[Layer].listSize;
    for (int i = 0; i < size; ++i) {
        objectLoop = drawListEntries[Layer].entityRefs[i];
        int type   = objectEntityList[objectLoop].type;

        if (type == OBJ_TYPE_PLAYER) {
            Player *player = &playerList[objectLoop];
            ProcessPlayerAnimationChange(player);
            if (player->visible) {
                DrawPlayer(player, &playerScriptList[player->type].animations[player->animation].frames[player->frame]);
            }
        }
        else if (type) {
            activePlayer = 0;
            if (scriptData[objectScriptList[type].subDraw.scriptCodePtr] > 0)
                ProcessScript(objectScriptList[type].subDraw.scriptCodePtr, objectScriptList[type].subDraw.jumpTablePtr, SUB_DRAW);
        }
    }
}
void DrawStageGFX()
{
    waterDrawPos = waterLevel - yScrollOffset;

    if (waterDrawPos < 0)
        waterDrawPos = 0;

    if (waterDrawPos > SCREEN_YSIZE)
        waterDrawPos = SCREEN_YSIZE;

    DrawObjectList(0);
    if (activeTileLayers[0] < LAYER_COUNT) {
        switch (stageLayouts[activeTileLayers[0]].type) {
            case LAYER_HSCROLL: DrawHLineScrollLayer(0); break;
            case LAYER_VSCROLL: DrawVLineScrollLayer(0); break;
            case LAYER_3DCLOUD: Draw3DCloudLayer(0); break;
            default: break;
        }
    }

    DrawObjectList(1);
    if (activeTileLayers[1] < LAYER_COUNT) {
        switch (stageLayouts[activeTileLayers[1]].type) {
            case LAYER_HSCROLL: DrawHLineScrollLayer(1); break;
            case LAYER_VSCROLL: DrawVLineScrollLayer(1); break;
            case LAYER_3DCLOUD: Draw3DCloudLayer(1); break;
            default: break;
        }
    }

    DrawObjectList(2);
    if (activeTileLayers[2] < LAYER_COUNT) {
        switch (stageLayouts[activeTileLayers[2]].type) {
            case LAYER_HSCROLL: DrawHLineScrollLayer(2); break;
            case LAYER_VSCROLL: DrawVLineScrollLayer(2); break;
            case LAYER_3DCLOUD: Draw3DCloudLayer(2); break;
            default: break;
        }
    }

    DrawObjectList(3);
    DrawObjectList(4);
    if (activeTileLayers[3] < LAYER_COUNT) {
        switch (stageLayouts[activeTileLayers[3]].type) {
            case LAYER_HSCROLL: DrawHLineScrollLayer(3); break;
            case LAYER_VSCROLL: DrawVLineScrollLayer(3); break;
            case LAYER_3DCLOUD: Draw3DCloudLayer(3); break;
            default: break;
        }
    }

    DrawObjectList(5);
    DrawObjectList(6);
}

void SetBlendTable(ushort alpha, byte type, byte a3, byte a4)
{
    switch (type) {
        case 0:
            for (int y = 0; y < 256; ++y) {
                for (int x = 0; x < 256; ++x) {
                    ushort mixR = (ushort)((0xFF - alpha) * palette8[y].r + alpha * palette8[x].r) >> 8;
                    ushort mixG = (ushort)((0xFF - alpha) * palette8[y].g + alpha * palette8[x].g) >> 8;
                    ushort mixB = (ushort)((0xFF - alpha) * palette8[y].b + alpha * palette8[x].b) >> 8;
                    int index   = 0;
                    int r       = -1;
                    int g       = -1;
                    int b       = -1;
                    for (int i = 0; i < 256; ++i) {
                        int mixR2 = abs(palette8[i].r - mixR);
                        int mixG2 = abs(palette8[i].g - mixG);
                        int mixB2 = abs(palette8[i].b - mixB);
                        if (mixR2 < r && mixG2 < g && mixB2 < b) {
                            r     = mixR2;
                            g     = mixG2;
                            b     = mixB2;
                            index = i;
                        }
                    }
                    blendLookupTable[(0x100 * y) + x] = index;
                }
            }
            break;
        case 1:
            for (int y = 0; y < 0x100; ++y) {
                for (int x = 0; x < 0x100; ++x) {
                    int v1                          = (byte)((palette8[y].b + palette8[y].g + palette8[y].r) / 3);
                    int v2                          = (byte)((palette8[x].b + palette8[x].g + palette8[x].r) / 3);
                    blendLookupTable[0x100 * y + x] = a4 + a3 * ((ushort)((0xFF - alpha) * v1 + alpha * v2) >> 8) / 0x100;
                }
            }
            break;
    }
}

void SetTintTable(short alpha, short a2, byte type, byte a4, byte a5, byte tableID)
{
    byte *tintTable = NULL;
    switch (tableID) {
        case 0: tintTable = tintTable0; break;
        case 1: tintTable = tintTable1; break;
        case 2: tintTable = tintTable2; break;
        case 3: tintTable = tintTable3; break;
        default: break;
    }

    switch (type) {
        case 0:
            for (int i = 0; i < 256; ++i) {
                byte val     = (byte)((palette8[i].b + palette8[i].g + palette8[i].r) / 3);
                tintTable[i] = a5 + a4 * ((ushort)((0xFF - alpha) * val + alpha * a2) >> 8) / 256;
            }
            break;
        case 1:
            for (int i = 0; i < 256; ++i) {
                tintTable[i] = a5 + a4 * ((ushort)((0xFF - alpha) * palette8[i].r + alpha * a2) >> 8) / 256;
            }
            break;
        case 2:
            for (int i = 0; i < 256; ++i) {
                tintTable[i] = a5 + a4 * ((ushort)((0xFF - alpha) * palette8[i].g + alpha * a2) >> 8) / 256;
            }
            break;
        case 3:
            for (int i = 0; i < 256; ++i) {
                tintTable[i] = a5 + a4 * ((ushort)((0xFF - alpha) * palette8[i].b + alpha * a2) >> 8) / 256;
            }
            break;
        default: break;
    }
}

void DrawHLineScrollLayer(int layerID)
{
    TileLayer *layer   = &stageLayouts[activeTileLayers[layerID]];
    int screenwidth16  = (SCREEN_XSIZE >> 4) - 1;
    int layerwidth     = layer->xsize;
    int layerheight    = layer->ysize;
    bool aboveMidPoint = layerID >= tLayerMidPoint;

    byte *lineScroll;
    int *deformationData;
    int *deformationDataW;

    int yscrollOffset = 0;
    if (activeTileLayers[layerID]) { // BG Layer
        int yScroll    = yScrollOffset * layer->parallaxFactor >> 6;
        int fullheight = layerheight << 7;
        layer->scrollPos += layer->scrollSpeed;
        if (layer->scrollPos > fullheight << 16)
            layer->scrollPos -= fullheight << 16;
        yscrollOffset    = (yScroll + (layer->scrollPos >> 16)) % fullheight;
        layerheight      = fullheight >> 7;
        lineScroll       = layer->lineScroll;
        deformationData  = &bgDeformationData2[(byte)(yscrollOffset + bgDeformationOffset)];
        deformationDataW = &bgDeformationData3[(byte)(yscrollOffset + waterDrawPos + bgDeformationOffsetW)];
    }
    else { // FG Layer
        lastXSize     = layer->xsize;
        yscrollOffset = yScrollOffset;
        lineScroll    = layer->lineScroll;
        for (int i = 0; i < PARALLAX_COUNT; ++i) hParallax.linePos[i] = xScrollOffset;
        deformationData  = &bgDeformationData0[(byte)(yscrollOffset + fgDeformationOffset)];
        deformationDataW = &bgDeformationData1[(byte)(yscrollOffset + waterDrawPos + fgDeformationOffsetW)];
    }

    if (layer->type == LAYER_HSCROLL) {
        if (lastXSize != layerwidth) {
            int fullLayerwidth = layerwidth << 7;
            for (int i = 0; i < hParallax.entryCount; ++i) {
                hParallax.linePos[i] = xScrollOffset * hParallax.parallaxFactor[i] >> 7;
                hParallax.scrollPos[i] += hParallax.scrollSpeed[i];
                if (hParallax.scrollPos[i] > fullLayerwidth << 16)
                    hParallax.scrollPos[i] -= fullLayerwidth << 16;
                if (hParallax.scrollPos[i] < 0)
                    hParallax.scrollPos[i] += fullLayerwidth << 16;
                hParallax.linePos[i] += hParallax.scrollPos[i] >> 16;
                hParallax.linePos[i] %= fullLayerwidth;
            }
        }
        lastXSize = layerwidth;
    }

    byte *pixelBufferPtr = Engine.pixelBuffer;
    int tileYPos         = yscrollOffset % (layerheight << 7);
    if (tileYPos < 0)
        tileYPos += layerheight << 7;
    byte *scrollIndex = &lineScroll[tileYPos];
    int tileY16       = tileYPos & 0xF;
    int chunkY        = tileYPos >> 7;
    int tileY         = (tileYPos & 0x7F) >> 4;

    // Draw Above Water (if applicable)
    int drawableLines[2] = { waterDrawPos, SCREEN_YSIZE - waterDrawPos };
    for (int i = 0; i < 2; ++i) {
        while (drawableLines[i]--) {
            int chunkX = hParallax.linePos[*scrollIndex];
            if (i == 0) {
                int deform = 0;
                if (hParallax.deform[*scrollIndex])
                    deform = *deformationData;

                // Fix for SS5 mobile bug
                if (StrComp(stageList[activeStageList][stageListPosition].name, "5") && activeStageList == STAGELIST_SPECIAL)
                    deform >>= 4;

                chunkX += deform;
                ++deformationData;
            }
            else {
                if (hParallax.deform[*scrollIndex])
                    chunkX += *deformationDataW;
                ++deformationDataW;
            }
            ++scrollIndex;
            int fullLayerwidth = layerwidth << 7;
            if (chunkX < 0)
                chunkX += fullLayerwidth;
            if (chunkX >= fullLayerwidth)
                chunkX -= fullLayerwidth;
            int chunkXPos         = chunkX >> 7;
            int tilePxXPos        = chunkX & 0xF;
            int tileXPxRemain     = TILE_SIZE - tilePxXPos;
            int chunk             = (layer->tiles[(chunkX >> 7) + (chunkY << 8)] << 6) + ((chunkX & 0x7F) >> 4) + 8 * tileY;
            int tileOffsetY       = TILE_SIZE * tileY16;
            int tileOffsetYFlipX  = TILE_SIZE * tileY16 + 0xF;
            int tileOffsetYFlipY  = TILE_SIZE * (0xF - tileY16);
            int tileOffsetYFlipXY = TILE_SIZE * (0xF - tileY16) + 0xF;
            int lineRemain        = SCREEN_XSIZE;

            byte *gfxDataPtr  = NULL;
            int tilePxLineCnt = 0;

            // Draw the first tile to the left
            if (tiles128x128.visualPlane[chunk] == (byte)aboveMidPoint) {
                tilePxLineCnt = TILE_SIZE - tilePxXPos;
                lineRemain -= tilePxLineCnt;
                switch (tiles128x128.direction[chunk]) {
                    case FLIP_NONE:
                        gfxDataPtr = &tilesetGFXData[tileOffsetY + tiles128x128.gfxDataPos[chunk] + tilePxXPos];
                        while (tilePxLineCnt--) {
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                        }
                        break;

                    case FLIP_X:
                        gfxDataPtr = &tilesetGFXData[tileOffsetYFlipX + tiles128x128.gfxDataPos[chunk] - tilePxXPos];
                        while (tilePxLineCnt--) {
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                        }
                        break;

                    case FLIP_Y:
                        gfxDataPtr = &tilesetGFXData[tileOffsetYFlipY + tiles128x128.gfxDataPos[chunk] + tilePxXPos];
                        while (tilePxLineCnt--) {
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                        }
                        break;

                    case FLIP_XY:
                        gfxDataPtr = &tilesetGFXData[tileOffsetYFlipXY + tiles128x128.gfxDataPos[chunk] - tilePxXPos];
                        while (tilePxLineCnt--) {
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                        }
                        break;
                    default: break;
                }
            }
            else {
                pixelBufferPtr += tileXPxRemain;
                lineRemain -= tileXPxRemain;
            }

            // Draw the bulk of the tiles
            int chunkTileX   = ((chunkX & 0x7F) >> 4) + 1;
            int tilesPerLine = screenwidth16;
            while (tilesPerLine--) {
                if (chunkTileX < 8) {
                    ++chunk;
                }
                else {
                    if (++chunkXPos == layerwidth)
                        chunkXPos = 0;
                    chunkTileX = 0;
                    chunk      = (layer->tiles[chunkXPos + (chunkY << 8)] << 6) + 8 * tileY;
                }
                lineRemain -= TILE_SIZE;

                // Loop Unrolling (faster but messier code)
                if (tiles128x128.visualPlane[chunk] == (byte)aboveMidPoint) {
                    switch (tiles128x128.direction[chunk]) {
                        case FLIP_NONE:
                            gfxDataPtr = &tilesetGFXData[tiles128x128.gfxDataPos[chunk] + tileOffsetY];
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            break;

                        case FLIP_X:
                            gfxDataPtr = &tilesetGFXData[tiles128x128.gfxDataPos[chunk] + tileOffsetYFlipX];
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            break;

                        case FLIP_Y:
                            gfxDataPtr = &tilesetGFXData[tiles128x128.gfxDataPos[chunk] + tileOffsetYFlipY];
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            ++gfxDataPtr;
                            break;

                        case FLIP_XY:
                            gfxDataPtr = &tilesetGFXData[tiles128x128.gfxDataPos[chunk] + tileOffsetYFlipXY];
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            ++pixelBufferPtr;
                            --gfxDataPtr;
                            break;
                    }
                }
                else {
                    pixelBufferPtr += TILE_SIZE;
                }
                ++chunkTileX;
            }

            // Draw any remaining tiles
            while (lineRemain > 0) {
                if (chunkTileX++ < 8) {
                    ++chunk;
                }
                else {
                    chunkTileX = 0;
                    if (++chunkXPos == layerwidth)
                        chunkXPos = 0;

                    chunk = (layer->tiles[chunkXPos + (chunkY << 8)] << 6) + 8 * tileY;
                }

                tilePxLineCnt = lineRemain >= TILE_SIZE ? TILE_SIZE : lineRemain;
                lineRemain -= tilePxLineCnt;
                if (tiles128x128.visualPlane[chunk] == (byte)aboveMidPoint) {
                    switch (tiles128x128.direction[chunk]) {
                        case FLIP_NONE:
                            gfxDataPtr = &tilesetGFXData[tiles128x128.gfxDataPos[chunk] + tileOffsetY];
                            while (tilePxLineCnt--) {
                                if (*gfxDataPtr > 0)
                                    *pixelBufferPtr = *gfxDataPtr;
                                ++pixelBufferPtr;
                                ++gfxDataPtr;
                            }
                            break;

                        case FLIP_X:
                            gfxDataPtr = &tilesetGFXData[tiles128x128.gfxDataPos[chunk] + tileOffsetYFlipX];
                            while (tilePxLineCnt--) {
                                if (*gfxDataPtr > 0)
                                    *pixelBufferPtr = *gfxDataPtr;
                                ++pixelBufferPtr;
                                --gfxDataPtr;
                            }
                            break;

                        case FLIP_Y:
                            gfxDataPtr = &tilesetGFXData[tiles128x128.gfxDataPos[chunk] + tileOffsetYFlipY];
                            while (tilePxLineCnt--) {
                                if (*gfxDataPtr > 0)
                                    *pixelBufferPtr = *gfxDataPtr;
                                ++pixelBufferPtr;
                                ++gfxDataPtr;
                            }
                            break;

                        case FLIP_XY:
                            gfxDataPtr = &tilesetGFXData[tiles128x128.gfxDataPos[chunk] + tileOffsetYFlipXY];
                            while (tilePxLineCnt--) {
                                if (*gfxDataPtr > 0)
                                    *pixelBufferPtr = *gfxDataPtr;
                                ++pixelBufferPtr;
                                --gfxDataPtr;
                            }
                            break;

                        default: break;
                    }
                }
                else {
                    pixelBufferPtr += tilePxLineCnt;
                }
            }

            if (++tileY16 >= TILE_SIZE) {
                tileY16 = 0;
                ++tileY;
            }

            if (tileY >= 8) {
                if (++chunkY == layerheight) {
                    chunkY = 0;
                    scrollIndex -= 0x80 * layerheight;
                }
                tileY = 0;
            }
        }
    }
}
void DrawVLineScrollLayer(int layerID)
{

    TileLayer *layer = &stageLayouts[activeTileLayers[layerID]];
    if (!layer->xsize || !layer->ysize)
        return;

    int layerwidth     = layer->xsize;
    int layerheight    = layer->ysize;
    bool aboveMidPoint = layerID >= tLayerMidPoint;

    byte *lineScroll;
    int *deformationData;

    int xscrollOffset = 0;
    if (activeTileLayers[layerID]) { // BG Layer
        int xScroll        = xScrollOffset * layer->parallaxFactor >> 6;
        int fullLayerwidth = layerwidth << 7;
        layer->scrollPos += layer->scrollSpeed;
        if (layer->scrollPos > fullLayerwidth << 16)
            layer->scrollPos -= fullLayerwidth << 16;
        xscrollOffset   = (xScroll + (layer->scrollPos >> 16)) % fullLayerwidth;
        layerwidth      = fullLayerwidth >> 7;
        lineScroll      = layer->lineScroll;
        deformationData = &bgDeformationData2[(byte)(xscrollOffset + bgDeformationOffset)];
    }
    else { // FG Layer
        lastYSize            = layer->ysize;
        xscrollOffset        = xScrollOffset;
        lineScroll           = layer->lineScroll;
        vParallax.linePos[0] = yScrollOffset;
        vParallax.deform[0]  = true;
        deformationData      = &bgDeformationData0[(byte)(xScrollOffset + fgDeformationOffset)];
    }

    if (layer->type == LAYER_VSCROLL) {
        if (lastYSize != layerheight) {
            int fullLayerheight = layerheight << 7;
            for (int i = 0; i < vParallax.entryCount; ++i) {
                vParallax.linePos[i] = yScrollOffset * vParallax.parallaxFactor[i] >> 7;

                vParallax.scrollPos[i] += vParallax.scrollPos[i] << 16;
                if (vParallax.scrollPos[i] > fullLayerheight << 16)
                    vParallax.scrollPos[i] -= fullLayerheight << 16;

                vParallax.linePos[i] += vParallax.scrollPos[i] >> 16;
                vParallax.linePos[i] %= fullLayerheight;
            }
            layerheight = fullLayerheight >> 7;
        }
        lastYSize = layerheight;
    }

    byte *pixelBufferPtr = Engine.pixelBuffer;
    int tileXPos         = xscrollOffset % (layerheight << 7);
    if (tileXPos < 0)
        tileXPos += layerheight << 7;
    byte *scrollIndex = &lineScroll[tileXPos];
    int chunkX        = tileXPos >> 7;
    int tileX16       = tileXPos & 0xF;
    int tileX         = (tileXPos & 0x7F) >> 4;

    // Draw Above Water (if applicable)
    int drawableLines = SCREEN_XSIZE;
    while (drawableLines--) {
        int chunkY = vParallax.linePos[*scrollIndex];
        if (vParallax.deform[*scrollIndex])
            chunkY += *deformationData;
        ++deformationData;
        ++scrollIndex;

        int fullLayerHeight = layerheight << 7;
        if (chunkY < 0)
            chunkY += fullLayerHeight;
        if (chunkY >= fullLayerHeight)
            chunkY -= fullLayerHeight;

        int chunkYPos         = chunkY >> 7;
        int tileY             = chunkY & 0xF;
        int tileYPxRemain     = TILE_SIZE - tileY;
        int chunk             = (layer->tiles[chunkX + (chunkY >> 7 << 8)] << 6) + tileX + 8 * ((chunkY & 0x7F) >> 4);
        int tileOffsetXFlipX  = 0xF - tileX16;
        int tileOffsetXFlipY  = tileX16 + SCREEN_YSIZE;
        int tileOffsetXFlipXY = 0xFF - tileX16;
        int lineRemain        = SCREEN_YSIZE;

        byte *gfxDataPtr  = NULL;
        int tilePxLineCnt = tileYPxRemain;

        // Draw the first tile to the left
        if (tiles128x128.visualPlane[chunk] == (byte)aboveMidPoint) {
            lineRemain -= tilePxLineCnt;
            switch (tiles128x128.direction[chunk]) {
                case FLIP_NONE:
                    gfxDataPtr = &tilesetGFXData[TILE_SIZE * tileY + tileX16 + tiles128x128.gfxDataPos[chunk]];
                    while (tilePxLineCnt--) {
                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;
                    }
                    break;

                case FLIP_X:
                    gfxDataPtr = &tilesetGFXData[TILE_SIZE * tileY + tileOffsetXFlipX + tiles128x128.gfxDataPos[chunk]];
                    while (tilePxLineCnt--) {
                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;
                    }
                    break;

                case FLIP_Y:
                    gfxDataPtr = &tilesetGFXData[tileOffsetXFlipY + tiles128x128.gfxDataPos[chunk] - TILE_SIZE * tileY];
                    while (tilePxLineCnt--) {
                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;
                    }
                    break;

                case FLIP_XY:
                    gfxDataPtr = &tilesetGFXData[tileOffsetXFlipXY + tiles128x128.gfxDataPos[chunk] - TILE_SIZE * tileY];
                    while (tilePxLineCnt--) {
                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;
                    }
                    break;

                default: break;
            }
        }
        else {
            pixelBufferPtr += SCREEN_XSIZE * tileYPxRemain;
            lineRemain -= tilePxLineCnt;
        }

        // Draw the bulk of the tiles
        int chunkTileY   = ((chunkY & 0x7F) >> 4) + 1;
        int tilesPerLine = (SCREEN_YSIZE >> 4) - 1;

        while (tilesPerLine--) {
            if (chunkTileY < 8) {
                chunk += 8;
            }
            else {
                if (++chunkYPos == layerheight)
                    chunkYPos = 0;

                chunkTileY = 0;
                chunk      = (layer->tiles[chunkX + (chunkYPos << 8)] << 6) + tileX;
            }
            lineRemain -= TILE_SIZE;

            // Loop Unrolling (faster but messier code)
            if (tiles128x128.visualPlane[chunk] == (byte)aboveMidPoint) {
                switch (tiles128x128.direction[chunk]) {
                    case FLIP_NONE:
                        gfxDataPtr = &tilesetGFXData[tiles128x128.gfxDataPos[chunk] + tileX16];
                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        break;

                    case FLIP_X:
                        gfxDataPtr = &tilesetGFXData[tiles128x128.gfxDataPos[chunk] + tileOffsetXFlipX];
                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr += TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        break;

                    case FLIP_Y:
                        gfxDataPtr = &tilesetGFXData[tiles128x128.gfxDataPos[chunk] + tileOffsetXFlipY];
                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        break;

                    case FLIP_XY:
                        gfxDataPtr = &tilesetGFXData[tiles128x128.gfxDataPos[chunk] + tileOffsetXFlipXY];
                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        gfxDataPtr -= TILE_SIZE;

                        if (*gfxDataPtr > 0)
                            *pixelBufferPtr = *gfxDataPtr;
                        pixelBufferPtr += SCREEN_XSIZE;
                        break;
                }
            }
            else {
                pixelBufferPtr += SCREEN_XSIZE * TILE_SIZE;
            }
            ++chunkTileY;
        }

        // Draw any remaining tiles
        while (lineRemain > 0) {
            if (chunkTileY < 8) {
                chunk += 8;
            }
            else {
                if (++chunkYPos == layerheight)
                    chunkYPos = 0;

                chunkTileY = 0;
                chunk      = (layer->tiles[chunkX + (chunkYPos << 8)] << 6) + tileX;
            }

            tilePxLineCnt = lineRemain >= TILE_SIZE ? TILE_SIZE : lineRemain;
            lineRemain -= tilePxLineCnt;

            if (tiles128x128.visualPlane[chunk] == (byte)aboveMidPoint) {
                switch (tiles128x128.direction[chunk]) {
                    case FLIP_NONE:
                        gfxDataPtr = &tilesetGFXData[tiles128x128.gfxDataPos[chunk] + tileX16];
                        while (tilePxLineCnt--) {
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            pixelBufferPtr += SCREEN_XSIZE;
                            gfxDataPtr += TILE_SIZE;
                        }
                        break;

                    case FLIP_X:
                        gfxDataPtr = &tilesetGFXData[tiles128x128.gfxDataPos[chunk] + tileOffsetXFlipX];
                        while (tilePxLineCnt--) {
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            pixelBufferPtr += SCREEN_XSIZE;
                            gfxDataPtr += TILE_SIZE;
                        }
                        break;

                    case FLIP_Y:
                        gfxDataPtr = &tilesetGFXData[tiles128x128.gfxDataPos[chunk] + tileOffsetXFlipY];
                        while (tilePxLineCnt--) {
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            pixelBufferPtr += SCREEN_XSIZE;
                            gfxDataPtr -= TILE_SIZE;
                        }
                        break;

                    case FLIP_XY:
                        gfxDataPtr = &tilesetGFXData[tiles128x128.gfxDataPos[chunk] + tileOffsetXFlipXY];
                        while (tilePxLineCnt--) {
                            if (*gfxDataPtr > 0)
                                *pixelBufferPtr = *gfxDataPtr;
                            pixelBufferPtr += SCREEN_XSIZE;
                            gfxDataPtr -= TILE_SIZE;
                        }
                        break;

                    default: break;
                }
            }
            else {
                pixelBufferPtr += SCREEN_XSIZE * tilePxLineCnt;
            }
            chunkTileY++;
        }

        if (++tileX16 >= TILE_SIZE) {
            tileX16 = 0;
            ++tileX;
        }

        if (tileX >= 8) {
            if (++chunkX == layerwidth) {
                chunkX = 0;
                scrollIndex -= 0x80 * layerwidth;
            }
            tileX = 0;
        }

        pixelBufferPtr -= (SCREEN_XSIZE * SCREEN_YSIZE) - 1;
    }
}
void Draw3DCloudLayer(int layerID) { TileLayer *layer = &stageLayouts[activeTileLayers[layerID]]; }

void DrawTintRectangle(int XPos, int YPos, int width, int height, byte tintID)
{
    if (width + XPos > SCREEN_XSIZE)
        width = SCREEN_XSIZE - XPos;
    if (XPos < 0) {
        width += XPos;
        XPos = 0;
    }

    if (height + YPos > SCREEN_YSIZE)
        height = SCREEN_YSIZE - YPos;
    if (YPos < 0) {
        height += YPos;
        YPos = 0;
    }
    if (width < 0 || height < 0)
        return;

    byte *tintTable = NULL;
    switch (tintID) {
        case 0: tintTable = tintTable0; break;
        case 1: tintTable = tintTable1; break;
        case 2: tintTable = tintTable2; break;
        case 3: tintTable = tintTable3; break;
        default: break;
    }

    int yOffset = SCREEN_XSIZE - width;
    for (byte *pixelBufferPtr = &Engine.pixelBuffer[XPos + SCREEN_XSIZE * YPos];; pixelBufferPtr += yOffset) {
        height--;
        if (height < 0)
            break;
        int w = width;
        while (w--) {
            *pixelBufferPtr = tintTable[*pixelBufferPtr];
            ++pixelBufferPtr;
        }
    }
}
void DrawScaledTintMask(int direction, int XPos, int YPos, int pivotX, int pivotY, int scaleX, int scaleY, int width, int height, int sprX, int sprY,
                        int tintID, int sheetID)
{
    int roundedYPos = 0;
    int roundedXPos = 0;
    int truescaleX  = 4 * scaleX;
    int truescaleY  = 4 * scaleY;
    int widthM1     = width - 1;
    int trueXPos    = XPos - (truescaleX * pivotX >> 11);
    width           = truescaleX * width >> 11;
    int trueYPos    = YPos - (truescaleY * pivotY >> 11);
    height          = truescaleY * height >> 11;
    int finalscaleX = (signed int)(float)((float)(2048.0 / (float)truescaleX) * 2048.0);
    int finalscaleY = (signed int)(float)((float)(2048.0 / (float)truescaleY) * 2048.0);
    if (width + trueXPos > SCREEN_XSIZE) {
        width = SCREEN_XSIZE - trueXPos;
    }

    if (direction) {
        if (trueXPos < 0) {
            widthM1 -= trueXPos * -finalscaleX >> 11;
            roundedXPos = (ushort)trueXPos * -(short)finalscaleX & 0x7FF;
            width += trueXPos;
            trueXPos = 0;
        }
    }
    else if (trueXPos < 0) {
        sprX += trueXPos * -finalscaleX >> 11;
        roundedXPos = (ushort)trueXPos * -(short)finalscaleX & 0x7FF;
        width += trueXPos;
        trueXPos = 0;
    }

    if (height + trueYPos > SCREEN_YSIZE) {
        height = SCREEN_YSIZE - trueYPos;
    }
    if (trueYPos < 0) {
        sprY += trueYPos * -finalscaleY >> 11;
        roundedYPos = (ushort)trueYPos * -(short)finalscaleY & 0x7FF;
        height += trueYPos;
        trueYPos = 0;
    }

    if (width <= 0 || height <= 0)
        return;

    byte *tintTable = NULL;
    switch (tintID) {
        case 0: tintTable = tintTable0; break;
        case 1: tintTable = tintTable1; break;
        case 2: tintTable = tintTable2; break;
        case 3: tintTable = tintTable3; break;
        default: break;
    }

    GFXSurface *surface = &gfxSurface[sheetID];
    int pitch           = SCREEN_XSIZE - width;
    int gfxwidth        = surface->width;
    // byte *lineBuffer       = &gfxLineBuffer[trueYPos];
    byte *gfxData        = &graphicData[sprX + surface->width * sprY + surface->dataPosition];
    byte *pixelBufferPtr = &Engine.pixelBuffer[trueXPos + SCREEN_XSIZE * trueYPos];
    if (direction == FLIP_X) {
        byte *gfxDataPtr = &gfxData[widthM1];
        int gfxPitch     = 0;
        while (height--) {
            int roundXPos = roundedXPos;
            int w         = width;
            while (w--) {
                if (*gfxDataPtr > 0)
                    *pixelBufferPtr = tintTable[*pixelBufferPtr];
                int offsetX = finalscaleX + roundXPos;
                gfxDataPtr -= offsetX >> 11;
                gfxPitch += offsetX >> 11;
                roundXPos = offsetX & 0x7FF;
                ++pixelBufferPtr;
            }
            pixelBufferPtr += pitch;
            int offsetY = finalscaleY + roundedYPos;
            gfxDataPtr += gfxPitch + (offsetY >> 11) * gfxwidth;
            roundedYPos = offsetY & 0x7FF;
            gfxPitch    = 0;
        }
    }
    else {
        int gfxPitch = 0;
        int h        = height;
        while (h--) {
            int roundXPos = roundedXPos;
            int w         = width;
            while (w--) {
                if (*gfxData > 0)
                    *pixelBufferPtr = tintTable[*pixelBufferPtr];
                int offsetX = finalscaleX + roundXPos;
                gfxData += offsetX >> 11;
                gfxPitch += offsetX >> 11;
                roundXPos = offsetX & 0x7FF;
                ++pixelBufferPtr;
            }
            pixelBufferPtr += pitch;
            int offsetY = finalscaleY + roundedYPos;
            gfxData += (offsetY >> 11) * gfxwidth - gfxPitch;
            roundedYPos = offsetY & 0x7FF;
            gfxPitch    = 0;
        }
    }
}

void DrawSprite(int XPos, int YPos, int width, int height, int sprX, int sprY, int sheetID)
{
    if (width + XPos > SCREEN_XSIZE)
        width = SCREEN_XSIZE - XPos;
    if (XPos < 0) {
        sprX -= XPos;
        width += XPos;
        XPos = 0;
    }
    if (height + YPos > SCREEN_YSIZE)
        height = SCREEN_YSIZE - YPos;
    if (YPos < 0) {
        sprY -= YPos;
        height += YPos;
        YPos = 0;
    }
    if (width <= 0 || height <= 0)
        return;

    GFXSurface *surface  = &gfxSurface[sheetID];
    int pitch            = SCREEN_XSIZE - width;
    int gfxPitch         = surface->width - width;
    byte *gfxDataPtr     = &graphicData[sprX + surface->width * sprY + surface->dataPosition];
    byte *pixelBufferPtr = &Engine.pixelBuffer[XPos + SCREEN_XSIZE * YPos];
    while (height--) {
        int w = width;
        while (w--) {
            if (*gfxDataPtr > 0)
                *pixelBufferPtr = *gfxDataPtr;
            ++gfxDataPtr;
            ++pixelBufferPtr;
        }
        pixelBufferPtr += pitch;
        gfxDataPtr += gfxPitch;
    }
}

void DrawSpriteNoKey(int XPos, int YPos, int width, int height, int sprX, int sprY, int sheetID)
{
    if (width + XPos > SCREEN_XSIZE)
        width = SCREEN_XSIZE - XPos;
    if (XPos < 0) {
        sprX -= XPos;
        width += XPos;
        XPos = 0;
    }
    if (height + YPos > SCREEN_YSIZE)
        height = SCREEN_YSIZE - YPos;
    if (YPos < 0) {
        sprY -= YPos;
        height += YPos;
        YPos = 0;
    }
    if (width <= 0 || height <= 0)
        return;

    GFXSurface *surface  = &gfxSurface[sheetID];
    int pitch            = SCREEN_XSIZE - width;
    int gfxPitch         = surface->width - width;
    byte *gfxDataPtr     = &graphicData[sprX + surface->width * sprY + surface->dataPosition];
    byte *pixelBufferPtr = &Engine.pixelBuffer[XPos + SCREEN_XSIZE * YPos];
    while (height--) {
        int w = width;
        while (w--) {
            *pixelBufferPtr = *gfxDataPtr;
            ++gfxDataPtr;
            ++pixelBufferPtr;
        }
        pixelBufferPtr += pitch;
        gfxDataPtr += gfxPitch;
    }
}

void DrawSpriteClipped(int XPos, int YPos, int width, int height, int sprX, int sprY, int sheetID, int clipY)
{
    if (width + XPos > SCREEN_XSIZE)
        width = SCREEN_XSIZE - XPos;
    if (XPos < 0) {
        sprX -= XPos;
        width += XPos;
        XPos = 0;
    }
    if (height + YPos > clipY)
        height = clipY - YPos;
    if (YPos < 0) {
        sprY -= YPos;
        height += YPos;
        YPos = 0;
    }
    if (width <= 0 || height <= 0)
        return;

    GFXSurface *surface  = &gfxSurface[sheetID];
    int pitch            = SCREEN_XSIZE - width;
    int gfxPitch         = surface->width - width;
    byte *gfxDataPtr     = &graphicData[sprX + surface->width * sprY + surface->dataPosition];
    byte *pixelBufferPtr = &Engine.pixelBuffer[XPos + SCREEN_XSIZE * YPos];
    while (height--) {
        int w = width;
        while (w--) {
            if (*gfxDataPtr > 0)
                *pixelBufferPtr = *gfxDataPtr;
            ++gfxDataPtr;
            ++pixelBufferPtr;
        }
        pixelBufferPtr += pitch;
        gfxDataPtr += gfxPitch;
    }
}

void DrawSpriteScaled(int direction, int XPos, int YPos, int pivotX, int pivotY, int scaleX, int scaleY, int width, int height, int sprX, int sprY,
                      int sheetID)
{
    int roundedYPos = 0;
    int roundedXPos = 0;
    int truescaleX  = 4 * scaleX;
    int truescaleY  = 4 * scaleY;
    int widthM1     = width - 1;
    int trueXPos    = XPos - (truescaleX * pivotX >> 11);
    width           = truescaleX * width >> 11;
    int trueYPos    = YPos - (truescaleY * pivotY >> 11);
    height          = truescaleY * height >> 11;
    int finalscaleX = (signed int)(float)((float)(2048.0 / (float)truescaleX) * 2048.0);
    int finalscaleY = (signed int)(float)((float)(2048.0 / (float)truescaleY) * 2048.0);
    if (width + trueXPos > SCREEN_XSIZE) {
        width = SCREEN_XSIZE - trueXPos;
    }

    if (direction) {
        if (trueXPos < 0) {
            widthM1 -= trueXPos * -finalscaleX >> 11;
            roundedXPos = (ushort)trueXPos * -(short)finalscaleX & 0x7FF;
            width += trueXPos;
            trueXPos = 0;
        }
    }
    else if (trueXPos < 0) {
        sprX += trueXPos * -finalscaleX >> 11;
        roundedXPos = (ushort)trueXPos * -(short)finalscaleX & 0x7FF;
        width += trueXPos;
        trueXPos = 0;
    }

    if (height + trueYPos > SCREEN_YSIZE) {
        height = SCREEN_YSIZE - trueYPos;
    }
    if (trueYPos < 0) {
        sprY += trueYPos * -finalscaleY >> 11;
        roundedYPos = (ushort)trueYPos * -(short)finalscaleY & 0x7FF;
        height += trueYPos;
        trueYPos = 0;
    }

    if (width <= 0 || height <= 0)
        return;

    GFXSurface *surface  = &gfxSurface[sheetID];
    int pitch            = SCREEN_XSIZE - width;
    int gfxwidth         = surface->width;
    byte *gfxData        = &graphicData[sprX + surface->width * sprY + surface->dataPosition];
    byte *pixelBufferPtr = &Engine.pixelBuffer[trueXPos + SCREEN_XSIZE * trueYPos];
    if (direction == FLIP_X) {
        byte *gfxDataPtr = &gfxData[widthM1];
        int gfxPitch     = 0;
        while (height--) {
            int roundXPos = roundedXPos;
            int w         = width;
            while (w--) {
                if (*gfxDataPtr > 0)
                    *pixelBufferPtr = *gfxDataPtr;
                int offsetX = finalscaleX + roundXPos;
                gfxDataPtr -= offsetX >> 11;
                gfxPitch += offsetX >> 11;
                roundXPos = offsetX & 0x7FF;
                ++pixelBufferPtr;
            }
            pixelBufferPtr += pitch;
            int offsetY = finalscaleY + roundedYPos;
            gfxDataPtr += gfxPitch + (offsetY >> 11) * gfxwidth;
            roundedYPos = offsetY & 0x7FF;
            gfxPitch    = 0;
        }
    }
    else {
        int gfxPitch = 0;
        int h        = height;
        while (h--) {
            int roundXPos = roundedXPos;
            int w         = width;
            while (w--) {
                if (*gfxData > 0)
                    *pixelBufferPtr = *gfxData;
                int offsetX = finalscaleX + roundXPos;
                gfxData += offsetX >> 11;
                gfxPitch += offsetX >> 11;
                roundXPos = offsetX & 0x7FF;
                ++pixelBufferPtr;
            }
            pixelBufferPtr += pitch;
            int offsetY = finalscaleY + roundedYPos;
            gfxData += (offsetY >> 11) * gfxwidth - gfxPitch;
            roundedYPos = offsetY & 0x7FF;
            gfxPitch    = 0;
        }
    }
}

void DrawSpriteRotated(int direction, int XPos, int YPos, int pivotX, int pivotY, int sprX, int sprY, int width, int height, int rotation,
                       int sheetID)
{
    int sprXPos    = (pivotX + sprX) << 9;
    int sprYPos    = (pivotY + sprY) << 9;
    int fullwidth  = width + sprX;
    int fullheight = height + sprY;
    int angle      = rotation & 0x1FF;
    if (angle < 0)
        angle += 0x200;
    if (angle)
        angle = 0x200 - angle;
    int sine   = sinVal512[angle];
    int cosine = cosVal512[angle];
    int XPositions[4];
    int YPositions[4];

    if (direction == FLIP_X) {
        XPositions[0] = XPos + ((sine * (-pivotY - 2) + cosine * (pivotX + 2)) >> 9);
        YPositions[0] = YPos + ((cosine * (-pivotY - 2) - sine * (pivotX + 2)) >> 9);
        XPositions[1] = XPos + ((sine * (-pivotY - 2) + cosine * (pivotX - width - 2)) >> 9);
        YPositions[1] = YPos + ((cosine * (-pivotY - 2) - sine * (pivotX - width - 2)) >> 9);
        XPositions[2] = XPos + ((sine * (height - pivotY + 2) + cosine * (pivotX + 2)) >> 9);
        YPositions[2] = YPos + ((cosine * (height - pivotY + 2) - sine * (pivotX + 2)) >> 9);
        int a         = pivotX - width - 2;
        int b         = height - pivotY + 2;
        XPositions[3] = XPos + ((sine * b + cosine * a) >> 9);
        YPositions[3] = YPos + ((cosine * b - sine * a) >> 9);
    }
    else {
        XPositions[0] = XPos + ((sine * (-pivotY - 2) + cosine * (-pivotX - 2)) >> 9);
        YPositions[0] = YPos + ((cosine * (-pivotY - 2) - sine * (-pivotX - 2)) >> 9);
        XPositions[1] = XPos + ((sine * (-pivotY - 2) + cosine * (width - pivotX + 2)) >> 9);
        YPositions[1] = YPos + ((cosine * (-pivotY - 2) - sine * (width - pivotX + 2)) >> 9);
        XPositions[2] = XPos + ((sine * (height - pivotY + 2) + cosine * (-pivotX - 2)) >> 9);
        YPositions[2] = YPos + ((cosine * (height - pivotY + 2) - sine * (-pivotX - 2)) >> 9);
        int a         = width - pivotX + 2;
        int b         = height - pivotY + 2;
        XPositions[3] = XPos + ((sine * b + cosine * a) >> 9);
        YPositions[3] = YPos + ((cosine * b - sine * a) >> 9);
    }

    int left = SCREEN_XSIZE;
    for (int i = 0; i < 4; ++i) {
        if (XPositions[i] < left)
            left = XPositions[i];
    }
    if (left < 0)
        left = 0;

    int right = 0;
    for (int i = 0; i < 4; ++i) {
        if (XPositions[i] > right)
            right = XPositions[i];
    }
    if (right > SCREEN_XSIZE)
        right = SCREEN_XSIZE;
    int maxX = right - left;

    int top = SCREEN_YSIZE;
    for (int i = 0; i < 4; ++i) {
        if (YPositions[i] < top)
            top = YPositions[i];
    }
    if (top < 0)
        top = 0;

    int bottom = 0;
    for (int i = 0; i < 4; ++i) {
        if (YPositions[i] > bottom)
            bottom = YPositions[i];
    }
    if (bottom > SCREEN_YSIZE)
        bottom = SCREEN_YSIZE;
    int maxY = bottom - top;

    if (maxX <= 0 || maxY <= 0)
        return;

    GFXSurface *surface  = &gfxSurface[sheetID];
    int pitch            = SCREEN_XSIZE - maxX;
    byte *pixelBufferPtr = &Engine.pixelBuffer[left + SCREEN_XSIZE * top];
    int startX           = left - XPos;
    int startY           = top - YPos;
    int shiftPivot       = (sprX << 9) - 1;
    fullwidth <<= 9;
    int shiftheight = (sprY << 9) - 1;
    fullheight <<= 9;
    byte *gfxData = &graphicData[surface->dataPosition];
    if (cosine < 0 || sine < 0)
        sprYPos += sine + cosine;

    if (direction == FLIP_X) {
        int drawX = sprXPos - (cosine * startX - sine * startY) - 0x100;
        int drawY = cosine * startY + sprYPos + sine * startX;
        while (maxY--) {
            int finalX = drawX;
            int finalY = drawY;
            int w      = maxX;
            while (w--) {
                if (finalX > shiftPivot && finalX < fullwidth && finalY > shiftheight && finalY < fullheight) {
                    byte index = gfxData[((finalY >> 9) * surface->width) + (finalX >> 9)];
                    if (index > 0)
                        *pixelBufferPtr = index;
                }
                ++pixelBufferPtr;
                finalX -= cosine;
                finalY += sine;
            }
            drawX += sine;
            drawY += cosine;
            pixelBufferPtr += pitch;
        }
    }
    else {
        int drawX = sprXPos + cosine * startX - sine * startY;
        int drawY = cosine * startY + sprYPos + sine * startX;
        while (maxY--) {
            int finalX = drawX;
            int finalY = drawY;
            int w      = maxX;
            while (w--) {
                if (finalX > shiftPivot && finalX < fullwidth && finalY > shiftheight && finalY < fullheight) {
                    byte index = gfxData[((finalY >> 9) * surface->width) + (finalX >> 9)];
                    if (index > 0)
                        *pixelBufferPtr = index;
                }
                ++pixelBufferPtr;
                finalX += cosine;
                finalY += sine;
            }
            drawX -= sine;
            drawY += cosine;
            pixelBufferPtr += pitch;
        }
    }
}

void DrawBlendedSprite(int XPos, int YPos, int width, int height, int sprX, int sprY, int sheetID)
{
    if (width + XPos > SCREEN_XSIZE)
        width = SCREEN_XSIZE - XPos;
    if (XPos < 0) {
        sprX -= XPos;
        width += XPos;
        XPos = 0;
    }
    if (height + YPos > SCREEN_YSIZE)
        height = SCREEN_YSIZE - YPos;
    if (YPos < 0) {
        sprY -= YPos;
        height += YPos;
        YPos = 0;
    }
    if (width <= 0 || height <= 0)
        return;

    GFXSurface *surface  = &gfxSurface[sheetID];
    int pitch            = SCREEN_XSIZE - width;
    int gfxPitch         = surface->width - width;
    byte *gfxData        = &graphicData[sprX + surface->width * sprY + surface->dataPosition];
    byte *pixelBufferPtr = &Engine.pixelBuffer[XPos + SCREEN_XSIZE * YPos];
    while (height--) {
        int w = width;
        while (w--) {
            if (*gfxData > 0)
                *pixelBufferPtr = blendLookupTable[(0x100 * *pixelBufferPtr) + *gfxData];
            ++gfxData;
            ++pixelBufferPtr;
        }
        pixelBufferPtr += pitch;
        gfxData += gfxPitch;
    }
}

void DrawTextMenuEntry(void *menu, int rowID, int XPos, int YPos, int textHighlight)
{
    TextMenu *tMenu = (TextMenu *)menu;
    int id          = tMenu->entryStart[rowID];
    for (int i = 0; i < tMenu->entrySize[rowID]; ++i) {
        if (tMenu->textData[id] > 0)
            DrawSprite(XPos + 8 * i, YPos, 8, 8, textHighlight, 8 * tMenu->textData[id] - 8, textMenuSurfaceNo);
        id++;
    }
}
void DrawStageTextEntry(void *menu, int rowID, int XPos, int YPos, int textHighlight)
{
    TextMenu *tMenu = (TextMenu *)menu;
    int id          = tMenu->entryStart[rowID];
    for (int i = 0; i < tMenu->entrySize[rowID]; ++i) {
        if (tMenu->textData[id] > 0) {
            if (i == tMenu->entrySize[rowID] - 1)
                DrawSprite(XPos + 8 * i, YPos, 8, 8, 0, 8 * tMenu->textData[id] - 8, textMenuSurfaceNo);
            else
                DrawSprite(XPos + 8 * i, YPos, 8, 8, textHighlight, 8 * tMenu->textData[id] - 8, textMenuSurfaceNo);
        }
        id++;
    }
}
void DrawTextMenu(void *menu, int XPos, int YPos)
{
    TextMenu *tMenu = (TextMenu *)menu;
    if (tMenu->selectionCount == 3) {
        tMenu->selection2 = -1;
        for (int i = 0; i < tMenu->selection1 + 1; ++i) {
            if (tMenu->entryHighlight[i] == 1) {
                tMenu->selection2 = i;
            }
        }
    }
    switch (tMenu->alignment) {
        case 0:
            for (int i = 0; i < tMenu->rowCount; ++i) {
                switch (tMenu->selectionCount) {
                    case 1:
                        if (i == tMenu->selection1)
                            DrawTextMenuEntry(tMenu, i, XPos, YPos, 8);
                        else
                            DrawTextMenuEntry(tMenu, i, XPos, YPos, 0);
                        break;
                    case 2:
                        if (i == tMenu->selection1 || i == tMenu->selection2)
                            DrawTextMenuEntry(tMenu, i, XPos, YPos, 8);
                        else
                            DrawTextMenuEntry(tMenu, i, XPos, YPos, 0);
                        break;
                    case 3:
                        if (i == tMenu->selection1)
                            DrawTextMenuEntry(tMenu, i, XPos, YPos, 8);
                        else
                            DrawTextMenuEntry(tMenu, i, XPos, YPos, 0);
                        if (i == tMenu->selection2 && i != tMenu->selection1)
                            DrawStageTextEntry(tMenu, i, XPos, YPos, 8);
                        break;
                }
                YPos += 8;
            }
            return;
        case 1:
            for (int i = 0; i < tMenu->rowCount; ++i) {
                int textX = XPos - (tMenu->entrySize[i] << 3);
                switch (tMenu->selectionCount) {
                    case 1:
                        if (i == tMenu->selection1)
                            DrawTextMenuEntry(tMenu, i, textX, YPos, 8);
                        else
                            DrawTextMenuEntry(tMenu, i, textX, YPos, 0);
                        break;
                    case 2:
                        if (i == tMenu->selection1 || i == tMenu->selection2)
                            DrawTextMenuEntry(tMenu, i, textX, YPos, 8);
                        else
                            DrawTextMenuEntry(tMenu, i, textX, YPos, 0);
                        break;
                    case 3:
                        if (i == tMenu->selection1)
                            DrawTextMenuEntry(tMenu, i, textX, YPos, 8);
                        else
                            DrawTextMenuEntry(tMenu, i, textX, YPos, 0);
                        if (i == tMenu->selection2 && i != tMenu->selection1)
                            DrawStageTextEntry(tMenu, i, textX, YPos, 8);
                        break;
                }
                YPos += 8;
            }
            return;
        case 2:
            for (int i = 0; i < tMenu->rowCount; ++i) {
                int textX = XPos - (tMenu->entrySize[i] >> 1 << 3);
                switch (tMenu->selectionCount) {
                    case 1:
                        if (i == tMenu->selection1)
                            DrawTextMenuEntry(tMenu, i, textX, YPos, 8);
                        else
                            DrawTextMenuEntry(tMenu, i, textX, YPos, 0);
                        break;
                    case 2:
                        if (i == tMenu->selection1 || i == tMenu->selection2)
                            DrawTextMenuEntry(tMenu, i, textX, YPos, 8);
                        else
                            DrawTextMenuEntry(tMenu, i, textX, YPos, 0);
                        break;
                    case 3:
                        if (i == tMenu->selection1)
                            DrawTextMenuEntry(tMenu, i, textX, YPos, 8);
                        else
                            DrawTextMenuEntry(tMenu, i, textX, YPos, 0);

                        if (i == tMenu->selection2 && i != tMenu->selection1)
                            DrawStageTextEntry(tMenu, i, textX, YPos, 8);
                        break;
                }
                YPos += 8;
            }
            return;
        default: return;
    }
}
