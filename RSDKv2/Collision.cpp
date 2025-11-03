#include "RetroEngine.hpp"
#include <stdlib.h>

int CollisionLeft   = 0;
int CollisionTop    = 0;
int CollisionRight  = 0;
int CollisionBottom = 0;

CollisionSensor sensors[6];

void FindFloorPosition(Player *player, CollisionSensor *sensor, int startY) {
    int c     = 0;
    int angle = sensor->angle;
    int tsm1  = (TILE_SIZE - 1);
    for (int i = 0; i < TILE_SIZE * 3; i += TILE_SIZE) {
        if (!sensor->collided) {
            int XPos   = sensor->XPos >> 16;
            int chunkX = XPos >> 7;
            int tileX  = (XPos & 0x7F) >> 4;
            int YPos   = (sensor->YPos >> 16) + i - TILE_SIZE;
            int chunkY = YPos >> 7;
            int tileY  = (YPos & 0x7F) >> 4;
            if (XPos > -1 && YPos > -1) {
                int tile = StageLayouts[0].tiles[chunkX + (chunkY << 8)] << 6;
                tile += tileX + (tileY << 3);
                int tileIndex = StageTiles.tileIndex[tile];
                if (StageTiles.collisionFlags[player->collisionPlane][tile] != SOLID_LRB
                    && StageTiles.collisionFlags[player->collisionPlane][tile] != SOLID_NONE) {
                    switch (StageTiles.direction[tile]) {
                        case FLIP_NONE: {
                            c = (XPos & tsm1) + (tileIndex << 4);
                            if (TileCollisions[player->collisionPlane].floorMasks[c] >= 0x40)
                                break;

                            sensor->YPos     = TileCollisions[player->collisionPlane].floorMasks[c] + (chunkY << 7) + (tileY << 4);
                            sensor->collided = true;
                            sensor->angle    = TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF;
                            break;
                        }
                        case FLIP_X: {
                            c = tsm1 - (XPos & tsm1) + (tileIndex << 4);
                            if (TileCollisions[player->collisionPlane].floorMasks[c] >= 0x40)
                                break;

                            sensor->YPos     = TileCollisions[player->collisionPlane].floorMasks[c] + (chunkY << 7) + (tileY << 4);
                            sensor->collided = true;
                            sensor->angle    = 0x100 - (TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF);
                            break;
                        }
                        case FLIP_Y: {
                            c = (XPos & 15) + (tileIndex << 4);
                            if (TileCollisions[player->collisionPlane].roofMasks[c] <= -0x40)
                                break;

                            sensor->YPos     = tsm1 - TileCollisions[player->collisionPlane].roofMasks[c] + (chunkY << 7) + (tileY << 4);
                            sensor->collided = true;
                            byte cAngle      = (TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF000000) >> 24;
                            sensor->angle    = (byte)(-0x80 - cAngle);
                            break;
                        }
                        case FLIP_XY: {
                            c = tsm1 - (XPos & tsm1) + (tileIndex << 4);
                            if (TileCollisions[player->collisionPlane].roofMasks[c] <= -0x40)
                                break;

                            sensor->YPos     = tsm1 - TileCollisions[player->collisionPlane].roofMasks[c] + (chunkY << 7) + (tileY << 4);
                            sensor->collided = true;
                            byte cAngle      = (TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF000000) >> 24;
                            sensor->angle    = 0x100 - (byte)(-0x80 - cAngle);
                            break;
                        }
                    }
                }

                if (sensor->collided) {
                    if (sensor->angle < 0)
                        sensor->angle += 0x100;

                    if (sensor->angle > 0xFF)
                        sensor->angle -= 0x100;

                    if ((abs(sensor->angle - angle) > 0x20) && (abs(sensor->angle - 0x100 - angle) > 0x20)
                        && (abs(sensor->angle + 0x100 - angle) > 0x20)) {
                        sensor->YPos     = startY << 16;
                        sensor->collided = false;
                        sensor->angle    = angle;
                        i                = TILE_SIZE * 3;
                    } else if (sensor->YPos - startY > (TILE_SIZE / 2)) {
                        sensor->YPos     = startY << 16;
                        sensor->collided = false;
                    } else if (sensor->YPos - startY < -(TILE_SIZE / 2)) {
                        sensor->YPos     = startY << 16;
                        sensor->collided = false;
                    }
                }
            }
        }
    }
}
void FindLWallPosition(Player *player, CollisionSensor *sensor, int startX) {
    int c     = 0;
    int angle = sensor->angle;
    int tsm1  = (TILE_SIZE - 1);
    for (int i = 0; i < TILE_SIZE * 3; i += TILE_SIZE) {
        if (!sensor->collided) {
            int XPos   = (sensor->XPos >> 16) + i - TILE_SIZE;
            int chunkX = XPos >> 7;
            int tileX  = (XPos & 0x7F) >> 4;
            int YPos   = sensor->YPos >> 16;
            int chunkY = YPos >> 7;
            int tileY  = (YPos & 0x7F) >> 4;
            if (XPos > -1 && YPos > -1) {
                int tile      = StageLayouts[0].tiles[chunkX + (chunkY << 8)] << 6;
                tile          = tile + tileX + (tileY << 3);
                int tileIndex = StageTiles.tileIndex[tile];
                if (StageTiles.collisionFlags[player->collisionPlane][tile] < SOLID_NONE) {
                    switch (StageTiles.direction[tile]) {
                        case FLIP_NONE: {
                            c = (YPos & tsm1) + (tileIndex << 4);
                            if (TileCollisions[player->collisionPlane].lWallMasks[c] >= 0x40)
                                break;

                            sensor->XPos     = TileCollisions[player->collisionPlane].lWallMasks[c] + (chunkX << 7) + (tileX << 4);
                            sensor->collided = true;
                            sensor->angle    = ((TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF00) >> 8);
                            break;
                        }
                        case FLIP_X: {
                            c = (YPos & tsm1) + (tileIndex << 4);
                            if (TileCollisions[player->collisionPlane].rWallMasks[c] <= -0x40)
                                break;

                            sensor->XPos     = tsm1 - TileCollisions[player->collisionPlane].rWallMasks[c] + (chunkX << 7) + (tileX << 4);
                            sensor->collided = true;
                            sensor->angle    = 0x100 - ((TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF0000) >> 16);
                            break;
                        }
                        case FLIP_Y: {
                            c = tsm1 - (YPos & tsm1) + (tileIndex << 4);
                            if (TileCollisions[player->collisionPlane].lWallMasks[c] >= 0x40)
                                break;

                            sensor->XPos     = TileCollisions[player->collisionPlane].lWallMasks[c] + (chunkX << 7) + (tileX << 4);
                            sensor->collided = true;
                            int cAngle       = (TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF00) >> 8;
                            sensor->angle    = (byte)(-0x80 - cAngle);
                            break;
                        }
                        case FLIP_XY: {
                            c = tsm1 - (YPos & tsm1) + (tileIndex << 4);
                            if (TileCollisions[player->collisionPlane].rWallMasks[c] <= -0x40)
                                break;

                            sensor->XPos     = tsm1 - TileCollisions[player->collisionPlane].rWallMasks[c] + (chunkX << 7) + (tileX << 4);
                            sensor->collided = true;
                            int cAngle       = (TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF0000) >> 16;
                            sensor->angle    = 0x100 - (byte)(-0x80 - cAngle);
                            break;
                        }
                    }
                }
                if (sensor->collided) {
                    if (sensor->angle < 0)
                        sensor->angle += 0x100;

                    if (sensor->angle > 0xFF)
                        sensor->angle -= 0x100;

                    if (abs(angle - sensor->angle) > 0x200) {
                        sensor->XPos     = startX << 16;
                        sensor->collided = false;
                        sensor->angle    = angle;
                        i                = TILE_SIZE * 3;
                    } else if (sensor->XPos - startX > TILE_SIZE / 2) {
                        sensor->XPos     = startX << 16;
                        sensor->collided = false;
                    } else if (sensor->XPos - startX < -(TILE_SIZE / 2)) {
                        sensor->XPos     = startX << 16;
                        sensor->collided = false;
                    }
                }
            }
        }
    }
}
void FindRoofPosition(Player *player, CollisionSensor *sensor, int startY) {
    int c     = 0;
    int angle = sensor->angle;
    int tsm1  = (TILE_SIZE - 1);
    for (int i = 0; i < TILE_SIZE * 3; i += TILE_SIZE) {
        if (!sensor->collided) {
            int XPos   = sensor->XPos >> 16;
            int chunkX = XPos >> 7;
            int tileX  = (XPos & 0x7F) >> 4;
            int YPos   = (sensor->YPos >> 16) + TILE_SIZE - i;
            int chunkY = YPos >> 7;
            int tileY  = (YPos & 0x7F) >> 4;
            if (XPos > -1 && YPos > -1) {
                int tile      = StageLayouts[0].tiles[chunkX + (chunkY << 8)] << 6;
                tile          = tile + tileX + (tileY << 3);
                int tileIndex = StageTiles.tileIndex[tile];
                if (StageTiles.collisionFlags[player->collisionPlane][tile] < SOLID_NONE) {
                    switch (StageTiles.direction[tile]) {
                        case FLIP_NONE: {
                            c = (XPos & tsm1) + (tileIndex << 4);
                            if (TileCollisions[player->collisionPlane].roofMasks[c] <= -0x40)
                                break;

                            sensor->YPos     = TileCollisions[player->collisionPlane].roofMasks[c] + (chunkY << 7) + (tileY << 4);
                            sensor->collided = true;
                            sensor->angle    = (TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF000000) >> 24;
                            break;
                        }
                        case FLIP_X: {
                            c = tsm1 - (XPos & tsm1) + (tileIndex << 4);
                            if (TileCollisions[player->collisionPlane].roofMasks[c] <= -0x40)
                                break;

                            sensor->YPos     = TileCollisions[player->collisionPlane].roofMasks[c] + (chunkY << 7) + (tileY << 4);
                            sensor->collided = true;
                            sensor->angle    = 0x100 - ((TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF000000) >> 24);
                            break;
                        }
                        case FLIP_Y: {
                            c = (XPos & tsm1) + (tileIndex << 4);
                            if (TileCollisions[player->collisionPlane].floorMasks[c] >= 0x40)
                                break;

                            sensor->YPos     = tsm1 - TileCollisions[player->collisionPlane].floorMasks[c] + (chunkY << 7) + (tileY << 4);
                            sensor->collided = true;
                            byte cAngle      = TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF;
                            sensor->angle    = (byte)(-0x80 - cAngle);
                            break;
                        }
                        case FLIP_XY: {
                            c = tsm1 - (XPos & tsm1) + (tileIndex << 4);
                            if (TileCollisions[player->collisionPlane].floorMasks[c] >= 0x40)
                                break;

                            sensor->YPos     = tsm1 - TileCollisions[player->collisionPlane].floorMasks[c] + (chunkY << 7) + (tileY << 4);
                            sensor->collided = true;
                            byte cAngle      = TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF;
                            sensor->angle    = 0x100 - (byte)(-0x80 - cAngle);
                            break;
                        }
                    }
                }

                if (sensor->collided) {
                    if (sensor->angle < 0)
                        sensor->angle += 0x100;

                    if (sensor->angle > 0xFF)
                        sensor->angle -= 0x100;

                    if (sensor->YPos - startY > tsm1) {
                        sensor->YPos     = startY << 16;
                        sensor->collided = false;
                    }
                    if (sensor->YPos - startY < -tsm1) {
                        sensor->YPos     = startY << 16;
                        sensor->collided = false;
                    }
                }
            }
        }
    }
}
void FindRWallPosition(Player *player, CollisionSensor *sensor, int startX) {
    int c;
    int angle = sensor->angle;
    int tsm1  = (TILE_SIZE - 1);
    for (int i = 0; i < TILE_SIZE * 3; i += TILE_SIZE) {
        if (!sensor->collided) {
            int XPos   = (sensor->XPos >> 16) + TILE_SIZE - i;
            int chunkX = XPos >> 7;
            int tileX  = (XPos & 0x7F) >> 4;
            int YPos   = sensor->YPos >> 16;
            int chunkY = YPos >> 7;
            int tileY  = (YPos & 0x7F) >> 4;
            if (XPos > -1 && YPos > -1) {
                int tile      = StageLayouts[0].tiles[chunkX + (chunkY << 8)] << 6;
                tile          = tile + tileX + (tileY << 3);
                int tileIndex = StageTiles.tileIndex[tile];
                if (StageTiles.collisionFlags[player->collisionPlane][tile] < SOLID_NONE) {
                    switch (StageTiles.direction[tile]) {
                        case FLIP_NONE: {
                            c = (YPos & tsm1) + (tileIndex << 4);
                            if (TileCollisions[player->collisionPlane].rWallMasks[c] <= -0x40)
                                break;

                            sensor->XPos     = TileCollisions[player->collisionPlane].rWallMasks[c] + (chunkX << 7) + (tileX << 4);
                            sensor->collided = true;
                            sensor->angle    = (TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF0000) >> 16;
                            break;
                        }
                        case FLIP_X: {
                            c = (YPos & tsm1) + (tileIndex << 4);
                            if (TileCollisions[player->collisionPlane].lWallMasks[c] >= 0x40)
                                break;

                            sensor->XPos     = tsm1 - TileCollisions[player->collisionPlane].lWallMasks[c] + (chunkX << 7) + (tileX << 4);
                            sensor->collided = true;
                            sensor->angle    = 0x100 - ((TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF00) >> 8);
                            break;
                        }
                        case FLIP_Y: {
                            c = tsm1 - (YPos & tsm1) + (tileIndex << 4);
                            if (TileCollisions[player->collisionPlane].rWallMasks[c] <= -0x40)
                                break;

                            sensor->XPos     = TileCollisions[player->collisionPlane].rWallMasks[c] + (chunkX << 7) + (tileX << 4);
                            sensor->collided = true;
                            int cAngle       = (TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF0000) >> 16;
                            sensor->angle    = (byte)(-0x80 - cAngle);
                            break;
                        }
                        case FLIP_XY: {
                            c = tsm1 - (YPos & tsm1) + (tileIndex << 4);
                            if (TileCollisions[player->collisionPlane].lWallMasks[c] >= 0x40)
                                break;

                            sensor->XPos     = tsm1 - TileCollisions[player->collisionPlane].lWallMasks[c] + (chunkX << 7) + (tileX << 4);
                            sensor->collided = true;
                            int cAngle       = (TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF00) >> 8;
                            sensor->angle    = 0x100 - (byte)(-0x80 - cAngle);
                            break;
                        }
                    }
                }
                if (sensor->collided) {
                    if (sensor->angle < 0)
                        sensor->angle += 0x100;

                    if (sensor->angle > 0xFF)
                        sensor->angle -= 0x100;

                    if (abs(sensor->angle - angle) > 0x20) {
                        sensor->XPos     = startX << 16;
                        sensor->collided = false;
                        sensor->angle    = angle;
                        i                = TILE_SIZE * 3;
                    } else if (sensor->XPos - startX > (TILE_SIZE / 2)) {
                        sensor->XPos     = startX >> 16;
                        sensor->collided = false;
                    } else if (sensor->XPos - startX < -(TILE_SIZE / 2)) {
                        sensor->XPos     = startX << 16;
                        sensor->collided = false;
                    }
                }
            }
        }
    }
}

