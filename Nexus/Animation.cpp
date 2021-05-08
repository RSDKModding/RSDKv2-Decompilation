#include "RetroEngine.hpp"

SpriteFrame scriptFrames[SPRITEFRAME_COUNT];
int scriptFrameCount = 0;

SpriteFrame animFrames[SPRITEFRAME_COUNT];
Hitbox hitboxList[HITBOX_COUNT];

void LoadPlayerAnimation(const char *filePath, int playerID)
{
    FileInfo info;
    if (LoadFile(filePath, &info)) {
        byte fileBuffer = 0;
        char strBuf[0x21];
        byte sheetIDs[4];
        sheetIDs[0] = 0;

        FileRead(&fileBuffer, 1);
        FileRead(&fileBuffer, 1);
        FileRead(&fileBuffer, 1);
        FileRead(&fileBuffer, 1);
        FileRead(&fileBuffer, 1);

        //Read & load each spritesheet
        for (int s = 0; s < 4; ++s) {
            FileRead(&fileBuffer, 1);
            if (fileBuffer) {
                int i = 0;
                for (; i < fileBuffer; ++i) FileRead(&strBuf[i], 1);
                strBuf[i] = 0;
                GetFileInfo(&info);
                CloseFile();

                RemoveGraphicsFile("", i + 4 * playerID + 16);
                switch (strBuf[strlen(strBuf) - 3]) {
                    case 'f': sheetIDs[s] = LoadGIFFile(strBuf, i + 4 * playerID + 16); break;
                    case 'p': sheetIDs[s] = LoadBMPFile(strBuf, i + 4 * playerID + 16); break;
                    case 'x': sheetIDs[s] = LoadGFXFile(strBuf, i + 4 * playerID + 16); break;
                }

                SetFileInfo(&info);
            }
        }

        byte animCount = 0;
        FileRead(&animCount, 1);

        //Read animations
        int frameID = playerID << 10;
        for (int a = 0; a < animCount; ++a) {
            SpriteAnimation *anim = &playerScriptList[playerID].animations[a];
            FileRead(&anim->frameCount, 1);
            FileRead(&anim->speed, 1);
            FileRead(&anim->loopPoint, 1);
            anim->frames = &animFrames[frameID];

            for (int j = 0; j < anim->frameCount; ++j) {
                SpriteFrame *frame = &animFrames[frameID++];
                FileRead(&frame->sheetID, 1);
                frame->sheetID = sheetIDs[frame->sheetID];
                FileRead(&frame->hitboxID, 1);
                FileRead(&fileBuffer, 1);
                frame->sprX = fileBuffer;
                FileRead(&fileBuffer, 1);
                frame->sprY = fileBuffer;
                FileRead(&fileBuffer, 1);
                frame->width = fileBuffer;
                FileRead(&fileBuffer, 1);
                frame->height = fileBuffer;

                sbyte buffer = 0;
                FileRead(&buffer, 1);
                frame->pivotX = buffer;
                FileRead(&buffer, 1);
                frame->pivotY = buffer;
            }
        }

        //Read Hitboxes
        FileRead(&fileBuffer, 1);
        int hitboxID = playerID << 3;
        for (int i = 0; i < fileBuffer; ++i) {
            Hitbox *hitbox = &hitboxList[hitboxID++];
            for (int d = 0; d < HITBOX_DIR_COUNT; ++d) {
                FileRead(&hitbox->left[d], 1);
                FileRead(&hitbox->top[d], 1);
                FileRead(&hitbox->right[d], 1);
                FileRead(&hitbox->bottom[d], 1);
            }
        }
        playerScriptList[playerID].startWalkSpeed  = playerScriptList[playerID].animations[5].speed - 20;
        playerScriptList[playerID].startRunSpeed   = playerScriptList[playerID].animations[6].speed;
        playerScriptList[playerID].startJumpSpeed  = playerScriptList[playerID].animations[10].speed - 48;

        CloseFile();
    }
}
void ClearAnimationData()
{
    for (int f = 0; f < SPRITEFRAME_COUNT; ++f) MEM_ZERO(scriptFrames[f]);
    for (int f = 0; f < SPRITEFRAME_COUNT; ++f) MEM_ZERO(animFrames[f]);
    for (int h = 0; h < HITBOX_COUNT; ++h) MEM_ZERO(hitboxList[h]);

    scriptFrameCount   = 0;
}
