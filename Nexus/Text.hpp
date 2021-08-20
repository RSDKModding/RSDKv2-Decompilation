#ifndef TEXTSYSTEM_H
#define TEXTSYSTEM_H

#define TEXTDATA_COUNT  (0x1000)
#define TEXTENTRY_COUNT (0x80)
#define TEXTMENU_COUNT (0x2)

#define FONTCHAR_COUNT (0x400)

enum TextInfoTypes { TEXTINFO_TEXTDATA = 0, TEXTINFO_TEXTSIZE = 1, TEXTINFO_ROWCOUNT = 2 };

struct TextMenu {
    char textData[TEXTDATA_COUNT];
    int entryStart[TEXTENTRY_COUNT];
    int entrySize[TEXTENTRY_COUNT];
    byte entryHighlight[TEXTENTRY_COUNT];
    int textDataPos;
    byte rowCount;
    byte alignment;
    byte selectionCount;
    sbyte selection1;
    sbyte selection2;
};

extern TextMenu gameMenu[TEXTMENU_COUNT];
extern int textMenuSurfaceNo;

void LoadConfigListText(TextMenu *menu, int listNo);

void SetupTextMenu(TextMenu *menu, int rowCount);
void AddTextMenuEntry(TextMenu *menu, const char *text);
void EditTextMenuEntry(TextMenu *menu, const char *text, int rowID);

#endif // !TEXTSYSTEM_H
