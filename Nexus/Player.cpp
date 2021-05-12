#include "RetroEngine.hpp"

Player playerList[PLAYER_COUNT];
PlayerScript playerScriptList[PLAYER_COUNT];
int activePlayer  = 0;
int activePlayerCount = 1;

ushort upBuffer        = 0;
ushort downBuffer      = 0;
ushort leftBuffer      = 0;
ushort rightBuffer     = 0;
ushort jumpPressBuffer = 0;
ushort jumpHoldBuffer  = 0;

void LoadPlayerFromList(byte characterID, byte playerID)
{
    FileInfo info;
    char strBuf[0x100];
    byte fileBuffer = 0;
    byte count      = 0;
    byte strLen     = 0;
    if (LoadFile("Data/Game/GameConfig.bin", &info)) {
        // Name
        FileRead(&strLen, 1);
        FileRead(&strBuf, strLen);
        strBuf[strLen] = 0;

        // 'Data'
        FileRead(&strLen, 1);
        FileRead(&strBuf, strLen);
        strBuf[strLen] = 0;

        // About
        FileRead(&strLen, 1);
        FileRead(&strBuf, strLen);
        strBuf[strLen] = 0;

        // Script Paths
        FileRead(&count, 1);
        for (int s = 0; s < count; ++s) {
            FileRead(&strLen, 1);
            FileRead(&strBuf, strLen);
            strBuf[strLen] = 0;
        }

        // Variables
        FileRead(&count, 1);
        for (int v = 0; v < count; ++v) {
            // Var Name
            FileRead(&strLen, 1);
            FileRead(&strBuf, strLen);
            strBuf[strLen] = 0;

            // Var Value
            FileRead(&fileBuffer, 1);
            FileRead(&fileBuffer, 1);
            FileRead(&fileBuffer, 1);
            FileRead(&fileBuffer, 1);
        }

        // SFX
        FileRead(&count, 1);
        for (int s = 0; s < count; ++s) {
            FileRead(&strLen, 1);
            FileRead(&strBuf, strLen);
            strBuf[strLen] = 0;
        }

        // Players
        FileRead(&count, 1);
        for (int p = 0; p < count; ++p) {
            FileRead(&strLen, 1);
            FileRead(&strBuf, strLen); //player anim file
            strBuf[strLen] = '\0';

            FileRead(&strLen, 1);
            FileRead(&playerScriptList[p].scriptPath, strLen); // player script file
            playerScriptList[p].scriptPath[strLen] = '\0';

            if (characterID == p) {
                GetFileInfo(&info);
                CloseFile();
                LoadPlayerAnimation(strBuf, playerID);
                SetFileInfo(&info);
            }
            FileRead(&strLen, 1);
            FileRead(&strBuf, strLen); // player name
            strBuf[strLen] = '\0';
        }
        CloseFile();
    }
}

void ProcessPlayerAnimationChange(Player *player)
{
    if (player->animation != player->prevAnimation) {
        if (player->animation == ANI_JUMPING)
            player->YPos += (hitboxList[0].bottom[0] - hitboxList[1].bottom[0]) << 16;
        if (player->prevAnimation == ANI_JUMPING)
            player->YPos -= (hitboxList[0].bottom[0] - hitboxList[1].bottom[0]) << 16;
        player->prevAnimation  = player->animation;
        player->frame          = 0;
        player->animationTimer = 0;
    }
}

void DrawPlayer(Player *player, SpriteFrame *frame)
{
    int rotation = 0;
    switch (player->animation) {
        case ANI_RUNNING:
        case ANI_WALKING:
        case ANI_PEELOUT:
        case ANI_CORKSCREW:
            if (player->rotation >= 0x80)
                rotation = 0x200 - ((266 - player->rotation) >> 5 << 6);
            else
                rotation = (player->rotation + 10) >> 5 << 6;
            break;
        default: break;
    }
    DrawSpriteRotated(player->direction, player->screenXPos, player->screenYPos, -frame->pivotX, -frame->pivotY, frame->sprX, frame->sprY,
                      frame->width, frame->height, rotation, frame->sheetID);
}

