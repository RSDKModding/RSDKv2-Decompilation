#ifndef RETROENGINE_H
#define RETROENGINE_H

// Disables POSIX use c++ name blah blah stuff
#pragma warning(disable : 4996)

// Setting this to true removes (almost) ALL changes from the original code, the trade off is that a playable game cannot be built, it is advised to
// be set to true only for preservation purposes
#define RETRO_USE_ORIGINAL_CODE (0)
#define RETRO_USE_MOD_LOADER    (0)

#if !RETRO_USE_ORIGINAL_CODE
#undef RETRO_USE_MOD_LOADER
#define RETRO_USE_MOD_LOADER (1)
#endif //  !RETRO_USE_ORIGINAL_CODE

// ================
// STANDARD LIBS
// ================
#include <stdio.h>
#include <string.h>
#include <cmath>

// ================
// STANDARD TYPES
// ================
typedef unsigned char byte;
typedef signed char sbyte;
typedef unsigned short ushort;
typedef unsigned int uint;

// Platforms (RSDKv3 only defines these 7, but feel free to add your own custom platform define for easier platform code changes)
#define RETRO_WIN      (0)
#define RETRO_OSX      (1)
#define RETRO_XBOX_360 (2)
#define RETRO_PS3      (3)
#define RETRO_iOS      (4)
#define RETRO_ANDROID  (5)
#define RETRO_WP7      (6)
// Custom Platforms start here
#define RETRO_UWP  (7)

// Platform types (Game manages platform-specific code such as HUD position using this rather than the above)
#define RETRO_STANDARD (0)
#define RETRO_MOBILE   (1)

// use this macro (RETRO_PLATFORM) to define platform specific code blocks and etc to run the engine
#if defined _WIN32
#if defined WINAPI_FAMILY
#if WINAPI_FAMILY != WINAPI_FAMILY_APP
#define RETRO_PLATFORM (RETRO_WIN)
#else
#include <WinRTIncludes.hpp>
#define RETRO_PLATFORM (RETRO_UWP)
#endif
#else
#define RETRO_PLATFORM (RETRO_WIN)
#endif
#elif defined __APPLE__
#define RETRO_USING_MOUSE
#define RETRO_USING_TOUCH
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
#define RETRO_PLATFORM (RETRO_iOS)
#elif TARGET_OS_IPHONE
#define RETRO_PLATFORM (RETRO_iOS)
#elif TARGET_OS_MAC
#define RETRO_PLATFORM (RETRO_OSX)
#else
#error "Unknown Apple platform"
#endif
#else
#define RETRO_PLATFORM (RETRO_WIN) // Default
#endif

#if RETRO_PLATFORM == RETRO_UWP
#define BASE_PATH            ""
#define DEFAULT_SCREEN_XSIZE 320
#define DEFAULT_FULLSCREEN   false
#else
#define BASE_PATH            ""
#define RETRO_USING_MOUSE
#define RETRO_USING_TOUCH
#define DEFAULT_SCREEN_XSIZE 320
#define DEFAULT_FULLSCREEN   false
#endif

#if RETRO_PLATFORM == RETRO_WIN || RETRO_PLATFORM == RETRO_OSX || RETRO_PLATFORM == RETRO_iOS                        \
    || RETRO_PLATFORM == RETRO_UWP
#define RETRO_USING_SDL1 (0)
#define RETRO_USING_SDL2 (1)
#else // Since its an else & not an elif these platforms probably aren't supported yet
#define RETRO_USING_SDL1 (0)
#define RETRO_USING_SDL2 (0)
#endif

enum RetroLanguages { RETRO_EN = 0, RETRO_FR = 1, RETRO_IT = 2, RETRO_DE = 3, RETRO_ES = 4, RETRO_JP = 5 };

enum RetroStates {
    ENGINE_SYSMENU         = 0,
    ENGINE_MAINGAME        = 1,
    ENGINE_INITSYSMENU     = 2,
    ENGINE_EXITGAME        = 3,
};

