#ifndef USERDATA_H
#define USERDATA_H

#define GLOBALVAR_COUNT (0x100)

#define ACHIEVEMENT_MAX (0x40)
#define LEADERBOARD_MAX (0x80)

#define MOD_MAX (0x100)

#define SAVEDATA_MAX (0x2000)

#include <string>
#include <map>
#include <unordered_map>

#if RETRO_USE_MOD_LOADER
struct ModInfo {
    std::string name;
    std::string desc;
    std::string author;
    std::string version;
    std::map<std::string, std::string> fileMap;
    std::string folder;
    bool active;
};
#endif

extern int globalVariablesCount;
extern int globalVariables[GLOBALVAR_COUNT];
extern char globalVariableNames[GLOBALVAR_COUNT][0x20];

#if RETRO_USE_MOD_LOADER
extern char gamePath[0x100];
extern char modsPath[0x100];
extern std::vector<ModInfo> modList;
#endif

inline int GetGlobalVariableByName(const char *name)
{
    for (int v = 0; v < globalVariablesCount; ++v) {
        if (StrComp(name, globalVariableNames[v]))
            return globalVariables[v];
    }
    return 0;
}

inline void SetGlobalVariableByName(const char *name, int value)
{
    for (int v = 0; v < globalVariablesCount; ++v) {
        if (StrComp(name, globalVariableNames[v])) {
            globalVariables[v] = value;
            break;
        }
    }
}

void InitUserdata();
void writeSettings();

#if RETRO_USE_MOD_LOADER
void initMods();
bool loadMod(ModInfo *info, std::string modsPath, std::string folder, bool active);
void saveMods();
#endif

#endif //!USERDATA_H