void FloorCollision(Player *player, CollisionSensor *sensor) {
    int c;
    int startY = sensor->YPos >> 16;
    int tsm1   = (TILE_SIZE - 1);
    for (int i = 0; i < TILE_SIZE * 3; i += TILE_SIZE) {
        if (!sensor->collided) {
            int XPos   = sensor->XPos >> 16;
            int chunkX = XPos >> 7;
            int tileX  = (XPos & 0x7F) >> 4;
            int YPos   = (sensor->YPos >> 16) + i - TILE_SIZE;
            int chunkY = YPos >> 7;
            int tileY  = (YPos & 0x7F) >> 4;
            if (XPos > -1 && YPos > -1) {
                int tile = StageLayouts[0].tiles[chunkX + (chunkY << 8)] << 6;
                tile += tileX + (tileY << 3);
                int tileIndex = StageTiles.tileIndex[tile];
                if (StageTiles.collisionFlags[player->collisionPlane][tile] != SOLID_LRB
                    && StageTiles.collisionFlags[player->collisionPlane][tile] != SOLID_NONE) {
                    switch (StageTiles.direction[tile]) {
                        case FLIP_NONE: {
                            c = (XPos & tsm1) + (tileIndex << 4);
                            if ((YPos & tsm1) <= TileCollisions[player->collisionPlane].floorMasks[c] + i - TILE_SIZE
                                || TileCollisions[player->collisionPlane].floorMasks[c] >= tsm1)
                                break;

                            sensor->YPos     = TileCollisions[player->collisionPlane].floorMasks[c] + (chunkY << 7) + (tileY << 4);
                            sensor->collided = true;
                            sensor->angle    = TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF;
                            break;
                        }
                        case FLIP_X: {
                            c = tsm1 - (XPos & tsm1) + (tileIndex << 4);
                            if ((YPos & tsm1) <= TileCollisions[player->collisionPlane].floorMasks[c] + i - TILE_SIZE
                                || TileCollisions[player->collisionPlane].floorMasks[c] >= tsm1)
                                break;

                            sensor->YPos     = TileCollisions[player->collisionPlane].floorMasks[c] + (chunkY << 7) + (tileY << 4);
                            sensor->collided = true;
                            sensor->angle    = 0x100 - (TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF);
                            break;
                        }
                        case FLIP_Y: {
                            c = (XPos & tsm1) + (tileIndex << 4);
                            if ((YPos & tsm1) <= tsm1 - TileCollisions[player->collisionPlane].roofMasks[c] + i - TILE_SIZE)
                                break;

                            sensor->YPos     = tsm1 - TileCollisions[player->collisionPlane].roofMasks[c] + (chunkY << 7) + (tileY << 4);
                            sensor->collided = true;
                            int cAngle       = (TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF000000) >> 24;
                            sensor->angle    = (byte)(-0x80 - cAngle);
                            break;
                        }
                        case FLIP_XY: {
                            c = tsm1 - (XPos & tsm1) + (tileIndex << 4);
                            if ((YPos & tsm1) <= tsm1 - TileCollisions[player->collisionPlane].roofMasks[c] + i - TILE_SIZE)
                                break;

                            sensor->YPos     = tsm1 - TileCollisions[player->collisionPlane].roofMasks[c] + (chunkY << 7) + (tileY << 4);
                            sensor->collided = true;
                            int cAngle       = (TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF000000) >> 24;
                            sensor->angle    = 0x100 - (byte)(-0x80 - cAngle);
                            break;
                        }
                    }
                }

                if (sensor->collided) {
                    if (sensor->angle < 0)
                        sensor->angle += 0x100;

                    if (sensor->angle > 0xFF)
                        sensor->angle -= 0x100;

                    if (sensor->YPos - startY > (TILE_SIZE - 2)) {
                        sensor->YPos     = startY << 16;
                        sensor->collided = false;
                    } else if (sensor->YPos - startY < -(TILE_SIZE + 1)) {
                        sensor->YPos     = startY << 16;
                        sensor->collided = false;
                    }
                }
            }
        }
    }
}
void LWallCollision(Player *player, CollisionSensor *sensor) {
    int c;
    int startX = sensor->XPos >> 16;
    int tsm1   = (TILE_SIZE - 1);
    for (int i = 0; i < TILE_SIZE * 3; i += TILE_SIZE) {
        if (!sensor->collided) {
            int XPos   = (sensor->XPos >> 16) + i - TILE_SIZE;
            int chunkX = XPos >> 7;
            int tileX  = (XPos & 0x7F) >> 4;
            int YPos   = sensor->YPos >> 16;
            int chunkY = YPos >> 7;
            int tileY  = (YPos & 0x7F) >> 4;
            if (XPos > -1 && YPos > -1) {
                int tile = StageLayouts[0].tiles[chunkX + (chunkY << 8)] << 6;
                tile += tileX + (tileY << 3);
                int tileIndex = StageTiles.tileIndex[tile];
                if (StageTiles.collisionFlags[player->collisionPlane][tile] != SOLID_TOP
                    && StageTiles.collisionFlags[player->collisionPlane][tile] < SOLID_NONE) {
                    switch (StageTiles.direction[tile]) {
                        case FLIP_NONE: {
                            c = (YPos & tsm1) + (tileIndex << 4);
                            if ((XPos & tsm1) <= TileCollisions[player->collisionPlane].lWallMasks[c] + i - TILE_SIZE)
                                break;

                            sensor->XPos     = TileCollisions[player->collisionPlane].lWallMasks[c] + (chunkX << 7) + (tileX << 4);
                            sensor->collided = true;
                            break;
                        }
                        case FLIP_X: {
                            c = (YPos & tsm1) + (tileIndex << 4);
                            if ((XPos & tsm1) <= tsm1 - TileCollisions[player->collisionPlane].rWallMasks[c] + i - TILE_SIZE)
                                break;

                            sensor->XPos     = tsm1 - TileCollisions[player->collisionPlane].rWallMasks[c] + (chunkX << 7) + (tileX << 4);
                            sensor->collided = true;
                            break;
                        }
                        case FLIP_Y: {
                            c = tsm1 - (YPos & tsm1) + (tileIndex << 4);
                            if ((XPos & tsm1) <= TileCollisions[player->collisionPlane].lWallMasks[c] + i - TILE_SIZE)
                                break;

                            sensor->XPos     = TileCollisions[player->collisionPlane].lWallMasks[c] + (chunkX << 7) + (tileX << 4);
                            sensor->collided = true;
                            break;
                        }
                        case FLIP_XY: {
                            c = tsm1 - (YPos & tsm1) + (tileIndex << 4);
                            if ((XPos & tsm1) <= tsm1 - TileCollisions[player->collisionPlane].rWallMasks[c] + i - TILE_SIZE)
                                break;

                            sensor->XPos     = tsm1 - TileCollisions[player->collisionPlane].rWallMasks[c] + (chunkX << 7) + (tileX << 4);
                            sensor->collided = true;
                            break;
                        }
                    }
                }

                if (sensor->collided) {
                    if (sensor->XPos - startX > tsm1) {
                        sensor->XPos     = startX << 16;
                        sensor->collided = false;
                    } else if (sensor->XPos - startX < -tsm1) {
                        sensor->XPos     = startX << 16;
                        sensor->collided = false;
                    }
                }
            }
        }
    }
}
void RoofCollision(Player *player, CollisionSensor *sensor) {
    int c;
    int startY = sensor->YPos >> 16;
    int tsm1   = (TILE_SIZE - 1);
    for (int i = 0; i < TILE_SIZE * 3; i += TILE_SIZE) {
        if (!sensor->collided) {
            int XPos   = sensor->XPos >> 16;
            int chunkX = XPos >> 7;
            int tileX  = (XPos & 0x7F) >> 4;
            int YPos   = (sensor->YPos >> 16) + TILE_SIZE - i;
            int chunkY = YPos >> 7;
            int tileY  = (YPos & 0x7F) >> 4;
            if (XPos > -1 && YPos > -1) {
                int tile = StageLayouts[0].tiles[chunkX + (chunkY << 8)] << 6;
                tile += tileX + (tileY << 3);
                int tileIndex = StageTiles.tileIndex[tile];
                if (StageTiles.collisionFlags[player->collisionPlane][tile] != SOLID_TOP
                    && StageTiles.collisionFlags[player->collisionPlane][tile] < SOLID_NONE) {
                    switch (StageTiles.direction[tile]) {
                        case FLIP_NONE: {
                            c = (XPos & tsm1) + (tileIndex << 4);
                            if ((YPos & tsm1) >= TileCollisions[player->collisionPlane].roofMasks[c] + TILE_SIZE - i)
                                break;

                            sensor->YPos     = TileCollisions[player->collisionPlane].roofMasks[c] + (chunkY << 7) + (tileY << 4);
                            sensor->collided = true;
                            sensor->angle    = ((TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF000000) >> 24);
                            break;
                        }
                        case FLIP_X: {
                            c = tsm1 - (XPos & tsm1) + (tileIndex << 4);
                            if ((YPos & tsm1) >= TileCollisions[player->collisionPlane].roofMasks[c] + TILE_SIZE - i)
                                break;

                            sensor->YPos     = TileCollisions[player->collisionPlane].roofMasks[c] + (chunkY << 7) + (tileY << 4);
                            sensor->collided = true;
                            sensor->angle    = 0x100 - ((TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF000000) >> 24);
                            break;
                        }
                        case FLIP_Y: {
                            c = (XPos & tsm1) + (tileIndex << 4);
                            if ((YPos & tsm1) >= tsm1 - TileCollisions[player->collisionPlane].floorMasks[c] + TILE_SIZE - i)
                                break;

                            sensor->YPos     = tsm1 - TileCollisions[player->collisionPlane].floorMasks[c] + (chunkY << 7) + (tileY << 4);
                            sensor->collided = true;
                            sensor->angle    = (byte)(-0x80 - (TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF));
                            break;
                        }
                        case FLIP_XY: {
                            c = tsm1 - (XPos & tsm1) + (tileIndex << 4);
                            if ((YPos & tsm1) >= tsm1 - TileCollisions[player->collisionPlane].floorMasks[c] + TILE_SIZE - i)
                                break;

                            sensor->YPos     = tsm1 - TileCollisions[player->collisionPlane].floorMasks[c] + (chunkY << 7) + (tileY << 4);
                            sensor->collided = true;
                            sensor->angle    = 0x100 - (byte)(-0x80 - (TileCollisions[player->collisionPlane].angles[tileIndex] & 0xFF));
                            break;
                        }
                    }
                }

                if (sensor->collided) {
                    if (sensor->angle < 0)
                        sensor->angle += 0x100;

                    if (sensor->angle > 0xFF)
                        sensor->angle -= 0x100;

                    if (sensor->YPos - startY > (TILE_SIZE - 2)) {
                        sensor->YPos     = startY << 16;
                        sensor->collided = false;
                    } else if (sensor->YPos - startY < -(TILE_SIZE - 2)) {
                        sensor->YPos     = startY << 16;
                        sensor->collided = false;
                    }
                }
            }
        }
    }
}
void RWallCollision(Player *player, CollisionSensor *sensor) {
    int c;
    int startX = sensor->XPos >> 16;
    int tsm1   = (TILE_SIZE - 1);
    for (int i = 0; i < TILE_SIZE * 3; i += TILE_SIZE) {
        if (!sensor->collided) {
            int XPos   = (sensor->XPos >> 16) + TILE_SIZE - i;
            int chunkX = XPos >> 7;
            int tileX  = (XPos & 0x7F) >> 4;
            int YPos   = sensor->YPos >> 16;
            int chunkY = YPos >> 7;
            int tileY  = (YPos & 0x7F) >> 4;
            if (XPos > -1 && YPos > -1) {
                int tile = StageLayouts[0].tiles[chunkX + (chunkY << 8)] << 6;
                tile += tileX + (tileY << 3);
                int tileIndex = StageTiles.tileIndex[tile];
                if (StageTiles.collisionFlags[player->collisionPlane][tile] != SOLID_TOP
                    && StageTiles.collisionFlags[player->collisionPlane][tile] < SOLID_NONE) {
                    switch (StageTiles.direction[tile]) {
                        case FLIP_NONE: {
                            c = (YPos & tsm1) + (tileIndex << 4);
                            if ((XPos & tsm1) >= TileCollisions[player->collisionPlane].rWallMasks[c] + TILE_SIZE - i)
                                break;

                            sensor->XPos     = TileCollisions[player->collisionPlane].rWallMasks[c] + (chunkX << 7) + (tileX << 4);
                            sensor->collided = true;
                            break;
                        }
                        case FLIP_X: {
                            c = (YPos & tsm1) + (tileIndex << 4);
                            if ((XPos & tsm1) >= tsm1 - TileCollisions[player->collisionPlane].lWallMasks[c] + TILE_SIZE - i)
                                break;

                            sensor->XPos     = tsm1 - TileCollisions[player->collisionPlane].lWallMasks[c] + (chunkX << 7) + (tileX << 4);
                            sensor->collided = true;
                            break;
                        }
                        case FLIP_Y: {
                            c = tsm1 - (YPos & tsm1) + (tileIndex << 4);
                            if ((XPos & tsm1) >= TileCollisions[player->collisionPlane].rWallMasks[c] + TILE_SIZE - i)
                                break;

                            sensor->XPos     = TileCollisions[player->collisionPlane].rWallMasks[c] + (chunkX << 7) + (tileX << 4);
                            sensor->collided = true;
                            break;
                        }
                        case FLIP_XY: {
                            c = tsm1 - (YPos & tsm1) + (tileIndex << 4);
                            if ((XPos & tsm1) >= tsm1 - TileCollisions[player->collisionPlane].lWallMasks[c] + TILE_SIZE - i)
                                break;

                            sensor->XPos     = tsm1 - TileCollisions[player->collisionPlane].lWallMasks[c] + (chunkX << 7) + (tileX << 4);
                            sensor->collided = true;
                            break;
                        }
                    }
                }

                if (sensor->collided) {
                    if (sensor->XPos - startX > tsm1) {
                        sensor->XPos     = startX << 16;
                        sensor->collided = false;
                    } else if (sensor->XPos - startX < -tsm1) {
                        sensor->XPos     = startX << 16;
                        sensor->collided = false;
                    }
                }
            }
        }
    }
}