void ProcessPlayerControl(Player *Player)
{
    if (Player->controlMode == -1) {
        upBuffer <<= 1;
        upBuffer |= (byte)Player->up;
        downBuffer <<= 1;
        downBuffer |= (byte)Player->down;
        leftBuffer <<= 1;
        leftBuffer |= (byte)Player->left;
        rightBuffer <<= 1;
        rightBuffer |= (byte)Player->right;
        jumpPressBuffer <<= 1;
        jumpPressBuffer |= (byte)Player->jumpPress;
        jumpHoldBuffer <<= 1;
        jumpHoldBuffer |= (byte)Player->jumpHold;
    }
    else if (Player->controlMode == 1) {
        Player->up        = upBuffer >> 15;
        Player->down      = downBuffer >> 15;
        Player->left      = leftBuffer >> 15;
        Player->right     = rightBuffer >> 15;
        Player->jumpPress = jumpPressBuffer >> 15;
        Player->jumpHold  = jumpHoldBuffer >> 15;
    }
    else {
        Player->up   = keyDown.up;
        Player->down = keyDown.down;
        if (!keyDown.left || !keyDown.right) {
            Player->left  = keyDown.left;
            Player->right = keyDown.right;
        }
        else {
            Player->left  = false;
            Player->right = false;
        }
        Player->jumpHold  = keyDown.C | keyDown.B | keyDown.A;
        Player->jumpPress = keyPress.C | keyPress.B | keyPress.A;
        upBuffer <<= 1;
        upBuffer |= (byte)Player->up;
        downBuffer <<= 1;
        downBuffer |= (byte)Player->down;
        leftBuffer <<= 1;
        leftBuffer |= (byte)Player->left;
        rightBuffer <<= 1;
        rightBuffer |= (byte)Player->right;
        jumpPressBuffer <<= 1;
        jumpPressBuffer |= (byte)Player->jumpPress;
        jumpHoldBuffer <<= 1;
        jumpHoldBuffer |= (byte)Player->jumpHold;
    }
}

void SetMovementStats(PlayerMovementStats *stats)
{
    stats->topSpeed            = 0x60000;
    stats->acceleration        = 0xC00;
    stats->deceleration        = 0xC00;
    stats->airAcceleration     = 0x1800;
    stats->airDeceleration     = 0x600;
    stats->gravityStrength     = 0x3800;
    stats->jumpStrength        = 0x68000;
    stats->rollingDeceleration = 0x2000;
}

void DefaultAirMovement(Player *player)
{
    if (player->YVelocity > -0x40000 && player->YVelocity < 0)
        player->speed -= player->speed >> 5;

    if (player->speed <= -player->stats.topSpeed) {
        if (player->left) {
            player->direction = FLIP_X;
        }
    }
    else {
        if (player->left) {
            player->speed -= player->stats.airAcceleration;
            player->direction = FLIP_X;
        }
    }

    if (player->speed >= player->stats.topSpeed) {
        if (player->right)
            player->direction = FLIP_NONE;
    }
    else if (player->right) {
        player->direction = FLIP_NONE;
        player->speed += player->stats.airAcceleration;
    }
}

void DefaultGravityFalse(Player *player)
{
    player->trackScroll = false;
    player->XVelocity   = player->speed * cosVal256[player->angle] >> 8;
    player->YVelocity   = player->speed * sinVal256[player->angle] >> 8;
}

void DefaultGravityTrue(Player *player)
{
    player->trackScroll = true;
    player->YVelocity += player->stats.gravityStrength;
    if (player->YVelocity >= -0x40000) {
        player->timer = 0;
    }
    else if (!player->jumpHold && player->timer > 0) {
        player->timer     = 0;
        player->YVelocity = -0x3C800;
        player->speed -= player->speed >> 5;
    }
    player->XVelocity = player->speed;
    if (player->rotation <= 0 || player->rotation >= 128) {
        if (player->rotation > 127 && player->rotation < 256) {
            player->rotation += 2;
            if (player->rotation > 255) {
                player->rotation = 0;
            }
        }
    }
    else {
        player->rotation -= 2;
        if (player->rotation < 1)
            player->rotation = 0;
    }
}

