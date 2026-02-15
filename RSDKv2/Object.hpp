#ifndef OBJECT_H
#define OBJECT_H

#define ENTITY_COUNT (0x4A0)
#define TEMPENTITY_START (ENTITY_COUNT - 0x80)
#define OBJECT_COUNT (0x100)

struct Entity {
    int XPos;
    int YPos;
    int values[8];
    int scale;
    int rotation;
    byte type;
    byte propertyValue;
    byte state;
    byte priority;
    byte drawOrder;
    byte direction;
    byte inkEffect;
    byte frame;
};

enum ObjectTypes {
    OBJ_TYPE_BLANKOBJECT = 0, //0 is always blank obj
    OBJ_TYPE_PLAYER = 1, //1 is always player obj
};

enum ObjectPriority {
    PRIORITY_BOUNDS,
    PRIORITY_ALWAYS,
};

extern int ObjectLoop;
extern int curObjectType;
extern Entity ObjectEntityList[ENTITY_COUNT];

#if RETRO_USE_MOD_LOADER
extern char typeNames[OBJECT_COUNT][0x40];
#endif

extern int OBJECT_BORDER_X1;
extern int OBJECT_BORDER_X2;
extern const int OBJECT_BORDER_Y1;
extern const int OBJECT_BORDER_Y2;

void ProcessStartupScripts();
void ProcessObjects();

#if RETRO_USE_MOD_LOADER
void SetObjectTypeName(const char *objectName, int objectID);
#endif

#endif // !OBJECT_H