void ProcessTracedCollision(Player *player) {
    PlayerScript *script = &PlayerScriptList[PlayerNo];
    Hitbox *playerHitbox = GetPlayerCBox(script);
    CollisionLeft        = playerHitbox->left[0];
    CollisionTop         = playerHitbox->top[0];
    CollisionRight       = playerHitbox->right[0];
    CollisionBottom      = playerHitbox->bottom[0];

    byte movingDown  = 0;
    byte movingUp    = 0;
    byte movingLeft  = 0;
    byte movingRight = 0;

    if (player->XVelocity < 0) {
        movingRight = 0;
    } else {
        movingRight     = 1;
        sensors[0].YPos = ((CollisionTop + 4) << 16) + player->YPos;
        sensors[1].YPos = ((CollisionBottom - 4) << 16) + player->YPos;
        sensors[0].XPos = (CollisionRight << 16) + player->XPos;
        sensors[1].XPos = (CollisionRight << 16) + player->XPos;
    }
    if (player->XVelocity > 0) {
        movingLeft = 0;
    } else {
        movingLeft      = 1;
        sensors[2].YPos = ((CollisionTop + 4) << 16) + player->YPos;
        sensors[3].YPos = ((CollisionBottom - 4) << 16) + player->YPos;
        sensors[2].XPos = ((CollisionLeft - 1) << 16) + player->XPos;
        sensors[3].XPos = ((CollisionLeft - 1) << 16) + player->XPos;
    }
    sensors[4].XPos     = ((CollisionLeft + 1) << 16) + player->XPos;
    sensors[5].XPos     = ((CollisionRight - 2) << 16) + player->XPos;
    sensors[0].collided = false;
    sensors[1].collided = false;
    sensors[2].collided = false;
    sensors[3].collided = false;
    sensors[4].collided = false;
    sensors[5].collided = false;

    movingDown = 0;
    movingUp   = 0;
    if (player->YVelocity < 0) {
        movingUp        = 1;
        sensors[4].YPos = ((CollisionTop - 1) << 16) + player->YPos;
        sensors[5].YPos = ((CollisionTop - 1) << 16) + player->YPos;
    } else if (player->YVelocity > 0) {
        movingDown      = 1;
        sensors[4].YPos = (CollisionBottom << 16) + player->YPos;
        sensors[5].YPos = (CollisionBottom << 16) + player->YPos;
    }

    int xDif    = ((player->XVelocity + player->XPos) >> 16) - (player->XPos >> 16);
    int yDif    = ((player->YVelocity + player->YPos) >> 16) - (player->YPos >> 16);
    int absXDif = abs(xDif);
    int absYDif = abs(yDif);
    int cnt     = 1;
    int XVel    = player->XVelocity;
    int YVel    = player->YVelocity;

    if (absXDif || absYDif) {
        if (absXDif <= absYDif) {
            XVel = (xDif << 16) / absYDif;
            cnt  = absYDif;
            if (yDif >= 0)
                YVel = 0x10000;
            else
                YVel = -0x10000;
        } else {
            YVel = (yDif << 16) / absXDif;
            cnt  = absXDif;
            if (xDif >= 0)
                XVel = 0x10000;
            else
                XVel = -0x10000;
        }
    }

    while (cnt > 0) {
        cnt--;

        if (movingRight == 1) {
            for (int i = 0; i < 2; i++) {
                if (!sensors[i].collided) {
                    sensors[i].XPos += XVel;
                    sensors[i].YPos += YVel;
                    LWallCollision(player, &sensors[i]);
                }
            }
            if (sensors[0].collided || sensors[1].collided) {
                movingRight = 2;
                cnt         = 0;
                XVel        = 0;
            }
        }

        if (movingLeft == 1) {
            for (int i = 2; i < 4; i++) {
                if (!sensors[i].collided) {
                    sensors[i].XPos += XVel;
                    sensors[i].YPos += YVel;
                    RWallCollision(player, &sensors[i]);
                }
            }
            if (sensors[2].collided || sensors[3].collided) {
                movingLeft = 2;
                cnt        = 0;
                XVel       = 0;
            }
        }

        if (movingDown == 1) {
            for (int i = 4; i < 6; i++) {
                if (!sensors[i].collided) {
                    sensors[i].XPos += XVel;
                    sensors[i].YPos += YVel;
                    FloorCollision(player, &sensors[i]);
                }
            }
            if (sensors[4].collided || sensors[5].collided) {
                movingDown = 2;
                cnt        = 0;
            }
        } else if (movingUp == 1) {
            for (int i = 4; i < 6; i++) {
                if (!sensors[i].collided) {
                    sensors[i].XPos += XVel;
                    sensors[i].YPos += YVel;
                    RoofCollision(player, &sensors[i]);
                }
            }
            if (sensors[4].collided || sensors[5].collided) {
                movingUp = 2;
                cnt      = 0;
            }
        }
    }

    if (movingLeft == 2 || movingRight == 2) {
        if (movingRight == 2) {
            player->XVelocity = 0;
            player->speed     = 0;
            if (!sensors[0].collided || !sensors[1].collided) {
                if (sensors[0].collided) {
                    player->XPos = (sensors[0].XPos - CollisionRight) << 16;
                } else if (sensors[1].collided) {
                    player->XPos = (sensors[1].XPos - CollisionRight) << 16;
                }
            } else if (sensors[0].XPos >= sensors[1].XPos) {
                player->XPos = (sensors[1].XPos - CollisionRight) << 16;
            } else {
                player->XPos = (sensors[0].XPos - CollisionRight) << 16;
            }
        }
        if (movingLeft == 2) {
            player->XVelocity = 0;
            player->speed     = 0;
            if (!sensors[2].collided || !sensors[3].collided) {
                if (sensors[2].collided) {
                    player->XPos = (sensors[2].XPos - CollisionLeft + 1) << 16;
                } else if (sensors[3].collided) {
                    player->XPos = (sensors[3].XPos - CollisionLeft + 1) << 16;
                }
            } else if (sensors[2].XPos <= sensors[3].XPos) {
                player->XPos = (sensors[3].XPos - CollisionLeft + 1) << 16;
            } else {
                player->XPos = (sensors[2].XPos - CollisionLeft + 1) << 16;
            }
        }
    } else {
        player->XPos += player->XVelocity;
    }

    if (movingUp < 2 && movingDown < 2) {
        player->YPos += player->YVelocity;
        return;
    }

    if (movingDown == 2) {
        player->gravity = 0;
        if (sensors[4].collided && sensors[5].collided) {
            if (sensors[4].YPos >= sensors[5].YPos) {
                player->YPos  = (sensors[5].YPos - CollisionBottom) << 16;
                player->angle = sensors[5].angle;
            } else {
                player->YPos  = (sensors[4].YPos - CollisionBottom) << 16;
                player->angle = sensors[4].angle;
            }
        } else if (sensors[4].collided) {
            player->YPos  = (sensors[4].YPos - CollisionBottom) << 16;
            player->angle = sensors[4].angle;
        } else if (sensors[5].collided) {
            player->YPos  = (sensors[5].YPos - CollisionBottom) << 16;
            player->angle = sensors[5].angle;
        }
        if (player->angle > 0xA0 && player->angle < 0xE0 && player->collisionMode != CMODE_LWALL) {
            player->collisionMode = CMODE_LWALL;
        }
        if (player->angle > 0x20 && player->angle < 0x60 && player->collisionMode != CMODE_RWALL) {
            player->collisionMode = CMODE_RWALL;
        }
        player->rotation = player->angle;

        player->speed += (player->YVelocity * SinValue256[player->angle] >> 8);
        player->YVelocity = 0;
    }

    if (movingUp == 2) {
        player->YVelocity = 0;
        if (sensors[4].collided && sensors[5].collided) {
            if (sensors[4].YPos <= sensors[5].YPos) {
                player->YPos = (sensors[5].YPos - CollisionTop + 1) << 16;
            } else {
                player->YPos = (sensors[4].YPos - CollisionTop + 1) << 16;
            }
        } else if (sensors[4].collided) {
            player->YPos = (sensors[4].YPos - CollisionTop + 1) << 16;
        } else if (sensors[5].collided) {
            player->YPos = (sensors[5].YPos - CollisionTop + 1) << 16;
        }
    }
}
void ProcessPathGrip(Player *player) {
    int cos;
    int sin;
    sensors[4].XPos = player->XPos;
    sensors[4].YPos = player->YPos;
    for (int i = 0; i < 6; ++i) {
        sensors[i].angle    = player->angle;
        sensors[i].collided = false;
    }
    SetPathGripSensors(player);
    int absSpeed  = abs(player->speed);
    int checkDist = absSpeed >> 18;
    absSpeed &= 0x3FFFF;
    byte cMode = player->collisionMode;

    while (checkDist > -1) {
        if (checkDist >= 1) {
            cos = CosValue256[player->angle] << 10;
            sin = SinValue256[player->angle] << 10;
            checkDist--;
        } else {
            cos       = absSpeed * CosValue256[player->angle] >> 8;
            sin       = absSpeed * SinValue256[player->angle] >> 8;
            checkDist = -1;
        }

        if (player->speed < 0) {
            cos = -cos;
            sin = -sin;
        }

        sensors[0].collided = false;
        sensors[1].collided = false;
        sensors[2].collided = false;
        sensors[4].XPos += cos;
        sensors[4].YPos += sin;
        int tileDistance = -1;
        switch (player->collisionMode) {
            case CMODE_FLOOR: {
                for (int i = 0; i < 3; i++) {
                    sensors[i].XPos += cos;
                    sensors[i].YPos += sin;
                    FindFloorPosition(player, &sensors[i], sensors[i].YPos >> 16);
                }

                tileDistance = -1;
                for (int i = 0; i < 3; i++) {
                    if (tileDistance > -1) {
                        if (sensors[i].collided) {
                            if (sensors[i].YPos < sensors[tileDistance].YPos)
                                tileDistance = i;

                            if (sensors[i].YPos == sensors[tileDistance].YPos && (sensors[i].angle < 0x08 || sensors[i].angle > 0xF8))
                                tileDistance = i;
                        }
                    } else if (sensors[i].collided)
                        tileDistance = i;
                }

                if (tileDistance <= -1) {
                    checkDist = -1;
                } else {
                    sensors[0].YPos  = sensors[tileDistance].YPos << 16;
                    sensors[0].angle = sensors[tileDistance].angle;
                    sensors[1].YPos  = sensors[0].YPos;
                    sensors[1].angle = sensors[0].angle;
                    sensors[2].YPos  = sensors[0].YPos;
                    sensors[2].angle = sensors[0].angle;
                    sensors[3].YPos  = sensors[0].YPos - 0x40000;
                    sensors[3].angle = sensors[0].angle;
                    sensors[4].XPos  = sensors[1].XPos;
                    sensors[4].YPos  = sensors[0].YPos - (CollisionBottom << 16);
                }

                sensors[3].XPos += cos;
                if (player->speed > 0)
                    LWallCollision(player, &sensors[3]);

                if (player->speed < 0)
                    RWallCollision(player, &sensors[3]);

                if (sensors[0].angle > 0xA0 && sensors[0].angle < 0xE0 && player->collisionMode != CMODE_LWALL)
                    player->collisionMode = CMODE_LWALL;
                if (sensors[0].angle > 0x20 && sensors[0].angle < 0x60 && player->collisionMode != CMODE_RWALL)
                    player->collisionMode = CMODE_RWALL;
                break;
            }
            case CMODE_LWALL: {
                for (int i = 0; i < 3; i++) {
                    sensors[i].XPos += cos;
                    sensors[i].YPos += sin;
                    FindLWallPosition(player, &sensors[i], sensors[i].XPos >> 16);
                }

                tileDistance = -1;
                for (int i = 0; i < 3; i++) {
                    if (tileDistance > -1) {
                        if (sensors[i].XPos < sensors[tileDistance].XPos && sensors[i].collided) {
                            tileDistance = i;
                        }
                    } else if (sensors[i].collided) {
                        tileDistance = i;
                    }
                }

                if (tileDistance <= -1) {
                    checkDist = -1;
                } else {
                    sensors[0].XPos  = sensors[tileDistance].XPos << 16;
                    sensors[0].angle = sensors[tileDistance].angle;
                    sensors[1].XPos  = sensors[0].XPos;
                    sensors[1].angle = sensors[0].angle;
                    sensors[2].XPos  = sensors[0].XPos;
                    sensors[2].angle = sensors[0].angle;
                    sensors[4].YPos  = sensors[1].YPos;
                    sensors[4].XPos  = sensors[1].XPos - (CollisionRight << 16);
                }

                if ((sensors[0].angle < 0x20 || sensors[0].angle > 0xE0) && player->collisionMode != CMODE_FLOOR)
                    player->collisionMode = CMODE_FLOOR;
                if (sensors[0].angle > 0x60 && sensors[0].angle < 0xA0 && player->collisionMode != CMODE_ROOF)
                    player->collisionMode = CMODE_ROOF;
                break;
            }
            case CMODE_ROOF: {
                for (int i = 0; i < 3; i++) {
                    sensors[i].XPos += cos;
                    sensors[i].YPos += sin;
                    FindRoofPosition(player, &sensors[i], sensors[i].YPos >> 16);
                }

                tileDistance = -1;
                for (int i = 0; i < 3; i++) {
                    if (tileDistance > -1) {
                        if (sensors[i].YPos > sensors[tileDistance].YPos && sensors[i].collided) {
                            tileDistance = i;
                        }
                    } else if (sensors[i].collided) {
                        tileDistance = i;
                    }
                }

                if (tileDistance <= -1) {
                    checkDist = -1;
                } else {
                    sensors[0].YPos  = sensors[tileDistance].YPos << 16;
                    sensors[0].angle = sensors[tileDistance].angle;
                    sensors[1].YPos  = sensors[0].YPos;
                    sensors[1].angle = sensors[0].angle;
                    sensors[2].YPos  = sensors[0].YPos;
                    sensors[2].angle = sensors[0].angle;
                    sensors[3].YPos  = sensors[0].YPos + 0x40000;
                    sensors[3].angle = sensors[0].angle;
                    sensors[4].XPos  = sensors[1].XPos;
                    sensors[4].YPos  = sensors[0].YPos - ((CollisionTop - 1) << 16);
                }

                sensors[3].XPos += cos;
                if (player->speed > 0)
                    RWallCollision(player, &sensors[3]);
                if (player->speed < 0)
                    LWallCollision(player, &sensors[3]);
                if (sensors[0].angle > 0xA0 && sensors[0].angle < 0xE0 && player->collisionMode != CMODE_LWALL)
                    player->collisionMode = CMODE_LWALL;
                if (sensors[0].angle > 0x20 && sensors[0].angle < 0x60 && player->collisionMode != CMODE_RWALL)
                    player->collisionMode = CMODE_RWALL;
                break;
            }
            case CMODE_RWALL: {
                for (int i = 0; i < 3; i++) {
                    sensors[i].XPos += cos;
                    sensors[i].YPos += sin;
                    FindRWallPosition(player, &sensors[i], sensors[i].XPos >> 16);
                }

                tileDistance = -1;
                for (int i = 0; i < 3; i++) {
                    if (tileDistance > -1) {
                        if (sensors[i].XPos > sensors[tileDistance].XPos && sensors[i].collided) {
                            tileDistance = i;
                        }
                    } else if (sensors[i].collided) {
                        tileDistance = i;
                    }
                }

                if (tileDistance <= -1) {
                    checkDist = -1;
                } else {
                    sensors[0].XPos  = sensors[tileDistance].XPos << 16;
                    sensors[0].angle = sensors[tileDistance].angle;
                    sensors[1].XPos  = sensors[0].XPos;
                    sensors[1].angle = sensors[0].angle;
                    sensors[2].XPos  = sensors[0].XPos;
                    sensors[2].angle = sensors[0].angle;
                    sensors[4].YPos  = sensors[1].YPos;
                    sensors[4].XPos  = sensors[1].XPos - ((CollisionLeft - 1) << 16);
                }

                if ((sensors[0].angle < 0x20 || sensors[0].angle > 0xE0) && player->collisionMode != CMODE_FLOOR)
                    player->collisionMode = CMODE_FLOOR;
                if (sensors[0].angle > 0x60 && sensors[0].angle < 0xA0 && player->collisionMode != CMODE_ROOF)
                    player->collisionMode = CMODE_ROOF;
                break;
            }
        }
        if (tileDistance > -1)
            player->angle = sensors[0].angle;

        if (!sensors[3].collided)
            SetPathGripSensors(player);
        else
            checkDist = -2;
    }

    switch (cMode) {
        case CMODE_FLOOR: {
            if (sensors[0].collided || sensors[1].collided || sensors[2].collided) {
                player->angle       = sensors[0].angle;
                player->rotation    = player->angle;
                player->flailing[0] = sensors[0].collided;
                player->flailing[1] = sensors[1].collided;
                player->flailing[2] = sensors[2].collided;
                if (!sensors[3].collided) {
                    player->pushing = 0;
                    player->XPos    = sensors[4].XPos;
                } else {
                    if (player->speed > 0)
                        player->XPos = (sensors[3].XPos - CollisionRight) << 16;

                    if (player->speed < 0)
                        player->XPos = (sensors[3].XPos - CollisionLeft + 1) << 16;

                    player->speed = 0;
                    if ((player->left || player->right) && player->pushing < 2)
                        player->pushing++;
                }
                player->YPos = sensors[4].YPos;
            } else {
                player->gravity       = 1;
                player->collisionMode = CMODE_FLOOR;
                player->XVelocity     = CosValue256[player->angle] * player->speed >> 8;
                player->YVelocity     = SinValue256[player->angle] * player->speed >> 8;

                player->speed = player->XVelocity;
                player->angle = 0;
                if (!sensors[3].collided) {
                    player->pushing = 0;
                    player->XPos += player->XVelocity;
                } else {
                    if (player->speed > 0)
                        player->XPos = (sensors[3].XPos - CollisionRight) << 16;
                    if (player->speed < 0)
                        player->XPos = (sensors[3].XPos - CollisionLeft + 1) << 16;

                    player->speed = 0;
                    if ((player->left || player->right) && player->pushing < 2)
                        player->pushing++;
                }
                player->YPos += player->YVelocity;
            }
            break;
        }
        case CMODE_LWALL: {
            if (!sensors[0].collided && !sensors[1].collided && !sensors[2].collided) {
                player->gravity       = 1;
                player->collisionMode = CMODE_FLOOR;
                player->XVelocity     = CosValue256[player->angle] * player->speed >> 8;
                player->YVelocity     = SinValue256[player->angle] * player->speed >> 8;
                player->speed         = player->XVelocity;
                player->angle         = 0;
            } else if (player->speed >= 0x20000 || player->speed <= -1) {
                player->angle    = sensors[0].angle;
                player->rotation = player->angle;
            } else {
                player->gravity       = 1;
                player->angle         = 0;
                player->collisionMode = CMODE_FLOOR;
                player->speed         = player->XVelocity;
            }

            player->XPos = sensors[4].XPos;
            player->YPos = sensors[4].YPos;
            break;
        }
        case CMODE_ROOF: {
            if (!sensors[0].collided && !sensors[1].collided && !sensors[2].collided) {
                player->gravity       = 1;
                player->collisionMode = CMODE_FLOOR;
                player->XVelocity     = CosValue256[player->angle] * player->speed >> 8;
                player->YVelocity     = SinValue256[player->angle] * player->speed >> 8;
                player->angle         = 0;
                player->speed         = player->XVelocity;
            } else if (player->speed <= -0x20000 || player->speed >= 0x20000) {
                player->angle    = sensors[0].angle;
                player->rotation = player->angle;
            } else {
                player->gravity       = 1;
                player->angle         = 0;
                player->collisionMode = CMODE_FLOOR;
                player->speed         = player->XVelocity;
            }

            if (!sensors[3].collided) {
                player->XPos = sensors[4].XPos;
            } else {
                if (player->speed > 0)
                    player->XPos = (sensors[3].XPos - CollisionRight) << 16;

                if (player->speed < 0)
                    player->XPos = (sensors[3].XPos - CollisionLeft + 1) << 16;
                player->speed = 0;
            }
            player->YPos = sensors[4].YPos;
            break;
        }
        case CMODE_RWALL: {
            if (!sensors[0].collided && !sensors[1].collided && !sensors[2].collided) {
                player->gravity       = 1;
                player->collisionMode = CMODE_FLOOR;
                player->XVelocity     = CosValue256[player->angle] * player->speed >> 8;
                player->YVelocity     = SinValue256[player->angle] * player->speed >> 8;
                player->speed         = player->XVelocity;
                player->angle         = 0;
            } else if (player->speed <= -0x20000 || player->speed >= 1) {
                player->angle    = sensors[0].angle;
                player->rotation = player->angle;
            } else {
                player->gravity       = 1;
                player->angle         = 0;
                player->collisionMode = CMODE_FLOOR;
                player->speed         = player->XVelocity;
            }

            player->XPos = sensors[4].XPos;
            player->YPos = sensors[4].YPos;
            break;
        }
        default: break;
    }
}