enum RetroBytecodeFormat {
    BYTECODE_MOBILE = 0,
    BYTECODE_PC     = 1,
};

// General Defines
#define SCREEN_YSIZE   (240)
#define SCREEN_CENTERY (SCREEN_YSIZE / 2)

#if RETRO_PLATFORM == RETRO_WIN || RETRO_PLATFORM == RETRO_UWP
#if RETRO_USING_SDL2
#include <SDL.h>
#elif RETRO_USING_SDL1
#include <SDL.h>
#endif
#include <vorbis/vorbisfile.h>
#elif RETRO_PLATFORM == RETRO_OSX
#include <SDL2/SDL.h>
#include <Vorbis/vorbisfile.h>

#include "cocoaHelpers.hpp"
#elif RETRO_PLATFORM == RETRO_iOS
#include <SDL2/SDL.h>
#include <vorbis/vorbisfile.h>

#include "cocoaHelpers.hpp"
#endif

extern bool usingCWD;
extern bool engineDebugMode;

// Utils
#include "Ini.hpp"
#include "Math.hpp"
#include "String.hpp"
#include "Reader.hpp"
#include "Animation.hpp"
#include "Audio.hpp"
#include "Input.hpp"
#include "Object.hpp"
#include "Player.hpp"
#include "Palette.hpp"
#include "Drawing.hpp"
#include "Collision.hpp"
#include "Scene.hpp"
#include "Script.hpp"
#include "Sprite.hpp"
#include "Text.hpp"
#include "Video.hpp"
#include "Userdata.hpp"
#include "Debug.hpp"

class RetroEngine
{
public:
    RetroEngine()
    {
    }

    bool usingBinFile      = false;
    bool usingDataFileStore = false;
    bool forceFolder        = false;

    char dataFile[0x80];

    bool initialised = false;
    bool running     = false;

    int gameMode     = 1;
    byte colourMode = 1; //16-bit

    int frameSkipSetting = 0;
    int frameSkipTimer   = 0;

    // Ported from RSDKv5
    int startList_Game  = -1;
    int startStage_Game = -1;

    bool consoleEnabled  = false;
    bool devMenu         = false;
    int startList        = -1;
    int startStage       = -1;
    int gameSpeed        = 1;
    int fastForwardSpeed = 8;
    bool masterPaused    = false;
    bool frameStep       = false;

    char startSceneFolder[0x10];
    char startSceneID[0x10];

    void Init();
    void Run();

    bool LoadGameConfig(const char *Filepath);

    char gameWindowText[0x40];
    char gameDescriptionText[0x100];

    byte *pixelBuffer   = nullptr;

    bool isFullScreen = false;

    bool startFullScreen  = false; // if should start as fullscreen
    bool borderless       = false;
    bool vsync            = false;
    bool enhancedScaling  = true; // enable enhanced scaling
    int windowScale       = 2;
    int refreshRate       = 60; // user-picked screen update rate
    int screenRefreshRate = 60; // hardware screen update rate
    int targetRefreshRate = 60; // game logic update rate

    uint frameCount      = 0; // frames since scene load
    int renderFrameIndex = 0;
    int skipFrameIndex   = 0;

    int windowXSize; // width of window/screen in the previous frame
    int windowYSize; // height of window/screen in the previous frame

#if RETRO_USING_SDL2
    SDL_Window *window          = nullptr;
    SDL_Renderer *renderer      = nullptr;
    SDL_Texture *screenBuffer   = nullptr;
    SDL_Texture *screenBuffer2x = nullptr;
    SDL_Texture *videoBuffer    = nullptr;

    SDL_Event sdlEvents;
#endif

#if RETRO_USING_SDL1
    SDL_Surface *windowSurface = nullptr;

    SDL_Surface *screenBuffer   = nullptr;
    SDL_Surface *screenBuffer2x = nullptr;
    SDL_Surface *videoBuffer    = nullptr;

    SDL_Event sdlEvents;
#endif
};

extern RetroEngine Engine;
#endif // !RETROENGINE_H
