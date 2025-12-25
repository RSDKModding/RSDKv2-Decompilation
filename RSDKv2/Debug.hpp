#ifndef DEBUG_H
#define DEBUG_H

void PrintLog(const char *msg, ...);

enum DevMenuMenus {
    DEVMENU_MAIN,
    DEVMENU_PLAYERSEL,
    DEVMENU_STAGELISTSEL,
    DEVMENU_STAGESEL,
    DEVMENU_MODMENU,
};

void InitSystemMenu();
void ProcessSystemMenu();

#endif //!DEBUG_H