void SetPathGripSensors(Player *player) {
    PlayerScript *script = &PlayerScriptList[PlayerNo];
    Hitbox *playerHitbox = GetPlayerCBox(script);
    switch (player->collisionMode) {
        case CMODE_FLOOR: {
            CollisionLeft   = playerHitbox->left[0];
            CollisionTop    = playerHitbox->top[0];
            CollisionRight  = playerHitbox->right[0];
            CollisionBottom = playerHitbox->bottom[0];
            sensors[0].YPos = sensors[4].YPos + (CollisionBottom << 16);
            sensors[1].YPos = sensors[0].YPos;
            sensors[2].YPos = sensors[0].YPos;
            sensors[3].YPos = sensors[4].YPos + 0x40000;
            sensors[0].XPos = sensors[4].XPos + ((playerHitbox->left[1] - 1) << 16);
            sensors[1].XPos = sensors[4].XPos;
            sensors[2].XPos = sensors[4].XPos + (playerHitbox->right[1] << 16);
            if (player->speed > 0) {
                sensors[3].XPos = sensors[4].XPos + ((CollisionRight + 1) << 16);
                return;
            }
            sensors[3].XPos = sensors[4].XPos + ((CollisionLeft - 1) << 16);
            return;
        }
        case CMODE_LWALL: {
            CollisionLeft   = playerHitbox->left[2];
            CollisionTop    = playerHitbox->top[2];
            CollisionRight  = playerHitbox->right[2];
            CollisionBottom = playerHitbox->bottom[2];
            sensors[0].XPos = sensors[4].XPos + (CollisionRight << 16);
            sensors[1].XPos = sensors[0].XPos;
            sensors[2].XPos = sensors[0].XPos;
            sensors[3].XPos = sensors[4].XPos + 0x40000;
            sensors[0].YPos = sensors[4].YPos + ((playerHitbox->top[3] - 1) << 16);
            sensors[1].YPos = sensors[4].YPos;
            sensors[2].YPos = sensors[4].YPos + (playerHitbox->bottom[3] << 16);
            if (player->speed > 0) {
                sensors[3].YPos = sensors[4].YPos + (CollisionTop << 16);
                return;
            }
            sensors[3].YPos = sensors[4].YPos + ((CollisionBottom - 1) << 16);
            return;
        }
        case CMODE_ROOF: {
            CollisionLeft   = playerHitbox->left[4];
            CollisionTop    = playerHitbox->top[4];
            CollisionRight  = playerHitbox->right[4];
            CollisionBottom = playerHitbox->bottom[4];
            sensors[0].YPos = sensors[4].YPos + ((CollisionTop - 1) << 16);
            sensors[1].YPos = sensors[0].YPos;
            sensors[2].YPos = sensors[0].YPos;
            sensors[3].YPos = sensors[4].YPos - 0x40000;
            sensors[0].XPos = sensors[4].XPos + ((playerHitbox->left[5] - 1) << 16);
            sensors[1].XPos = sensors[4].XPos;
            sensors[2].XPos = sensors[4].XPos + (playerHitbox->right[5] << 16);
            if (player->speed < 0) {
                sensors[3].XPos = sensors[4].XPos + ((CollisionRight + 1) << 16);
                return;
            }
            sensors[3].XPos = sensors[4].XPos + ((CollisionLeft - 1) << 16);
            return;
        }
        case CMODE_RWALL: {
            CollisionLeft   = playerHitbox->left[6];
            CollisionTop    = playerHitbox->top[6];
            CollisionRight  = playerHitbox->right[6];
            CollisionBottom = playerHitbox->bottom[6];
            sensors[0].XPos = sensors[4].XPos + ((CollisionLeft - 1) << 16);
            sensors[1].XPos = sensors[0].XPos;
            sensors[2].XPos = sensors[0].XPos;
            sensors[3].XPos = sensors[4].XPos - 0x40000;
            sensors[0].YPos = sensors[4].YPos + ((playerHitbox->top[7] - 1) << 16);
            sensors[1].YPos = sensors[4].YPos;
            sensors[2].YPos = sensors[4].YPos + (playerHitbox->bottom[7] << 16);
            if (player->speed > 0) {
                sensors[3].YPos = sensors[4].YPos + (CollisionBottom << 16);
                return;
            }
            sensors[3].YPos = sensors[4].YPos + ((CollisionTop - 1) << 16);
            return;
        }
        default: return;
    }
}

