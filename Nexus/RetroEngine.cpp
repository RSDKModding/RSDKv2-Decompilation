#include "RetroEngine.hpp"
#if RETRO_PLATFORM == RETRO_UWP
#include <winrt/base.h>
#include <winrt/Windows.Storage.h>
#endif

bool usingCWD        = false;
bool engineDebugMode = false;

RetroEngine Engine = RetroEngine();

inline int getLowerRate(int intendRate, int targetRate) {
    int result   = 0;
    int valStore = 0;

    result = targetRate;
    if (intendRate) {
        do {
            valStore   = result % intendRate;
            result     = intendRate;
            intendRate = valStore;
        } while (valStore);
    }
    return result;
}

bool processEvents() {
#if RETRO_USING_SDL1 || RETRO_USING_SDL2
    while (SDL_PollEvent(&Engine.sdlEvents)) {
        // Main Events
        switch (Engine.sdlEvents.type) {
#if RETRO_USING_SDL2
            case SDL_WINDOWEVENT:
                switch (Engine.sdlEvents.window.event) {
                    case SDL_WINDOWEVENT_MAXIMIZED: {
                        SDL_RestoreWindow(Engine.window);
                        SDL_SetWindowFullscreen(Engine.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                        Engine.isFullScreen = true;
                        break;
                    }
                    case SDL_WINDOWEVENT_CLOSE: Engine.GameMode = ENGINE_EXITGAME; return false;
                }
                break;
            case SDL_CONTROLLERDEVICEADDED: controllerInit(Engine.sdlEvents.cdevice.which); break;
            case SDL_CONTROLLERDEVICEREMOVED: controllerClose(Engine.sdlEvents.cdevice.which); break;
            case SDL_APP_TERMINATING: Engine.GameMode = ENGINE_EXITGAME; break;
#endif
            case SDL_KEYDOWN:
                switch (Engine.sdlEvents.key.keysym.sym) {
                    default: break;

                    case SDLK_ESCAPE:
                        if (Engine.devMenu) {
#if RETRO_USE_MOD_LOADER
                            // hacky patch because people can escape
                            if (Engine.GameMode == ENGINE_SYSMENU && StageMode == DEVMENU_MODMENU) {
                                RefreshEngine();
                            }
#endif

                            Engine.GameMode = ENGINE_INITSYSMENU;
                        } else {
                            Engine.GameMode = ENGINE_EXITGAME;
                            return false;
                        }
                        break;

                    case SDLK_F1:
                        if (Engine.devMenu) {
                            ActiveStageList   = 0;
                            StageListPosition = 0;
                            StageMode         = STAGEMODE_LOAD;
                            Engine.GameMode   = ENGINE_MAINGAME;
                        } else {
                            Engine.GameRunning = false;
                        }
                        break;

                    case SDLK_F2:
                        if (Engine.devMenu) {
                            StageListPosition--;
                            while (StageListPosition < 0) {
                                ActiveStageList--;

                                if (ActiveStageList < 0)
                                    ActiveStageList = 3;
                                StageListPosition = stageListCount[ActiveStageList] - 1;
                            }
                            StageMode       = STAGEMODE_LOAD;
                            Engine.GameMode = ENGINE_MAINGAME;
                        }
                        break;

                    case SDLK_F3:
                        if (Engine.devMenu) {
                            StageListPosition++;
                            while (StageListPosition >= stageListCount[ActiveStageList]) {
                                ActiveStageList++;

                                StageListPosition = 0;

                                if (ActiveStageList >= 4)
                                    ActiveStageList = 0;
                            }
                            StageMode       = STAGEMODE_LOAD;
                            Engine.GameMode = ENGINE_MAINGAME;
                        }
                        break;

                    case SDLK_F4:
                        Engine.isFullScreen ^= 1;
                        if (Engine.isFullScreen) {
#if RETRO_USING_SDL1
                            Engine.windowSurface = SDL_SetVideoMode(SCREEN_XSIZE * Engine.windowScale, SCREEN_YSIZE * Engine.windowScale, 16,
                                                                    SDL_SWSURFACE | SDL_FULLSCREEN);
                            SDL_ShowCursor(SDL_FALSE);
#endif

#if RETRO_USING_SDL2
                            SDL_RestoreWindow(Engine.window);
                            SDL_SetWindowFullscreen(Engine.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
#endif
                        } else {
#if RETRO_USING_SDL1
                            Engine.windowSurface =
                                SDL_SetVideoMode(SCREEN_XSIZE * Engine.windowScale, SCREEN_YSIZE * Engine.windowScale, 16, SDL_SWSURFACE);
                            SDL_ShowCursor(SDL_TRUE);
#endif

#if RETRO_USING_SDL2
                            SDL_SetWindowFullscreen(Engine.window, 0);
                            SDL_SetWindowSize(Engine.window, SCREEN_XSIZE * Engine.windowScale, SCREEN_YSIZE * Engine.windowScale);
                            SDL_SetWindowPosition(Engine.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
                            SDL_RestoreWindow(Engine.window);
#endif
                        }
                        break;

                    case SDLK_F5:
                        if (Engine.devMenu) {
                            CurrentStageFolder[0] = 0; // reload all assets & scripts
                            StageMode             = STAGEMODE_LOAD;
                        }
                        break;

#if RETRO_PLATFORM == RETRO_OSX
                    case SDLK_TAB:
                        if (Engine.devMenu)
                            Engine.gameSpeed = Engine.fastForwardSpeed;
                        break;

                    case SDLK_F6:
                        if (Engine.masterPaused)
                            Engine.frameStep = true;
                        break;

                    case SDLK_F7:
                        if (Engine.devMenu)
                            Engine.masterPaused ^= 1;
                        break;
#else
                    case SDLK_BACKSPACE:
                        if (Engine.devMenu)
                            Engine.gameSpeed = Engine.fastForwardSpeed;
                        break;

                    case SDLK_F11:
                    case SDLK_INSERT:
                        if (Engine.masterPaused)
                            Engine.frameStep = true;
                        break;

                    case SDLK_F12:
                    case SDLK_PAUSE:
                        if (Engine.devMenu)
                            Engine.masterPaused ^= 1;
                        break;
#endif
                }

#if RETRO_USING_SDL1
                keyState[Engine.sdlEvents.key.keysym.sym] = 1;
#endif
                break;
            case SDL_KEYUP:
                switch (Engine.sdlEvents.key.keysym.sym) {
                    default: break;
#if RETRO_PLATFORM == RETRO_OSX
                    case SDLK_TAB: Engine.gameSpeed = 1; break;
#else
                    case SDLK_BACKSPACE: Engine.gameSpeed = 1; break;
#endif
                }
#if RETRO_USING_SDL1
                keyState[Engine.sdlEvents.key.keysym.sym] = 0;
#endif
                break;
            case SDL_QUIT: Engine.GameMode = ENGINE_EXITGAME; return false;
        }
    }
#endif
    return true;
}

void RetroEngine::Init() {
    CalculateTrigAngles();
#if !RETRO_USE_ORIGINAL_CODE
    InitUserdata();
#endif
#if RETRO_USE_MOD_LOADER
    InitMods();
#endif
    char dest[0x200];
#if RETRO_PLATFORM == RETRO_UWP
    static char resourcePath[256] = { 0 };

    if (strlen(resourcePath) == 0) {
        auto folder = winrt::Windows::Storage::ApplicationData::Current().LocalFolder();
        auto path   = to_string(folder.Path());

        std::copy(path.begin(), path.end(), resourcePath);
    }

    strcat(dest, resourcePath);
    strcat(dest, "\\");
    strcat(dest, Engine.dataFile);
#else
    StrCopy(dest, BASE_PATH);
    StrAdd(dest, Engine.dataFile);
#endif
    CheckBinFile(dest);

    GameMode = ENGINE_EXITGAME;
    GameRunning  = false;
    if (LoadGameConfig("Data/Game/GameConfig.bin")) {
        if (InitRenderDevice()) {
            if (InitSoundDevice()) {
                InitSystemMenu();
                ClearScriptData();
                initialised = true;
                GameRunning     = true;
            }
        }
    }

    // Calculate Skip frame
    int lower        = getLowerRate(targetRefreshRate, refreshRate);
    renderFrameIndex = targetRefreshRate / lower;
    skipFrameIndex   = refreshRate / lower;
}

void RetroEngine::Run() {
    unsigned long long targetFreq = SDL_GetPerformanceFrequency() / Engine.refreshRate;
    unsigned long long curTicks   = 0;

    while (GameRunning) {
#if !RETRO_USE_ORIGINAL_CODE
        if (!vsync) {
            if (SDL_GetPerformanceCounter() < curTicks + targetFreq)
                continue;
            curTicks = SDL_GetPerformanceCounter();
        }
#endif
        GameRunning = processEvents();

        for (int s = 0; s < gameSpeed; ++s) {
            ReadInputDevice();

            if (!masterPaused || frameStep) {
                switch (GameMode) {
                    case ENGINE_SYSMENU:
                        ProcessSystemMenu();
                        FlipScreen();
                        break;
                    case ENGINE_MAINGAME: 
                        ProcessStage(); 
                        break;
                    case ENGINE_INITSYSMENU:
                        LoadGameConfig("Data/Game/GameConfig.bin");
                        InitSystemMenu();
                        ResetCurrentStageFolder();
                        break;
                    case ENGINE_EXITGAME: 
                        GameRunning = false; 
                        break;
                    default: break;
                }
            }
        }

        frameStep = false;
    }

    ReleaseSoundDevice();
    ReleaseRenderDevice();
    WriteSettings();
#if RETRO_USE_MOD_LOADER
    SaveMods();
#endif

#if RETRO_USING_SDL1 || RETRO_USING_SDL2
    SDL_Quit();
#endif
}

bool RetroEngine::LoadGameConfig(const char *filePath) {
    FileInfo info;
    byte fileBuffer  = 0;
    byte fileBuffer2 = 0;
    char data[0x40];
    char strBuf[0x80];
    byte strLen = 0;

    if (LoadFile(filePath, &info)) {
        FileRead(&fileBuffer, 1);
        FileRead(GameWindowText, fileBuffer);
        GameWindowText[fileBuffer] = 0;

        FileRead(&fileBuffer, 1);
        FileRead(&data, fileBuffer); // Load 'Data'
        data[fileBuffer] = 0;

        FileRead(&fileBuffer, 1);
        FileRead(GameDescriptionText, fileBuffer);
        GameDescriptionText[fileBuffer] = 0;

        // Read Script Paths
        byte scriptCount = 0;
        FileRead(&scriptCount, 1);
        for (byte s = 0; s < scriptCount; ++s) {
            FileRead(&fileBuffer, 1);
            for (byte i = 0; i < fileBuffer; ++i) FileRead(&fileBuffer2, 1);
        }

        byte varCount = 0;
        FileRead(&varCount, 1);
        NO_GLOBALVARIABLES = varCount;
        for (byte v = 0; v < varCount; ++v) {
            // Read Variable Name
            FileRead(&fileBuffer, 1);
            FileRead(&GlobalVariableNames[v], fileBuffer);
            GlobalVariableNames[v][fileBuffer] = 0;

            // Read Variable Value
            FileRead(&fileBuffer2, 1);
            GlobalVariables[v] = fileBuffer2 << 24;
            FileRead(&fileBuffer2, 1);
            GlobalVariables[v] += fileBuffer2 << 16;
            FileRead(&fileBuffer2, 1);
            GlobalVariables[v] += fileBuffer2 << 8;
            FileRead(&fileBuffer2, 1);
            GlobalVariables[v] += fileBuffer2;
        }

        // Read SFX
        byte sfxCount = 0;
        FileRead(&sfxCount, 1);
        for (byte s = 0; s < sfxCount; ++s) {
            FileRead(&fileBuffer, 1);
            for (byte i = 0; i < fileBuffer; ++i) FileRead(&fileBuffer2, 1);
        }

        // Read Player Names
        byte playerCount = 0;
        FileRead(&playerCount, 1);
        for (byte p = 0; p < playerCount; ++p) {
            // Player Anim
            FileRead(&strLen, 1);
            FileRead(&strBuf, strLen);
            strBuf[strLen] = 0;
            // Player Script
            FileRead(&strLen, 1);
            FileRead(&strBuf, strLen);
            strBuf[strLen] = 0;
            // Player Name
            FileRead(&strLen, 1);
            FileRead(&strBuf, strLen);
            strBuf[strLen] = 0;
        }

        for (int c = 0; c < 4; ++c) {
            // Special Stages are stored as cat 2 in file, but cat 3 in game :(
            int cat = c;
            if (c == 2)
                cat = 3;
            else if (c == 3)
                cat = 2;
            FileRead(&fileBuffer, 1);
            stageListCount[cat] = fileBuffer;
            for (int s = 0; s < stageListCount[cat]; ++s) {
                // Read Stage Folder
                FileRead(&fileBuffer, 1);
                FileRead(&stageList[cat][s].folder, fileBuffer);
                stageList[cat][s].folder[fileBuffer] = 0;

                // Read Stage ID
                FileRead(&fileBuffer, 1);
                FileRead(&stageList[cat][s].id, fileBuffer);
                stageList[cat][s].id[fileBuffer] = 0;

                // Read Stage Name
                FileRead(&fileBuffer, 1);
                FileRead(&stageList[cat][s].name, fileBuffer);
                stageList[cat][s].name[fileBuffer] = 0;

                // Read Stage Mode
                FileRead(&fileBuffer, 1);
                stageList[cat][s].highlighted = fileBuffer;
            }
        }

#if !RETRO_USE_ORIGINAL_CODE
        if (strlen(Engine.startSceneFolder) && strlen(Engine.startSceneID)) {
            SceneInfo *scene = &stageList[STAGELIST_BONUS][0xFE]; // slot 0xFF is used for "none" startStage
            strcpy(scene->name, "_RSDK_SCENE");
            strcpy(scene->folder, Engine.startSceneFolder);
            strcpy(scene->id, Engine.startSceneID);
            startList_Game  = STAGELIST_BONUS;
            startStage_Game = 0xFE;
        }
#endif

        CloseFile();
        return true;
    }

    return false;
}