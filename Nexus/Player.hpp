#ifndef PLAYER_H
#define PLAYER_H

#define PLAYER_COUNT (0x2)

enum PlayerAni {
    ANI_STOPPED,
    ANI_WAITING,
    ANI_BORED,
    ANI_LOOKINGUP,
    ANI_LOOKINGDOWN,
    ANI_WALKING,
    ANI_RUNNING,
    ANI_SKIDDING,
    ANI_PEELOUT,
    ANI_SPINDASH,
    ANI_JUMPING,
    ANI_BOUNCING,
    ANI_HURT,
    ANI_DYING,
    ANI_DROWNING,
    ANI_LIFEICON,
    ANI_FANROTATE,
    ANI_BREATHING,
    ANI_PUSHING,
    ANI_FLAILINGLEFT,
    ANI_FLAILINGRIGHT,
    ANI_SLIDING,
    ANI_FINISHPOSE = 23,
    ANI_CORKSCREW  = 34,
    ANI_HANGING    = 43,
};

enum PlayerControlModes {
    CONTROLMODE_NONE     = -1,
    CONTROLMODE_NORMAL   = 0,
    CONTROLMODE_SIDEKICK = 1,
};

struct PlayerMovementStats {
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
    byte type;
    byte state;
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
    sbyte controlMode;
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

extern Player PlayerList[PLAYER_COUNT];
extern PlayerScript PlayerScriptList[PLAYER_COUNT];
extern int PlayerNo;
extern int activePlayerCount;

extern ushort DelayUp;
extern ushort DelayDown;
extern ushort DelayLeft;
extern ushort DelayRight;
extern ushort DelayJumpPress;
extern ushort DelayJumpHold;

void LoadPlayerFromList(byte characterID, byte playerID);

void ProcessPlayerAnimationChange(Player *player);
void DrawPlayer(Player *player, SpriteFrame *frame);

void ProcessPlayerControl(Player *player);

void SetMovementStats(PlayerMovementStats *stats);

void ProcessDefaultAirMovement(Player *player);
void ProcessDefaultGravityFalse(Player *player);
void ProcessDefaultGravityTrue(Player *player);
void ProcessDefaultGroundMovement(Player *player);
void ProcessDefaultJumpAction(Player *player);
void ProcessDefaultRollingMovement(Player *player);
void ProcessDebugMode(Player *player);

void ProcessPlayerAnimation(Player *player);

#endif // !PLAYER_H