void DefaultGroundMovement(Player *player)
{
    if ((signed int)player->frictionLoss <= 0) {
        if (player->left && player->speed > -player->stats.topSpeed) {
            if (player->speed <= 0) {
                player->speed -= player->stats.acceleration;
                player->skidding = 0;
            }
            else {
                if (player->speed > 0x40000)
                    player->skidding = 16;
                if (player->speed >= 0x8000) {
                    player->speed -= 0x8000;
                }
                else {
                    player->speed    = -0x8000;
                    player->skidding = 0;
                }
            }
        }
        if (player->right && player->speed < player->stats.topSpeed) {
            if (player->speed >= 0) {
                player->speed += player->stats.acceleration;
                player->skidding = 0;
            }
            else {
                if (player->speed < -0x40000)
                    player->skidding = 16;
                if (player->speed <= -0x8000) {
                    player->speed += 0x8000;
                }
                else {
                    player->speed    = 0x8000;
                    player->skidding = 0;
                }
            }
        }

        if (player->left && player->speed <= 0)
            player->direction = FLIP_X;
        if (player->right && player->speed >= 0)
            player->direction = FLIP_NONE;

        if (player->left || player->right) {
            switch (player->collisionMode) {
                case CMODE_FLOOR:
                    player->speed += sinVal256[player->angle] << 13 >> 8;
                    break;
                case CMODE_LWALL:
                    if (player->angle >= 176) {
                        player->speed += (sinVal256[player->angle] << 13 >> 8);
                    }
                    else {
                        if (player->speed < -0x60000 || player->speed > 0x60000)
                            player->speed += sinVal256[player->angle] << 13 >> 8;
                        else
                            player->speed += 0x1400 * sinVal256[player->angle] >> 8;
                    }
                    break;
                case CMODE_ROOF:
                    if (player->speed < -0x60000 || player->speed > 0x60000)
                        player->speed += sinVal256[player->angle] << 13 >> 8;
                    else
                        player->speed += 0x1400 * sinVal256[player->angle] >> 8;
                    break;
                case CMODE_RWALL:
                    if (player->angle <= 80) {
                        player->speed += sinVal256[player->angle] << 13 >> 8;
                    }
                    else {
                        if (player->speed < -0x60000 || player->speed > 0x60000)
                            player->speed += sinVal256[player->angle] << 13 >> 8;
                        else
                            player->speed += 0x1400 * sinVal256[player->angle] >> 8;
                    }
                    break;
                default: break;
            }

            if (player->angle > 192) {
                if (player->angle < 226 && !player->left) {
                    if (player->right && player->speed < 0x20000) {
                        if (player->speed > -0x60000)
                            player->frictionLoss = 30;
                    }
                }
            }
            if (player->angle > 30) {
                if (player->angle < 64 && player->left) {
                    if (!player->right && player->speed > -0x20000) {
                        if (player->speed < 0x60000)
                            player->frictionLoss = 30;
                    }
                }
            }
        }
        else {
            if (player->speed < 0) {
                player->speed += player->stats.deceleration;
                if (player->speed > 0)
                    player->speed = 0;
            }
            if (player->speed > 0) {
                player->speed -= player->stats.deceleration;
                if (player->speed < 0)
                    player->speed = 0;
            }
            if (player->speed < -0x4000 || player->speed > 0x4000)
                player->speed += sinVal256[player->angle] << 13 >> 8;
            if ((player->angle > 30 && player->angle < 64) || (player->angle > 192 && player->angle < 226)) {
                if (player->speed > -0x10000 && player->speed < 0x10000)
                    player->frictionLoss = 30;
            }
        }
    }
    else {
        --player->frictionLoss;
        player->speed = (sinVal256[player->angle] << 13 >> 8) + player->speed;
    }
}

