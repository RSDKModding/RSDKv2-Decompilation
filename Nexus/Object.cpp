#include "RetroEngine.hpp"

int objectLoop    = 0;
int curObjectType = 0;
Entity objectEntityList[ENTITY_COUNT];

int OBJECT_BORDER_X1       = 0x80;
int OBJECT_BORDER_X2       = 0;
const int OBJECT_BORDER_Y1 = 0x100;
const int OBJECT_BORDER_Y2 = SCREEN_YSIZE + 0x100;

void ProcessStartupObjects()
{
    scriptFrameCount           = 0;
    ClearAnimationData();
    activePlayer               = 0;
    activePlayerCount          = 1;
    scriptEng.arrayPosition[2] = TEMPENTITY_START;
    Entity *entity             = &objectEntityList[TEMPENTITY_START];
    for (int i = 0; i < OBJECT_COUNT; ++i) {
        ObjectScript *scriptInfo    = &objectScriptList[i];
        objectLoop                  = TEMPENTITY_START;
        curObjectType               = i;
        int frameStart              = scriptFrameCount;
        scriptInfo->frameStartPtr   = &scriptFrames[scriptFrameCount];
        scriptInfo->spriteSheetID   = 0;
        entity->type                = i;
        if (scriptData[scriptInfo->subStartup.scriptCodePtr] > 0)
            ProcessScript(scriptInfo->subStartup.scriptCodePtr, scriptInfo->subStartup.jumpTablePtr, SUB_SETUP);
        scriptInfo->frameCount = scriptFrameCount - frameStart;
    }
    entity->type  = 0;
    curObjectType = 0;
}

void ProcessObjects()
{
    for (int i = 0; i < DRAWLAYER_COUNT; ++i) drawListEntries[i].listSize = 0;

    for (objectLoop = 0; objectLoop < ENTITY_COUNT; ++objectLoop) {
        bool active = false;
        int x = 0, y = 0;
        Entity *entity = &objectEntityList[objectLoop];

        if (entity->priority <= 0) {
            x      = entity->XPos >> 16;
            y      = entity->YPos >> 16;
            active = x > xScrollOffset - OBJECT_BORDER_X1 && x < OBJECT_BORDER_X2 + xScrollOffset && y > yScrollOffset - OBJECT_BORDER_Y1
                     && y < yScrollOffset + OBJECT_BORDER_Y2;
        }
        else {
            active = true;
        }

        if (active && entity->type > OBJ_TYPE_BLANKOBJECT) {
            if (entity->type == OBJ_TYPE_PLAYER) {
                if (objectLoop >= 2) {
                    entity->type = OBJ_TYPE_BLANKOBJECT;
                }
                else {
                    Player *player = &playerList[objectLoop];
                    PlayerScript *script = &playerScriptList[objectLoop];
                    switch (entity->propertyValue) {
                        case 0:
                            activePlayer = objectLoop;
                            ProcessPlayerControl(player);
                            player->animationSpeed = 0;
                            if (scriptData[script->scriptCodePtr_PlayerMain] > 0)
                                ProcessScript(script->scriptCodePtr_PlayerMain, script->jumpTablePtr_PlayerMain, SUB_PLAYERMAIN);
                            if (scriptData[script->scriptCodePtr_PlayerState[player->state]] > 0)
                                ProcessScript(script->scriptCodePtr_PlayerState[player->state], script->jumpTablePtr_PlayerState[player->state],
                                              SUB_PLAYERSTATE);
                            ProcessPlayerAnimation(player);
                            if (player->tileCollisions)
                                ProcessPlayerTileCollisions(player);
                            break;
                        case 1:
                            ProcessPlayerControl(player);
                            ProcessPlayerAnimation(player);
                            if (scriptData[script->scriptCodePtr_PlayerMain] > 0)
                                ProcessScript(script->scriptCodePtr_PlayerMain, script->jumpTablePtr_PlayerMain, SUB_PLAYERMAIN);
                            if (player->tileCollisions)
                                ProcessPlayerTileCollisions(player);
                            break;
                        case 2:
                            ProcessPlayerControl(player);
                            ProcessDebugMode(player);
                            if (!objectLoop) {
                                cameraEnabled = true;
                                if (keyPress.B) {
                                    player->tileCollisions                     = true;
                                    player->objectInteraction                  = true;
                                    player->controlMode                        = 0;
                                    objectEntityList[objectLoop].propertyValue = 0;
                                }
                            }
                            break;
                    }
                    if (entity->drawOrder < DRAWLAYER_COUNT)
                        drawListEntries[entity->drawOrder].entityRefs[drawListEntries[entity->drawOrder].listSize++] = objectLoop;
                }
            }
            else {
                ObjectScript *scriptInfo = &objectScriptList[entity->type];
                activePlayer             = 0;
                if (scriptData[scriptInfo->subMain.scriptCodePtr] > 0)
                    ProcessScript(scriptInfo->subMain.scriptCodePtr, scriptInfo->subMain.jumpTablePtr, SUB_MAIN);
                if (scriptData[scriptInfo->subPlayerInteraction.scriptCodePtr] > 0) {
                    while (activePlayer < activePlayerCount) {
                        if (playerList[activePlayer].objectInteraction)
                            ProcessScript(scriptInfo->subPlayerInteraction.scriptCodePtr, scriptInfo->subPlayerInteraction.jumpTablePtr,
                                          SUB_PLAYERINTERACTION);
                        ++activePlayer;
                    }
                }

                if (entity->drawOrder < DRAWLAYER_COUNT)
                    drawListEntries[entity->drawOrder].entityRefs[drawListEntries[entity->drawOrder].listSize++] = objectLoop;
            }
        }
    }
}