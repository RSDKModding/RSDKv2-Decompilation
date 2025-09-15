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
#if !RETRO_USE_ORIGINAL_CODE
    unsigned long long targetFreq = SDL_GetPerformanceFrequency() / Engine.refreshRate;
    unsigned long long curTicks   = 0;
    unsigned long long prevTicks  = 0;
#endif

    while (GameRunning) {
#if !RETRO_USE_ORIGINAL_CODE
        if (!vsync) {
            curTicks = SDL_GetPerformanceCounter();
            if (curTicks < prevTicks + targetFreq)
                continue;
            prevTicks = curTicks;
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

#if RETRO_USE_MOD_LOADER
const tinyxml2::XMLElement *FirstXMLChildElement(tinyxml2::XMLDocument *doc, const tinyxml2::XMLElement *elementPtr, const char *name)
{
    if (doc) {
        if (!elementPtr)
            return doc->FirstChildElement(name);
        else
            return elementPtr->FirstChildElement(name);
    }
    return NULL;
}

const tinyxml2::XMLElement *NextXMLSiblingElement(tinyxml2::XMLDocument *doc, const tinyxml2::XMLElement *elementPtr, const char *name)
{
    if (doc) {
        if (!elementPtr)
            return doc->NextSiblingElement(name);
        else
            return elementPtr->NextSiblingElement(name);
    }
    return NULL;
}

const tinyxml2::XMLAttribute *FindXMLAttribute(const tinyxml2::XMLElement *elementPtr, const char *name) { return elementPtr->FindAttribute(name); }
const char *GetXMLAttributeName(const tinyxml2::XMLAttribute *attributePtr) { return attributePtr->Name(); }
int GetXMLAttributeValueInt(const tinyxml2::XMLAttribute *attributePtr) { return attributePtr->IntValue(); }
bool GetXMLAttributeValueBool(const tinyxml2::XMLAttribute *attributePtr) { return attributePtr->BoolValue(); }
const char *GetXMLAttributeValueString(const tinyxml2::XMLAttribute *attributePtr) { return attributePtr->Value(); }

void RetroEngine::LoadXMLWindowText()
{
    FileInfo info;
    for (int m = 0; m < (int)modList.size(); ++m) {
        if (!modList[m].active)
            continue;

        SetActiveMod(m);
        if (LoadFile("Data/Game/Game.xml", &info)) {
            tinyxml2::XMLDocument *doc = new tinyxml2::XMLDocument;

            char *xmlData = new char[info.fileSize + 1];
            FileRead(xmlData, info.fileSize);
            xmlData[info.fileSize] = 0;

            bool success = doc->Parse(xmlData) == tinyxml2::XML_SUCCESS;

            if (success) {
                const tinyxml2::XMLElement *gameElement  = FirstXMLChildElement(doc, nullptr, "game");
                const tinyxml2::XMLElement *titleElement = FirstXMLChildElement(doc, gameElement, "title");
                if (titleElement) {
                    const tinyxml2::XMLAttribute *nameAttr = FindXMLAttribute(titleElement, "name");
                    if (nameAttr)
                        StrCopy(GameWindowText, GetXMLAttributeValueString(nameAttr));
                }
            }

            delete[] xmlData;
            delete doc;

            CloseFile();
        }
    }
    SetActiveMod(-1);
}
void RetroEngine::LoadXMLVariables()
{
    FileInfo info;
    for (int m = 0; m < (int)modList.size(); ++m) {
        if (!modList[m].active)
            continue;

        SetActiveMod(m);
        if (LoadFile("Data/Game/Game.xml", &info)) {
            tinyxml2::XMLDocument *doc = new tinyxml2::XMLDocument;

            char *xmlData = new char[info.fileSize + 1];
            FileRead(xmlData, info.fileSize);
            xmlData[info.fileSize] = 0;

            bool success = doc->Parse(xmlData) == tinyxml2::XML_SUCCESS;

            if (success) {
                const tinyxml2::XMLElement *gameElement      = FirstXMLChildElement(doc, nullptr, "game");
                const tinyxml2::XMLElement *variablesElement = FirstXMLChildElement(doc, gameElement, "variables");
                if (variablesElement) {
                    const tinyxml2::XMLElement *varElement = FirstXMLChildElement(doc, variablesElement, "variable");
                    if (varElement) {
                        do {
                            const tinyxml2::XMLAttribute *nameAttr = FindXMLAttribute(varElement, "name");
                            const char *varName                    = "UnknownVariable";
                            if (nameAttr)
                                varName = GetXMLAttributeValueString(nameAttr);

                            const tinyxml2::XMLAttribute *valAttr = FindXMLAttribute(varElement, "value");
                            int varValue                          = 0;
                            if (valAttr)
                                varValue = GetXMLAttributeValueInt(valAttr);

                            StrCopy(GlobalVariableNames[NO_GLOBALVARIABLES], varName);
                            GlobalVariables[NO_GLOBALVARIABLES] = varValue;
                            NO_GLOBALVARIABLES++;

                        } while ((varElement = NextXMLSiblingElement(doc, varElement, "variable")));
                    }
                }
            }

            delete[] xmlData;
            delete doc;

            CloseFile();
        }
    }
    SetActiveMod(-1);
}
void RetroEngine::LoadXMLPalettes()
{
    FileInfo info;
    for (int m = 0; m < (int)modList.size(); ++m) {
        if (!modList[m].active)
            continue;

        SetActiveMod(m);
        if (LoadFile("Data/Game/Game.xml", &info)) {
            tinyxml2::XMLDocument *doc = new tinyxml2::XMLDocument;

            char *xmlData = new char[info.fileSize + 1];
            FileRead(xmlData, info.fileSize);
            xmlData[info.fileSize] = 0;

            bool success = doc->Parse(xmlData) == tinyxml2::XML_SUCCESS;

            if (success) {
                const tinyxml2::XMLElement *gameElement    = FirstXMLChildElement(doc, nullptr, "game");
                const tinyxml2::XMLElement *paletteElement = FirstXMLChildElement(doc, gameElement, "palette");
                if (paletteElement) {
                    for (const tinyxml2::XMLElement *clrElement = paletteElement->FirstChildElement("color"); clrElement;
                         clrElement                             = clrElement->NextSiblingElement("color")) {
                        const tinyxml2::XMLAttribute *indAttr = clrElement->FindAttribute("index");
                        int clrInd                            = 0;
                        if (indAttr)
                            clrInd = indAttr->IntValue();

                        const tinyxml2::XMLAttribute *rAttr = clrElement->FindAttribute("r");
                        int clrR                            = 0;
                        if (rAttr)
                            clrR = rAttr->IntValue();

                        const tinyxml2::XMLAttribute *gAttr = clrElement->FindAttribute("g");
                        int clrG                            = 0;
                        if (gAttr)
                            clrG = gAttr->IntValue();

                        const tinyxml2::XMLAttribute *bAttr = clrElement->FindAttribute("b");
                        int clrB                            = 0;
                        if (bAttr)
                            clrB = bAttr->IntValue();

                        SetPaletteEntry(clrInd, clrR, clrG, clrB);
                    }

                    for (const tinyxml2::XMLElement *clrsElement = paletteElement->FirstChildElement("colors"); clrsElement;
                         clrsElement                             = clrsElement->NextSiblingElement("colors")) {
                        const tinyxml2::XMLAttribute *indAttr = clrsElement->FindAttribute("start");
                        int index                             = 0;
                        if (indAttr)
                            index = indAttr->IntValue();

                        std::string text = clrsElement->GetText();
                        // working: AABBFF #FFaaFF (12, 32, 34) (145 53 234)
                        std::regex search(R"((?:#?([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{2}))|(?:\((\d+),?\s*(\d+),?\s*(\d+)\)))",
                                          std::regex_constants::icase | std::regex_constants::ECMAScript);
                        std::smatch match;
                        while (std::regex_search(text, match, search)) {
                            int r, g, b;
                            int base, start;
                            if (match[1].matched) {
                                // we have hex
                                base  = 16;
                                start = 1;
                            }
                            else {
                                // triplet
                                base  = 10;
                                start = 4;
                            }

                            r = std::stoi(match[start + 0].str(), nullptr, base);
                            g = std::stoi(match[start + 1].str(), nullptr, base);
                            b = std::stoi(match[start + 2].str(), nullptr, base);

                            SetPaletteEntry(index++, r, g, b);
                            text = match.suffix();
                        }
                    }
                }
            }

            delete[] xmlData;
            delete doc;

            CloseFile();
        }
    }
    SetActiveMod(-1);
}
void RetroEngine::LoadXMLObjects()
{
    FileInfo info;
    modObjCount = 0;

    for (int m = 0; m < (int)modList.size(); ++m) {
        if (!modList[m].active)
            continue;

        SetActiveMod(m);
        if (LoadFile("Data/Game/Game.xml", &info)) {
            tinyxml2::XMLDocument *doc = new tinyxml2::XMLDocument;

            char *xmlData = new char[info.fileSize + 1];
            FileRead(xmlData, info.fileSize);
            xmlData[info.fileSize] = 0;

            bool success = doc->Parse(xmlData) == tinyxml2::XML_SUCCESS;

            if (success) {
                const tinyxml2::XMLElement *gameElement    = FirstXMLChildElement(doc, nullptr, "game");
                const tinyxml2::XMLElement *objectsElement = FirstXMLChildElement(doc, gameElement, "objects");
                if (objectsElement) {
                    const tinyxml2::XMLElement *objElement = FirstXMLChildElement(doc, objectsElement, "object");
                    if (objElement) {
                        do {
                            const tinyxml2::XMLAttribute *nameAttr = FindXMLAttribute(objElement, "name");
                            const char *objName                    = "UnknownObject";
                            if (nameAttr)
                                objName = GetXMLAttributeValueString(nameAttr);

                            const tinyxml2::XMLAttribute *scrAttr = FindXMLAttribute(objElement, "script");
                            const char *objScript                 = "UnknownObject.txt";
                            if (scrAttr)
                                objScript = GetXMLAttributeValueString(scrAttr);

                            byte flags = 0;

                            // forces the object to be loaded, this means the object doesn't have to be and *SHOULD NOT* be in the stage object list
                            // if it is, it'll cause issues!!!!
                            const tinyxml2::XMLAttribute *loadAttr = FindXMLAttribute(objElement, "forceLoad");
                            int objForceLoad                       = false;
                            if (loadAttr)
                                objForceLoad = GetXMLAttributeValueBool(loadAttr);

                            flags |= (objForceLoad & 1);

                            StrCopy(modTypeNames[modObjCount], objName);
                            StrCopy(modScriptPaths[modObjCount], objScript);
                            modScriptFlags[modObjCount] = flags;
                            modObjCount++;

                        } while ((objElement = NextXMLSiblingElement(doc, objElement, "object")));
                    }
                }
            }
            else {
                PrintLog("Failed to parse game.xml File!");
            }

            delete[] xmlData;
            delete doc;

            CloseFile();
        }
    }
    SetActiveMod(-1);
}
void RetroEngine::LoadXMLSoundFX()
{
    FileInfo info;
    FileInfo infoStore;
    for (int m = 0; m < (int)modList.size(); ++m) {
        if (!modList[m].active)
            continue;

        SetActiveMod(m);
        if (LoadFile("Data/Game/Game.xml", &info)) {
            tinyxml2::XMLDocument *doc = new tinyxml2::XMLDocument;

            char *xmlData = new char[info.fileSize + 1];
            FileRead(xmlData, info.fileSize);
            xmlData[info.fileSize] = 0;

            bool success = doc->Parse(xmlData) == tinyxml2::XML_SUCCESS;

            if (success) {
                const tinyxml2::XMLElement *gameElement   = FirstXMLChildElement(doc, nullptr, "game");
                const tinyxml2::XMLElement *soundsElement = FirstXMLChildElement(doc, gameElement, "sounds");
                if (soundsElement) {
                    const tinyxml2::XMLElement *sfxElement = FirstXMLChildElement(doc, soundsElement, "soundfx");
                    if (sfxElement) {
                        do {
                            const tinyxml2::XMLAttribute *pathAttr = FindXMLAttribute(sfxElement, "path");
                            const char *sfxPath                    = "UnknownSFX.wav";
                            if (pathAttr)
                                sfxPath = GetXMLAttributeValueString(pathAttr);

                            SetSfxName(sfxPath, NoGlobalSFX, true);

                            GetFileInfo(&infoStore);
                            CloseFile();
                            LoadSfx((char *)sfxPath, NoGlobalSFX);
                            SetFileInfo(&infoStore);
                            NoGlobalSFX++;

                        } while ((sfxElement = NextXMLSiblingElement(doc, sfxElement, "soundfx")));
                    }
                }
            }
            else {
                PrintLog("Failed to parse game.xml File!");
            }

            delete[] xmlData;
            delete doc;

            CloseFile();
        }
    }
    SetActiveMod(-1);
}
void RetroEngine::LoadXMLPlayers(TextMenu *menu)
{
    FileInfo info;

    for (int m = 0; m < (int)modList.size(); ++m) {
        if (!modList[m].active)
            continue;

        SetActiveMod(m);
        if (LoadFile("Data/Game/Game.xml", &info)) {
            tinyxml2::XMLDocument *doc = new tinyxml2::XMLDocument;

            char *xmlData = new char[info.fileSize + 1];
            FileRead(xmlData, info.fileSize);
            xmlData[info.fileSize] = 0;

            bool success = doc->Parse(xmlData) == tinyxml2::XML_SUCCESS;

            if (success) {
                const tinyxml2::XMLElement *gameElement   = FirstXMLChildElement(doc, nullptr, "game");
                const tinyxml2::XMLElement *playerElement = FirstXMLChildElement(doc, gameElement, "players");
                if (playerElement) {
                    const tinyxml2::XMLElement *plrElement = FirstXMLChildElement(doc, playerElement, "player");
                    if (plrElement) {
                        do {
                            const tinyxml2::XMLAttribute *animAttr = FindXMLAttribute(plrElement, "animation");
                            const char *plrAnim                    = "UnknownPlayer.ani";
                            if (animAttr)
                                plrAnim = GetXMLAttributeValueString(animAttr);

                            const tinyxml2::XMLAttribute *scrAttr = FindXMLAttribute(plrElement, "script");
                            const char *plrScript                 = "UnknownPlayer.txt";
                            if (scrAttr)
                                plrScript = GetXMLAttributeValueString(scrAttr);

                            const tinyxml2::XMLAttribute *nameAttr = FindXMLAttribute(plrElement, "name");
                            const char *plrName                    = "UnknownPlayer";
                            if (nameAttr)
                                plrName = GetXMLAttributeValueString(nameAttr);

                            modPlayerAnimations.resize(playerCount + 1);
                            modPlayerScripts.resize(playerCount + 1);
                            playerNames.resize(playerCount + 1);

                            if (menu)
                                AddTextMenuEntry(menu, plrName);
                            else
                                playerNames[playerCount] = plrName;
                            modPlayerAnimations[playerCount] = plrAnim;
                            modPlayerScripts[playerCount++] = plrScript;
                            PrintLog("player stats: anim=\"%s\" scr=\"%s\" name=\"%s\"", playerNames[playerCount - 1].data(), modPlayerScripts[playerCount - 1].data(), modPlayerAnimations[playerCount - 1].data());

                        } while ((plrElement = NextXMLSiblingElement(doc, plrElement, "player")));
                    }
                }
            }
            else {
                PrintLog("Failed to parse game.xml File!");
            }

            delete[] xmlData;
            delete doc;

            CloseFile();
        }
    }
    SetActiveMod(-1);
}
void RetroEngine::LoadXMLStages(TextMenu *menu, int listNo)
{
    FileInfo info;
    for (int m = 0; m < (int)modList.size(); ++m) {
        if (!modList[m].active)
            continue;

        SetActiveMod(m);
        if (LoadFile("Data/Game/Game.xml", &info)) {
            tinyxml2::XMLDocument *doc = new tinyxml2::XMLDocument;

            char *xmlData = new char[info.fileSize + 1];
            FileRead(xmlData, info.fileSize);
            xmlData[info.fileSize] = 0;

            bool success = doc->Parse(xmlData) == tinyxml2::XML_SUCCESS;

            if (success) {
                const char *elementNames[] = { "presentationStages", "regularStages", "bonusStages", "specialStages" };

                const tinyxml2::XMLElement *gameElement = FirstXMLChildElement(doc, nullptr, "game");
                for (int l = 0; l < STAGELIST_MAX; ++l) {
                    const tinyxml2::XMLElement *listElement = FirstXMLChildElement(doc, gameElement, elementNames[l]);
                    if (listElement) {
                        const tinyxml2::XMLElement *stgElement = FirstXMLChildElement(doc, listElement, "stage");
                        if (stgElement) {
                            do {
                                const tinyxml2::XMLAttribute *nameAttr = FindXMLAttribute(stgElement, "name");
                                const char *stgName                    = "UnknownStage";
                                if (nameAttr)
                                    stgName = GetXMLAttributeValueString(nameAttr);

                                const tinyxml2::XMLAttribute *folderAttr = FindXMLAttribute(stgElement, "folder");
                                const char *stgFolder                    = "UnknownStageFolder";
                                if (folderAttr)
                                    stgFolder = GetXMLAttributeValueString(folderAttr);

                                const tinyxml2::XMLAttribute *idAttr = FindXMLAttribute(stgElement, "id");
                                const char *stgID                    = "UnknownStageID";
                                if (idAttr)
                                    stgID = GetXMLAttributeValueString(idAttr);

                                const tinyxml2::XMLAttribute *highlightAttr = FindXMLAttribute(stgElement, "highlight");
                                bool stgHighlighted                         = false;
                                if (highlightAttr)
                                    stgHighlighted = GetXMLAttributeValueBool(highlightAttr);

                                if (menu) {
                                    if (listNo == 3 || listNo == 4) {
                                        if ((listNo == 4 && l == 2) || (listNo == 3 && l == 3)) {
                                            AddTextMenuEntry(menu, stgName);
                                            menu->entryHighlight[menu->rowCount - 1] = stgHighlighted;
                                        }
                                    }
                                    else if (listNo == l + 1) {
                                        AddTextMenuEntry(menu, stgName);
                                        menu->entryHighlight[menu->rowCount - 1] = stgHighlighted;
                                    }
                                }
                                else {

                                    StrCopy(stageList[l][stageListCount[l]].name, stgName);
                                    StrCopy(stageList[l][stageListCount[l]].folder, stgFolder);
                                    StrCopy(stageList[l][stageListCount[l]].id, stgID);
                                    stageList[l][stageListCount[l]].highlighted = stgHighlighted;

                                    stageListCount[l]++;
                                }

                            } while ((stgElement = NextXMLSiblingElement(doc, stgElement, "stage")));
                        }
                    }
                }
            }
            else {
                PrintLog("Failed to parse game.xml File!");
            }

            delete[] xmlData;
            delete doc;

            CloseFile();
        }
    }
    SetActiveMod(-1);
}
#endif

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
#if RETRO_USE_MOD_LOADER
        FileRead(&playerCount, 1);
        playerNames.resize(playerCount);
#else
        byte playerCount = 0;
        FileRead(&playerCount, 1);
#endif
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

#if RETRO_USE_MOD_LOADER
        LoadXMLWindowText();
        LoadXMLVariables();
        LoadXMLPalettes();
        LoadXMLObjects();
        LoadXMLPlayers(NULL);
        LoadXMLStages(NULL, 0);
#endif

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