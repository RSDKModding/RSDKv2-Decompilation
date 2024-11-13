#include "RetroEngine.hpp"

int ObjectLoop    = 0;
int curObjectType = 0;
Entity ObjectEntityList[ENTITY_COUNT];

int OBJECT_BORDER_X1       = 0x80;
int OBJECT_BORDER_X2       = 0;
const int OBJECT_BORDER_Y1 = 0x100;
const int OBJECT_BORDER_Y2 = SCREEN_YSIZE + 0x100;

void ProcessStartupObjects() {
    ScriptFramesNo = 0;
    ClearAnimationData();
    PlayerNo                   = 0;
    ScriptEng.arrayPosition[2] = TEMPENTITY_START;
    Entity *entity             = &ObjectEntityList[TEMPENTITY_START];
    for (int i = 0; i < OBJECT_COUNT; ++i) {
        ObjectScript *scriptInfo  = &ObjectScriptList[i];
        ObjectLoop                = TEMPENTITY_START;
        curObjectType             = i;
        int frameStart            = ScriptFramesNo;
        scriptInfo->frameStartPtr = &ScriptFrames[ScriptFramesNo];
        scriptInfo->spriteSheetID = 0;
        entity->type              = i;
        if (ScriptData[scriptInfo->subStartup.scriptCodePtr] > 0)
            ProcessScript(scriptInfo->subStartup.scriptCodePtr, scriptInfo->subStartup.jumpTablePtr, SUB_SETUP);
        scriptInfo->frameCount = ScriptFramesNo - frameStart;
    }
    entity->type     = 0;
    curObjectType    = 0;
    ScriptFramesNo = 0;
}

void ProcessObjects() {
    for (int i = 0; i < DRAWLAYER_COUNT; ++i) ObjectDrawOrderList[i].listSize = 0;

    for (ObjectLoop = 0; ObjectLoop < ENTITY_COUNT; ++ObjectLoop) {
        bool active = false;
        int x = 0, y = 0;
        Entity *entity = &ObjectEntityList[ObjectLoop];

        if (entity->priority <= 0) {
            x      = entity->XPos >> 16;
            y      = entity->YPos >> 16;
            active = x > XScrollOffset - OBJECT_BORDER_X1 && x < OBJECT_BORDER_X2 + XScrollOffset && y > YScrollOffset - OBJECT_BORDER_Y1
                     && y < YScrollOffset + OBJECT_BORDER_Y2;
        } else {
            active = true;
        }

        if (active && entity->type > OBJ_TYPE_BLANKOBJECT) {
            if (entity->type == OBJ_TYPE_PLAYER) {
                if (ObjectLoop >= 2) {
                    entity->type = OBJ_TYPE_BLANKOBJECT;
                } else {
                    Player *player       = &PlayerList[ObjectLoop];
                    PlayerScript *script = &PlayerScriptList[ObjectLoop];
                    switch (entity->propertyValue) {
                        case 0:
                            PlayerNo = ObjectLoop;
                            ProcessPlayerControl(player);
                            player->animationSpeed = 0;
                            if (ScriptData[script->scriptCodePtr_PlayerMain] > 0)
                                ProcessScript(script->scriptCodePtr_PlayerMain, script->jumpTablePtr_PlayerMain, SUB_PLAYERMAIN);
                            if (ScriptData[script->scriptCodePtr_PlayerState[player->state]] > 0)
                                ProcessScript(script->scriptCodePtr_PlayerState[player->state], script->jumpTablePtr_PlayerState[player->state],
                                              SUB_PLAYERSTATE);
                            ProcessPlayerAnimation(player);
                            if (player->tileCollisions)
                                ProcessPlayerTileCollisions(player);
                            break;
                        case 1:
                            ProcessPlayerControl(player);
                            ProcessPlayerAnimation(player);
                            if (ScriptData[script->scriptCodePtr_PlayerMain] > 0)
                                ProcessScript(script->scriptCodePtr_PlayerMain, script->jumpTablePtr_PlayerMain, SUB_PLAYERMAIN);
                            if (player->tileCollisions)
                                ProcessPlayerTileCollisions(player);
                            break;
                        case 2:
                            ProcessPlayerControl(player);
                            ProcessDebugMode(player);
                            if (!ObjectLoop) {
                                CameraEnabled = true;
                                if (GKeyPress.B) {
                                    player->tileCollisions                     = true;
                                    player->objectInteraction                  = true;
                                    player->controlMode                        = CONTROLMODE_NORMAL;
                                    ObjectEntityList[ObjectLoop].propertyValue = 0;
                                }
                            }
                            break;
                    }
                    if (entity->drawOrder < DRAWLAYER_COUNT)
                        ObjectDrawOrderList[entity->drawOrder].entityRefs[ObjectDrawOrderList[entity->drawOrder].listSize++] = ObjectLoop;
                }
            } else {
                ObjectScript *scriptInfo = &ObjectScriptList[entity->type];
                PlayerNo                 = 0;
                if (ScriptData[scriptInfo->subMain.scriptCodePtr] > 0)
                    ProcessScript(scriptInfo->subMain.scriptCodePtr, scriptInfo->subMain.jumpTablePtr, SUB_MAIN);
                if (ScriptData[scriptInfo->subPlayerInteraction.scriptCodePtr] > 0) {
                    while (PlayerNo < activePlayerCount) {
                        if (PlayerList[PlayerNo].objectInteraction)
                            ProcessScript(scriptInfo->subPlayerInteraction.scriptCodePtr, scriptInfo->subPlayerInteraction.jumpTablePtr,
                                          SUB_PLAYERINTERACTION);
                        ++PlayerNo;
                    }
                }

                if (entity->drawOrder < DRAWLAYER_COUNT)
                    ObjectDrawOrderList[entity->drawOrder].entityRefs[ObjectDrawOrderList[entity->drawOrder].listSize++] = ObjectLoop;
            }
        }
    }
}