void ProcessPlayerTileCollisions(Player *player) {
    player->flailing[0] = 0;
    player->flailing[1] = 0;
    player->flailing[2] = 0;
    if (player->gravity == 1)
        ProcessTracedCollision(player);
    else
        ProcessPathGrip(player);
}

void ObjectFloorCollision(int xOffset, int yOffset, int cPath) {
    ScriptEng.checkResult = false;
    Entity *entity        = &ObjectEntityList[ObjectLoop];
    int c                 = 0;
    int XPos              = (entity->XPos >> 16) + xOffset;
    int YPos              = (entity->YPos >> 16) + yOffset;
    if (XPos > 0 && XPos < StageLayouts[0].xsize << 7 && YPos > 0 && YPos < StageLayouts[0].ysize << 7) {
        int chunkX    = XPos >> 7;
        int tileX     = (XPos & 0x7F) >> 4;
        int chunkY    = YPos >> 7;
        int tileY     = (YPos & 0x7F) >> 4;
        int chunk     = (StageLayouts[0].tiles[chunkX + (chunkY << 8)] << 6) + tileX + (tileY << 3);
        int tileIndex = StageTiles.tileIndex[chunk];
        if (StageTiles.collisionFlags[cPath][chunk] != SOLID_LRB && StageTiles.collisionFlags[cPath][chunk] != SOLID_NONE) {
            switch (StageTiles.direction[chunk]) {
                case 0: {
                    c = (XPos & 15) + (tileIndex << 4);
                    if ((YPos & 15) <= TileCollisions[cPath].floorMasks[c]) {
                        break;
                    }
                    YPos                  = TileCollisions[cPath].floorMasks[c] + (chunkY << 7) + (tileY << 4);
                    ScriptEng.checkResult = true;
                    break;
                }
                case 1: {
                    c = 15 - (XPos & 15) + (tileIndex << 4);
                    if ((YPos & 15) <= TileCollisions[cPath].floorMasks[c]) {
                        break;
                    }
                    YPos                  = TileCollisions[cPath].floorMasks[c] + (chunkY << 7) + (tileY << 4);
                    ScriptEng.checkResult = true;
                    break;
                }
                case 2: {
                    c = (XPos & 15) + (tileIndex << 4);
                    if ((YPos & 15) <= 15 - TileCollisions[cPath].roofMasks[c]) {
                        break;
                    }
                    YPos                  = 15 - TileCollisions[cPath].roofMasks[c] + (chunkY << 7) + (tileY << 4);
                    ScriptEng.checkResult = true;
                    break;
                }
                case 3: {
                    c = 15 - (XPos & 15) + (tileIndex << 4);
                    if ((YPos & 15) <= 15 - TileCollisions[cPath].roofMasks[c]) {
                        break;
                    }
                    YPos                  = 15 - TileCollisions[cPath].roofMasks[c] + (chunkY << 7) + (tileY << 4);
                    ScriptEng.checkResult = true;
                    break;
                }
            }
        }
        if (ScriptEng.checkResult) {
            entity->YPos = (YPos - yOffset) << 16;
        }
    }
}

