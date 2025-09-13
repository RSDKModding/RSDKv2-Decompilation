#include "RetroEngine.hpp"

Player PlayerList[PLAYER_COUNT];
PlayerScript PlayerScriptList[PLAYER_COUNT];
int PlayerNo          = 0;
int activePlayerCount = 1;

ushort DelayUp        = 0;
ushort DelayDown      = 0;
ushort DelayLeft      = 0;
ushort DelayRight     = 0;
ushort DelayJumpPress = 0;
ushort DelayJumpHold  = 0;

void LoadPlayerFromList(byte characterID, byte playerID) {
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
#if RETRO_USE_MOD_LOADER
        playerNames.resize(count);
#endif
        for (int p = 0; p < count; ++p) {
            FileRead(&strLen, 1);
            FileRead(&strBuf, strLen); // player anim file
            strBuf[strLen] = '\0';

            FileRead(&strLen, 1);
            FileRead(&PlayerScriptList[p].scriptPath, strLen); // player script file
            PlayerScriptList[p].scriptPath[strLen] = '\0';

            if (characterID == p) {
                GetFileInfo(&info);
                CloseFile();
#if RETRO_USE_MOD_LOADER
                strcpy(PlayerScriptList[playerID].scriptPath, PlayerScriptList[p].scriptPath);
#endif
                LoadPlayerAnimation(strBuf, playerID);
                SetFileInfo(&info);
            }
            FileRead(&strLen, 1);
            FileRead(&strBuf, strLen); // player name
            strBuf[strLen] = '\0';

#if RETRO_USE_MOD_LOADER
            playerNames[p] = strBuf; // Add the player's name to the playerNames vector
#endif
        }
        CloseFile();
    }
}

void ProcessPlayerAnimationChange(Player *player) {
    if (player->animation != player->prevAnimation) {
        if (player->animation == ANI_JUMPING)
            player->YPos += (PlayerCBoxes[0].bottom[0] - PlayerCBoxes[1].bottom[0]) << 16;
        if (player->prevAnimation == ANI_JUMPING)
            player->YPos -= (PlayerCBoxes[0].bottom[0] - PlayerCBoxes[1].bottom[0]) << 16;
        player->prevAnimation  = player->animation;
        player->frame          = 0;
        player->animationTimer = 0;
    }
}

void DrawPlayer(Player *player, SpriteFrame *frame) {
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
    DrawRotatedSprite(player->direction, player->screenXPos, player->screenYPos, -frame->pivotX, -frame->pivotY, frame->sprX, frame->sprY,
                      frame->width, frame->height, rotation, frame->sheetID);
}

void ProcessPlayerControl(Player *Player) {
    if (Player->controlMode == CONTROLMODE_NONE) {
        DelayUp <<= 1;
        DelayUp |= (byte)Player->up;
        DelayDown <<= 1;
        DelayDown |= (byte)Player->down;
        DelayLeft <<= 1;
        DelayLeft |= (byte)Player->left;
        DelayRight <<= 1;
        DelayRight |= (byte)Player->right;
        DelayJumpPress <<= 1;
        DelayJumpPress |= (byte)Player->jumpPress;
        DelayJumpHold <<= 1;
        DelayJumpHold |= (byte)Player->jumpHold;
    } else if (Player->controlMode == CONTROLMODE_SIDEKICK) {
        Player->up        = DelayUp >> 15;
        Player->down      = DelayDown >> 15;
        Player->left      = DelayLeft >> 15;
        Player->right     = DelayRight >> 15;
        Player->jumpPress = DelayJumpPress >> 15;
        Player->jumpHold  = DelayJumpHold >> 15;
    } else if (Player->controlMode == CONTROLMODE_NORMAL) {
        Player->up   = GKeyDown.up;
        Player->down = GKeyDown.down;
        if (!GKeyDown.left || !GKeyDown.right) {
            Player->left  = GKeyDown.left;
            Player->right = GKeyDown.right;
        } else {
            Player->left  = false;
            Player->right = false;
        }
        Player->jumpHold  = GKeyDown.C | GKeyDown.B | GKeyDown.A;
        Player->jumpPress = GKeyPress.C | GKeyPress.B | GKeyPress.A;
        DelayUp <<= 1;
        DelayUp |= (byte)Player->up;
        DelayDown <<= 1;
        DelayDown |= (byte)Player->down;
        DelayLeft <<= 1;
        DelayLeft |= (byte)Player->left;
        DelayRight <<= 1;
        DelayRight |= (byte)Player->right;
        DelayJumpPress <<= 1;
        DelayJumpPress |= (byte)Player->jumpPress;
        DelayJumpHold <<= 1;
        DelayJumpHold |= (byte)Player->jumpHold;
    }
}

void SetMovementStats(PlayerMovementStats *stats) {
    stats->topSpeed            = 0x60000;
    stats->acceleration        = 0xC00;
    stats->deceleration        = 0xC00;
    stats->airAcceleration     = 0x1800;
    stats->airDeceleration     = 0x600;
    stats->gravityStrength     = 0x3800;
    stats->jumpStrength        = 0x68000;
    stats->rollingDeceleration = 0x2000;
}

void ProcessDefaultAirMovement(Player *player) {
    if (player->speed <= -player->stats.topSpeed) {
        if (player->left)
            player->direction = FLIP_X;
    } else if (player->left) {
        player->speed -= player->stats.airAcceleration;
        player->direction = FLIP_X;
    }
    if (player->speed >= player->stats.topSpeed) {
        if (player->right)
            player->direction = FLIP_NONE;
    } else if (player->right) {
        player->speed += player->stats.airAcceleration;
        player->direction = FLIP_NONE;
    }

    if (player->YVelocity > -0x40001 && player->YVelocity < 1)
        player->speed -= player->speed >> 5;
}

