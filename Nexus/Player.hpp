#ifndef PLAYER_H
#define PLAYER_H

#define PLAYER_COUNT (0x2) 

struct PlayerMovementStats
{
    int topSpeed;
    int acceleration;
    int deceleration;
    int airAcceleration;
    int airDeceleration;
    int gravityStrength;
    int jumpStrength;
    int rollingAcceleration;
    int rollingDeceleration;
};

struct Player {
    int XPos;
    int YPos;
    int XVelocity;
    int YVelocity;
    int speed;
    int screenXPos;
    int screenYPos;
    int angle;
    int rotation;
    int timer;
    byte state;
    byte controlMode;
    byte collisionMode;
    int animationTimer;
    byte animation;
    byte animationSpeed;
    byte prevAnimation;
    byte frame;
    byte direction;
    byte skidding;
    byte pushing;
    byte collisionPlane;
    byte controlLock;
    byte frictionLoss;
    int lookPos;
    PlayerMovementStats stats;
    byte visible;
    byte tileCollisions;
    byte objectInteraction;
    byte left;
    byte right;
    byte up;
    byte down;
    byte jumpPress;
    byte jumpHold;
    byte followPlayer1;
    byte trackScroll;
    byte gravity;
    byte water;
    byte flailing[3];
    byte runningSpeed;
    byte walkingSpeed;
    byte jumpingSpeed;
};

struct PlayerScript {
    char scriptPath[64];
    int field_40;
    int scriptCodePtr_PlayerMain;
    int jumpTablePtr_PlayerMain;
    int scriptCodePtr_PlayerState[256];
    int jumpTablePtr_PlayerState[256];
    int field_84C;
    SpriteAnimation animations[64];
    byte startWalkSpeed;
    byte startRunSpeed;
    byte startJumpSpeed;
};


extern Player playerList[PLAYER_COUNT];
extern PlayerScript playerScriptList[PLAYER_COUNT];
extern int playerListPos;
extern int activePlayer;
extern int activePlayerCount;

extern ushort upBuffer;
extern ushort downBuffer;
extern ushort leftBuffer;
extern ushort rightBuffer;
extern ushort jumpPressBuffer;
extern ushort jumpHoldBuffer;

void ProcessPlayerControl(Player *player);

void SetMovementStats(PlayerMovementStats *stats);

void DefaultAirMovement(Player *player);
void DefaultGravityFalse(Player *player);
void DefaultGravityTrue(Player *player);
void DefaultGroundMovement(Player *player);
void DefaultJumpAction(Player *player);
void DefaultRollingMovement(Player *player);
void ProcessDebugMode(Player *player);

void ProcessPlayerAnimation(Player *player);

#endif // !PLAYER_H