void ObjectFloorGrip(int xOffset, int yOffset, int cPath) {
    int c;
    ScriptEng.checkResult = false;
    Entity *entity        = &ObjectEntityList[ObjectLoop];
    int XPos              = (entity->XPos >> 16) + xOffset;
    int YPos              = (entity->YPos >> 16) + yOffset;
    int chunkX            = YPos;
    YPos                  = YPos - 16;
    for (int i = 3; i > 0; i--) {
        if (XPos > 0 && XPos < StageLayouts[0].xsize << 7 && YPos > 0 && YPos < StageLayouts[0].ysize << 7 && !ScriptEng.checkResult) {
            int chunkX    = XPos >> 7;
            int tileX     = (XPos & 0x7F) >> 4;
            int chunkY    = YPos >> 7;
            int tileY     = (YPos & 0x7F) >> 4;
            int chunk     = (StageLayouts[0].tiles[chunkX + (chunkY << 8)] << 6) + tileX + (tileY << 3);
            int tileIndex = StageTiles.tileIndex[chunk];
            if (StageTiles.collisionFlags[cPath][chunk] != SOLID_LRB && StageTiles.collisionFlags[cPath][chunk] != SOLID_NONE) {
                switch (StageTiles.direction[chunk]) {
                    case 0: {
                        c = (XPos & 15) + (tileIndex << 4);
                        if (TileCollisions[cPath].floorMasks[c] >= 64) {
                            break;
                        }
                        entity->YPos          = TileCollisions[cPath].floorMasks[c] + (chunkY << 7) + (tileY << 4);
                        ScriptEng.checkResult = true;
                        break;
                    }
                    case 1: {
                        c = 15 - (XPos & 15) + (tileIndex << 4);
                        if (TileCollisions[cPath].floorMasks[c] >= 64) {
                            break;
                        }
                        entity->YPos          = TileCollisions[cPath].floorMasks[c] + (chunkY << 7) + (tileY << 4);
                        ScriptEng.checkResult = true;
                        break;
                    }
                    case 2: {
                        c = (XPos & 15) + (tileIndex << 4);
                        if (TileCollisions[cPath].roofMasks[c] <= -64) {
                            break;
                        }
                        entity->YPos          = 15 - TileCollisions[cPath].roofMasks[c] + (chunkY << 7) + (tileY << 4);
                        ScriptEng.checkResult = true;
                        break;
                    }
                    case 3: {
                        c = 15 - (XPos & 15) + (tileIndex << 4);
                        if (TileCollisions[cPath].roofMasks[c] <= -64) {
                            break;
                        }
                        entity->YPos          = 15 - TileCollisions[cPath].roofMasks[c] + (chunkY << 7) + (tileY << 4);
                        ScriptEng.checkResult = true;
                        break;
                    }
                }
            }
        }
        YPos += 16;
    }
    if (ScriptEng.checkResult) {
        if (abs(entity->YPos - chunkX) < 16) {
            entity->YPos = (entity->YPos - yOffset) << 16;
            return;
        }
        entity->YPos          = (chunkX - yOffset) << 16;
        ScriptEng.checkResult = false;
    }
}