void ProcessDefaultGravityFalse(Player *player) {
    player->trackScroll = false;
    player->XVelocity   = (player->speed * CosValue256[player->angle]) >> 8;
    player->YVelocity   = (player->speed * SinValue256[player->angle]) >> 8;
}

void ProcessDefaultGravityTrue(Player *player) {
    player->trackScroll = true;
    player->YVelocity += player->stats.gravityStrength;
    if (player->YVelocity >= -0x33CB0) {
        player->timer = 0;
    } else if (!player->jumpHold && player->timer > 0) {
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
    } else {
        player->rotation -= 2;
        if (player->rotation < 1)
            player->rotation = 0;
    }
}

void ProcessDefaultGroundMovement(Player *player) {
    if ((signed int)player->frictionLoss <= 0) {
        if (player->left && player->speed > -player->stats.topSpeed) {
            if (player->speed <= 0) {
                player->speed -= player->stats.acceleration;
                player->skidding = 0;
            } else {
                if (player->speed > 0x40000)
                    player->skidding = 16;
                if (player->speed >= 0x8000) {
                    player->speed -= 0x8000;
                } else {
                    player->speed    = -0x8000;
                    player->skidding = 0;
                }
            }
        }
        if (player->right && player->speed < player->stats.topSpeed) {
            if (player->speed >= 0) {
                player->speed += player->stats.acceleration;
                player->skidding = 0;
            } else {
                if (player->speed < -0x40000)
                    player->skidding = 16;
                if (player->speed <= -0x8000) {
                    player->speed += 0x8000;
                } else {
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
                case CMODE_FLOOR: player->speed += SinValue256[player->angle] << 13 >> 8; break;
                case CMODE_LWALL:
                    if (player->angle >= 176) {
                        player->speed += (SinValue256[player->angle] << 13 >> 8);
                    } else {
                        if (player->speed < -0x60000 || player->speed > 0x60000)
                            player->speed += SinValue256[player->angle] << 13 >> 8;
                        else
                            player->speed += 0x1400 * SinValue256[player->angle] >> 8;
                    }
                    break;
                case CMODE_ROOF:
                    if (player->speed < -0x60000 || player->speed > 0x60000)
                        player->speed += SinValue256[player->angle] << 13 >> 8;
                    else
                        player->speed += 0x1400 * SinValue256[player->angle] >> 8;
                    break;
                case CMODE_RWALL:
                    if (player->angle <= 80) {
                        player->speed += SinValue256[player->angle] << 13 >> 8;
                    } else {
                        if (player->speed < -0x60000 || player->speed > 0x60000)
                            player->speed += SinValue256[player->angle] << 13 >> 8;
                        else
                            player->speed += 0x1400 * SinValue256[player->angle] >> 8;
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
        } else {
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
                player->speed += SinValue256[player->angle] << 13 >> 8;
            if ((player->angle > 30 && player->angle < 64) || (player->angle > 192 && player->angle < 226)) {
                if (player->speed > -0x10000 && player->speed < 0x10000)
                    player->frictionLoss = 30;
            }
        }
    } else {
        --player->frictionLoss;
        player->speed = (SinValue256[player->angle] << 13 >> 8) + player->speed;
    }
}

void ProcessDefaultJumpAction(Player *player) {
    player->frictionLoss  = 0;
    player->gravity       = true;
    player->XVelocity     = (player->speed * CosValue256[player->angle] + player->stats.jumpStrength * SinValue256[player->angle]) >> 8;
    player->YVelocity     = (player->speed * SinValue256[player->angle] + -player->stats.jumpStrength * CosValue256[player->angle]) >> 8;
    player->speed         = player->XVelocity;
    player->trackScroll   = true;
    player->animation     = ANI_JUMPING;
    player->angle         = 0;
    player->collisionMode = CMODE_FLOOR;
    player->timer         = 1;
}

void ProcessDefaultRollingMovement(Player *player) {
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
        if (SinValue256[player->angle] >= 0) {
            player->speed += (player->stats.rollingDeceleration * SinValue256[player->angle] >> 8);
        } else {
            player->speed += 0x5000 * SinValue256[player->angle] >> 8;
        }
    } else if (SinValue256[player->angle] <= 0) {
        player->speed += (player->stats.rollingDeceleration * SinValue256[player->angle] >> 8);
    } else {
        player->speed += 0x5000 * SinValue256[player->angle] >> 8;
    }

    if (player->speed > 0x180000)
        player->speed = 0x180000;
}

void ProcessDebugMode(Player *player) {
    if (player->down || player->up || player->right || player->left) {
        if (player->speed < 0x100000) {
            player->speed += 0xC00;
            if (player->speed > 0x100000)
                player->speed = 0x100000;
        }
    } else {
        player->speed = 0;
    }

    if (GKeyDown.left)
        player->XPos -= player->speed;
    if (GKeyDown.right)
        player->XPos += player->speed;

    if (GKeyDown.up)
        player->YPos -= player->speed;
    if (GKeyDown.down)
        player->YPos += player->speed;
}

void ProcessPlayerAnimation(Player *player) {
    PlayerScript *script = &PlayerScriptList[player->type];
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
    if (player->animationSpeed)
        player->animationTimer += player->animationSpeed;
    else
        player->animationTimer += script->animations[player->animation].speed;
    if (player->animation != player->prevAnimation) {
        if (player->animation == ANI_JUMPING)
            player->YPos += (PlayerCBoxes[0].bottom[0] - PlayerCBoxes[1].bottom[0]) << 16;
        if (player->prevAnimation == ANI_JUMPING)
            player->YPos -= (PlayerCBoxes[0].bottom[0] - PlayerCBoxes[1].bottom[0]) << 16;
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