void DefaultJumpAction(Player *player)
{
    player->frictionLoss  = 0;
    player->gravity       = true;
    player->XVelocity     = (player->speed * cosVal256[player->angle] + player->stats.jumpStrength * sinVal256[player->angle]) >> 8;
    player->YVelocity     = (player->speed * sinVal256[player->angle] + -player->stats.jumpStrength * cosVal256[player->angle]) >> 8;
    player->speed         = player->XVelocity;
    player->trackScroll   = true;
    player->animation     = ANI_JUMPING;
    player->angle         = 0;
    player->collisionMode = CMODE_FLOOR;
    player->timer         = 1;
}

void DefaultRollingMovement(Player *player)
{

    if (player->right && player->speed < 0)
        player->speed += player->stats.rollingDeceleration;
    if (player->left && player->speed > 0)
        player->speed -= player->stats.rollingDeceleration;

    if (player->speed < 0) {
        player->speed += player->stats.airDeceleration;
        if (player->speed > 0)
            player->speed = 0;
    }
    if (player->speed > 0) {
        player->speed -= player->stats.airDeceleration;
        if (player->speed < 0)
            player->speed = 0;
    }
    if ((player->angle < 12 || player->angle > 244) && !player->speed)
        player->state = 0;

    if (player->speed <= 0) {
        if (sinVal256[player->angle] >= 0) {
            player->speed += (player->stats.rollingDeceleration * sinVal256[player->angle] >> 8);
        }
        else {
            player->speed += 0x5000 * sinVal256[player->angle] >> 8;
        }
    }
    else if (sinVal256[player->angle] <= 0) {
        player->speed += (player->stats.rollingDeceleration * sinVal256[player->angle] >> 8);
    }
    else {
        player->speed += 0x5000 * sinVal256[player->angle] >> 8;
    }

    if (player->speed > 0x180000)
        player->speed = 0x180000;
}

void ProcessDebugMode(Player *player)
{
    if (player->down || player->up || player->right || player->left) {
        if (player->speed < 0x100000) {
            player->speed += 0xC00;
            if (player->speed > 0x100000)
                player->speed = 0x100000;
        }
    }
    else {
        player->speed = 0;
    }

    if (keyDown.left)
        player->XPos -= player->speed;
    if (keyDown.right)
        player->XPos += player->speed;

    if (keyDown.up)
        player->YPos -= player->speed;
    if (keyDown.down)
        player->YPos += player->speed;
}

void ProcessPlayerAnimation(Player *player)
{
    PlayerScript *script = &playerScriptList[player->type];
    if (!player->gravity) {
        int speed = (player->jumpingSpeed * abs(player->speed) / 6 >> 16) + 48;
        if (speed > 0xF0)
            speed = 0xF0;
        script->animations[ANI_JUMPING].speed = speed;

        switch (player->animation) {
            case ANI_WALKING: script->animations[player->animation].speed = ((uint)(player->walkingSpeed * abs(player->speed) / 6) >> 16) + 20; break;
            case ANI_RUNNING:
                speed = player->runningSpeed * abs(player->speed) / 6 >> 16;
                if (speed > 0xF0)
                    speed = 0xF0;
                script->animations[player->animation].speed = speed;
                break;
            case ANI_PEELOUT:
                speed = player->runningSpeed * abs(player->speed) / 6 >> 16;
                if (speed > 0xF0)
                    speed = 0xF0;
                script->animations[player->animation].speed = speed;
                break;
        }
    }
    if ((signed int)player->animationSpeed <= 0)
        player->animationTimer += script->animations[player->animation].speed;
    else
        player->animationTimer += player->animationSpeed;
    if (player->animation != player->prevAnimation) {
        if (player->animation == ANI_JUMPING)
            player->YPos += (hitboxList[0].bottom[0] - hitboxList[1].bottom[0]) << 16;
        if (player->prevAnimation == ANI_JUMPING)
            player->YPos -= (hitboxList[0].bottom[0] - hitboxList[1].bottom[0]) << 16;
        player->prevAnimation  = player->animation;
        player->frame          = 0;
        player->animationTimer = 0;
    }
    if (player->animationTimer >= 0xF0) {
        player->animationTimer -= 0xF0;
        ++player->frame;
    }
    if (player->frame == script->animations[player->animation].frameCount) {
        player->frame = script->animations[player->animation].loopPoint;
    }
}