void BasicCollision(int left, int top, int right, int bottom) {
    Player *player       = &PlayerList[PlayerNo];
    PlayerScript *script = &PlayerScriptList[PlayerNo];
    Hitbox *playerHitbox = GetPlayerCBox(script);

    CollisionLeft   = player->XPos >> 16;
    CollisionTop    = player->YPos >> 16;
    CollisionRight  = CollisionLeft;
    CollisionBottom = CollisionTop;
    CollisionLeft += playerHitbox->left[0];
    CollisionTop += playerHitbox->top[0];
    CollisionRight += playerHitbox->right[0];
    CollisionBottom += playerHitbox->bottom[0];
    ScriptEng.checkResult = CollisionRight > left && CollisionLeft < right && CollisionBottom > top && CollisionTop < bottom;
}
void BoxCollision(int left, int top, int right, int bottom) {
    Player *player       = &PlayerList[PlayerNo];
    PlayerScript *script = &PlayerScriptList[PlayerNo];
    Hitbox *playerHitbox = GetPlayerCBox(script);

    CollisionLeft         = playerHitbox->left[0];
    CollisionTop          = playerHitbox->top[0];
    CollisionRight        = playerHitbox->right[0];
    CollisionBottom       = playerHitbox->bottom[0];
    ScriptEng.checkResult = false;

    int spd = 0;
    switch (player->collisionMode) {
        case CMODE_FLOOR:
        case CMODE_ROOF:
            if (player->XVelocity)
                spd = abs(player->XVelocity);
            else
                spd = abs(player->speed);
            break;
        case CMODE_LWALL:
        case CMODE_RWALL: spd = abs(player->XVelocity); break;
        default: break;
    }
    if (spd <= abs(player->YVelocity)) {
        sensors[0].collided = false;
        sensors[1].collided = false;
        sensors[2].collided = false;
        sensors[0].XPos     = player->XPos + ((CollisionLeft + 2) << 16);
        sensors[1].XPos     = player->XPos;
        sensors[2].XPos     = player->XPos + ((CollisionRight - 2) << 16);
        sensors[0].YPos     = player->YPos + (CollisionBottom << 16);
        sensors[1].YPos     = sensors[0].YPos;
        sensors[2].YPos     = sensors[0].YPos;
        if (player->YVelocity > -1) {
            for (int i = 0; i < 3; ++i) {
                if (sensors[i].XPos > left && sensors[i].XPos < right && sensors[i].YPos >= top && player->YPos - player->YVelocity < top) {
                    sensors[i].collided = true;
                    player->flailing[i] = true;
                }
            }
        }
        if (sensors[2].collided || sensors[1].collided || sensors[0].collided) {
            if (!player->gravity && (player->collisionMode == CMODE_RWALL || player->collisionMode == CMODE_LWALL)) {
                player->XVelocity = 0;
                player->speed     = 0;
            }
            player->YPos          = top - (CollisionBottom << 16);
            player->gravity       = 0;
            player->YVelocity     = 0;
            player->angle         = 0;
            player->rotation      = 0;
            ScriptEng.checkResult = true;
        } else {
            sensors[0].collided = false;
            sensors[1].collided = false;
            sensors[0].XPos     = player->XPos + ((CollisionLeft + 2) << 16);
            sensors[1].XPos     = player->XPos + ((CollisionRight - 2) << 16);
            sensors[0].YPos     = player->YPos + (CollisionTop << 16);
            sensors[1].YPos     = sensors[0].YPos;
            for (int i = 0; i < 2; ++i) {
                if (sensors[i].XPos > left && sensors[i].XPos < right && sensors[i].YPos <= bottom && player->YPos - player->YVelocity > bottom) {
                    sensors[i].collided = true;
                }
            }
            if (sensors[1].collided || sensors[0].collided) {
                if (player->gravity == 1) {
                    player->YPos = bottom - (CollisionTop << 16);
                }
                if (player->YVelocity < 1)
                    player->YVelocity = 0;
                ScriptEng.checkResult = 4;
            } else {
                sensors[0].collided = false;
                sensors[1].collided = false;
                sensors[0].XPos     = player->XPos + (CollisionRight << 16);
                sensors[1].XPos     = sensors[0].XPos;
                sensors[0].YPos     = player->YPos - 0x20000;
                sensors[1].YPos     = player->YPos + 0x80000;
                for (int i = 0; i < 2; ++i) {
                    if (sensors[i].XPos >= left && player->XPos - player->XVelocity < left && sensors[1].YPos > top && sensors[0].YPos < bottom) {
                        sensors[i].collided = true;
                    }
                }
                if (sensors[1].collided || sensors[0].collided) {
                    player->XPos = left - (CollisionRight << 16);
                    if (player->XVelocity > 0) {
                        if (!player->direction)
                            player->pushing = 2;
                        player->XVelocity = 0;
                        player->speed     = 0;
                    }
                    ScriptEng.checkResult = 2;
                } else {
                    sensors[0].collided = false;
                    sensors[1].collided = false;
                    sensors[0].XPos     = player->XPos + (CollisionLeft << 16);
                    sensors[1].XPos     = sensors[0].XPos;
                    sensors[0].YPos     = player->YPos - 0x20000;
                    sensors[1].YPos     = player->YPos + 0x80000;
                    for (int i = 0; i < 2; ++i) {
                        if (sensors[i].XPos <= right && player->XPos - player->XVelocity > right && sensors[1].YPos > top
                            && sensors[0].YPos < bottom) {
                            sensors[i].collided = true;
                        }
                    }

                    if (sensors[1].collided || (sensors[0].collided)) {
                        player->XPos = right - (CollisionLeft << 16);
                        if (player->XVelocity < 0) {
                            if (player->direction == FLIP_X)
                                player->pushing = 2;
                            player->XVelocity = 0;
                            player->speed     = 0;
                        }
                        ScriptEng.checkResult = 3;
                    }
                }
            }
        }
    } else {
        sensors[0].collided = false;
        sensors[1].collided = false;
        sensors[0].XPos     = player->XPos + (CollisionRight << 16);
        sensors[1].XPos     = sensors[0].XPos;
        sensors[0].YPos     = player->YPos - 0x20000;
        sensors[1].YPos     = player->YPos + 0x80000;
        for (int i = 0; i < 2; ++i) {
            if (sensors[i].XPos >= left && player->XPos - player->XVelocity < left && sensors[1].YPos > top && sensors[0].YPos < bottom) {
                sensors[i].collided = true;
            }
        }
        if (sensors[1].collided || sensors[0].collided) {
            player->XPos = left - (CollisionRight << 16);
            if (player->XVelocity > 0) {
                if (!player->direction)
                    player->pushing = 2;
                player->XVelocity = 0;
                player->speed     = 0;
            }
            ScriptEng.checkResult = 2;
        } else {
            sensors[0].collided = false;
            sensors[1].collided = false;
            sensors[0].XPos     = player->XPos + (CollisionLeft << 16);
            sensors[1].XPos     = sensors[0].XPos;
            sensors[0].YPos     = player->YPos - 0x20000;
            sensors[1].YPos     = player->YPos + 0x80000;
            for (int i = 0; i < 2; ++i) {
                if (sensors[i].XPos <= right && player->XPos - player->XVelocity > right && sensors[1].YPos > top && sensors[0].YPos < bottom) {
                    sensors[i].collided = true;
                }
            }
            if (sensors[1].collided || sensors[0].collided) {
                player->XPos = right - (CollisionLeft << 16);
                if (player->XVelocity < 0) {
                    if (player->direction == FLIP_X) {
                        player->pushing = 2;
                    }
                    player->XVelocity = 0;
                    player->speed     = 0;
                }
                ScriptEng.checkResult = 3;
            } else {
                sensors[0].collided = false;
                sensors[1].collided = false;
                sensors[2].collided = false;
                sensors[0].XPos     = player->XPos + ((CollisionLeft + 2) << 16);
                sensors[1].XPos     = player->XPos;
                sensors[2].XPos     = player->XPos + ((CollisionRight - 2) << 16);
                sensors[0].YPos     = player->YPos + (CollisionBottom << 16);
                sensors[1].YPos     = sensors[0].YPos;
                sensors[2].YPos     = sensors[0].YPos;
                if (player->YVelocity > -1) {
                    for (int i = 0; i < 3; ++i) {
                        if (sensors[i].XPos > left && sensors[i].XPos < right && sensors[i].YPos >= top && player->YPos - player->YVelocity < top) {
                            sensors[i].collided = true;
                            player->flailing[i] = true;
                        }
                    }
                }
                if (sensors[2].collided || sensors[1].collided || sensors[0].collided) {
                    if (!player->gravity && (player->collisionMode == CMODE_RWALL || player->collisionMode == CMODE_LWALL)) {
                        player->XVelocity = 0;
                        player->speed     = 0;
                    }
                    player->YPos          = top - (CollisionBottom << 16);
                    player->gravity       = 0;
                    player->YVelocity     = 0;
                    player->angle         = 0;
                    player->rotation      = 0;
                    ScriptEng.checkResult = true;
                } else {
                    sensors[0].collided = false;
                    sensors[1].collided = false;
                    sensors[0].XPos     = player->XPos + ((CollisionLeft + 2) << 16);
                    sensors[1].XPos     = player->XPos + ((CollisionRight - 2) << 16);
                    sensors[0].YPos     = player->YPos + (CollisionTop << 16);
                    sensors[1].YPos     = sensors[0].YPos;
                    for (int i = 0; i < 2; ++i) {
                        if (sensors[i].XPos > left && sensors[i].XPos < right && sensors[i].YPos <= bottom
                            && player->YPos - player->YVelocity > bottom) {
                            sensors[i].collided = true;
                        }
                    }

                    if (sensors[1].collided || sensors[0].collided) {
                        if (player->gravity == 1) {
                            player->YPos = bottom - (CollisionTop << 16);
                        }
                        if (player->YVelocity < 1)
                            player->YVelocity = 0;
                        ScriptEng.checkResult = 4;
                    }
                }
            }
        }
    }
}
void PlatformCollision(int left, int top, int right, int bottom) {
    Player *player       = &PlayerList[PlayerNo];
    PlayerScript *script = &PlayerScriptList[PlayerNo];
    Hitbox *playerHitbox = GetPlayerCBox(script);

    CollisionLeft         = playerHitbox->left[0];
    CollisionTop          = playerHitbox->top[0];
    CollisionRight        = playerHitbox->right[0];
    CollisionBottom       = playerHitbox->bottom[0];
    sensors[0].collided   = false;
    sensors[1].collided   = false;
    sensors[2].collided   = false;
    sensors[0].XPos       = player->XPos + ((CollisionLeft + 1) << 16);
    sensors[1].XPos       = player->XPos;
    sensors[2].XPos       = player->XPos + (CollisionRight << 16);
    sensors[0].YPos       = player->YPos + (CollisionBottom << 16);
    sensors[1].YPos       = sensors[0].YPos;
    sensors[2].YPos       = sensors[0].YPos;
    ScriptEng.checkResult = false;
    for (int i = 0; i < 3; ++i) {
        if (sensors[i].XPos > left && sensors[i].XPos < right && sensors[i].YPos > top - 2 && sensors[i].YPos < bottom && player->YVelocity >= 0) {
            sensors[i].collided = 1;
            player->flailing[i] = 1;
        }
    }

    if (!sensors[0].collided && !sensors[1].collided && !sensors[2].collided)
        return;
    if (!player->gravity && (player->collisionMode == CMODE_RWALL || player->collisionMode == CMODE_LWALL)) {
        player->XVelocity = 0;
        player->speed     = 0;
    }
    player->YPos          = top - (CollisionBottom << 16);
    player->gravity       = 0;
    player->YVelocity     = 0;
    player->angle         = 0;
    player->rotation      = 0;
    ScriptEng.checkResult = true;
}