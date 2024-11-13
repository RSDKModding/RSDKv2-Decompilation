#include "RetroEngine.hpp"
#include <cmath>

ObjectScript ObjectScriptList[OBJECT_COUNT];

int ScriptData[SCRIPTDATA_COUNT];
int JumpTableData[JUMPTABLE_COUNT];
int JumpTableStack[JUMPSTACK_COUNT];

int JumpTablePos      = 0;
int JumpTableStackPos = 0;

ScriptEngine ScriptEng = ScriptEngine();
char ScriptText[0x100];

int ScriptDataPos    = 0;
int scriptDataOffset = 0;
int JumpTableDataPos = 0;
int JumpTableOffset  = 0;

#define ALIAS_COUNT (0x80)
#if !RETRO_USE_ORIGINAL_CODE
#define COMMONALIAS_COUNT (22)
#else
#define COMMONALIAS_COUNT (14)
#endif
int NO_ALIASES = 0;
int lineID     = 0;

struct AliasInfo {
    AliasInfo() {
        StrCopy(name, "");
        StrCopy(value, "");
    }
    AliasInfo(const char *aliasName, const char *aliasVal) {
        StrCopy(name, aliasName);
        StrCopy(value, aliasVal);
    }

    char name[0x20];
    char value[0x20];
};

struct FunctionInfo {
    FunctionInfo() {
        StrCopy(name, "");
        opcodeSize = 0;
    }
    FunctionInfo(const char *functionName, int opSize) {
        StrCopy(name, functionName);
        opcodeSize = opSize;
    }

    char name[0x20];
    int opcodeSize;
};

const char variableNames[][0x20] = {
    "Object.Type",
    "Object.PropertyValue",
    "Object.XPos",
    "Object.YPos",
    "Object.iXPos",
    "Object.iYPos",
    "Object.State",
    "Object.Rotation",
    "Object.Scale",
    "Object.Priority",
    "Object.DrawOrder",
    "Object.Direction",
    "Object.InkEffect",
    "Object.Frame",
    "Object.Value0",
    "Object.Value1",
    "Object.Value2",
    "Object.Value3",
    "Object.Value4",
    "Object.Value5",
    "Object.Value6",
    "Object.Value7",
    "TempValue0",
    "TempValue1",
    "TempValue2",
    "TempValue3",
    "TempValue4",
    "TempValue5",
    "TempValue6",
    "TempValue7",
    "CheckResult",
    "ArrayPos0",
    "ArrayPos1",
    "KeyDown.Up",
    "KeyDown.Down",
    "KeyDown.Left",
    "KeyDown.Right",
    "KeyDown.ButtonA",
    "KeyDown.ButtonB",
    "KeyDown.ButtonC",
    "KeyDown.Start",
    "KeyPress.Up",
    "KeyPress.Down",
    "KeyPress.Left",
    "KeyPress.Right",
    "KeyPress.ButtonA",
    "KeyPress.ButtonB",
    "KeyPress.ButtonC",
    "KeyPress.Start",
    "Menu1.Selection",
    "Menu2.Selection",
    "Stage.ActiveList",
    "Stage.ListPos",
    "XScrollOffset",
    "YScrollOffset",
    "Global",
    "Stage.TimeEnabled",
    "Stage.MilliSeconds",
    "Stage.Seconds",
    "Stage.Minutes",
    "Stage.ActNo",
    "Object.EntityNo",
    "Player.Type",
    "Player.State",
    "Player.ControlMode",
    "Player.CollisionMode",
    "Player.CollisionPlane",
    "Player.XPos",
    "Player.YPos",
    "Player.ScreenXPos",
    "Player.ScreenYPos",
    "Player.Speed",
    "Player.XVelocity",
    "Player.YVelocity",
    "Player.Gravity",
    "Player.Angle",
    "Player.Rotation",
    "Player.Direction",
    "Player.Animation",
    "Player.Frame",
    "Player.Skidding",
    "Player.Pushing",
    "Player.FrictionLoss",
    "Player.WalkingSpeed",
    "Player.RunningSpeed",
    "Player.JumpingSpeed",
    "Player.TrackScroll",
    "Player.Up",
    "Player.Down",
    "Player.Left",
    "Player.Right",
    "Player.JumpPress",
    "Player.JumpHold",
    "Player.FollowPlayer1",
    "Player.LookPos",
    "Player.Water",
    "Player.TopSpeed",
    "Player.Acceleration",
    "Player.Deceleration",
    "Player.AirAcceleration",
    "Player.AirDeceleration",
    "Player.GravityStrength",
    "Player.JumpStrength",
    "Player.RollingAcceleration",
    "Player.RollingDeceleration",
    "Player.EntityNo",
    "Player.CollisionLeft",
    "Player.CollisionTop",
    "Player.CollisionRight",
    "Player.CollisionBottom",
    "Player.Flailing",
    "Stage.PauseEnabled",
    "Stage.ListSize",
    "Player.Timer",
    "Player.AnimationSpeed",
    "Player.TileCollisions",
    "Player.ObjectInteraction",
    "Stage.CameraEnabled",
    "Stage.CameraStyle",
    "Music.Volume",
    "Music.CurrentTrack",
    "Player.Visible",
    "Stage.NewXBoundary1",
    "Stage.NewXBoundary2",
    "Stage.NewYBoundary1",
    "Stage.NewYBoundary2",
    "Stage.XBoundary1",
    "Stage.XBoundary2",
    "Stage.YBoundary1",
    "Stage.YBoundary2",
    "Object.OutOfBounds",
};

const FunctionInfo functions[] = { FunctionInfo("End", 0),
                                   FunctionInfo("Equal", 2),
                                   FunctionInfo("Add", 2),
                                   FunctionInfo("Sub", 2),
                                   FunctionInfo("Inc", 1),
                                   FunctionInfo("Dec", 1),
                                   FunctionInfo("Mul", 2),
                                   FunctionInfo("Div", 2),
                                   FunctionInfo("ShR", 2),
                                   FunctionInfo("ShL", 2),
                                   FunctionInfo("And", 2),
                                   FunctionInfo("Or", 2),
                                   FunctionInfo("Xor", 2),
                                   FunctionInfo("Not", 1),
                                   FunctionInfo("FlipSign", 1),
                                   FunctionInfo("CheckEqual", 2),
                                   FunctionInfo("CheckGreater", 2),
                                   FunctionInfo("CheckLower", 2),
                                   FunctionInfo("CheckNotEqual", 2),
                                   FunctionInfo("IfEqual", 3),
                                   FunctionInfo("IfGreater", 3),
                                   FunctionInfo("IfGreaterOrEqual", 3),
                                   FunctionInfo("IfLower", 3),
                                   FunctionInfo("IfLowerOrEqual", 3),
                                   FunctionInfo("IfNotEqual", 3),
                                   FunctionInfo("else", 0),
                                   FunctionInfo("endif", 0),
                                   FunctionInfo("WEqual", 3),
                                   FunctionInfo("WGreater", 3),
                                   FunctionInfo("WGreaterOrEqual", 3),
                                   FunctionInfo("WLower", 3),
                                   FunctionInfo("WLowerOrEqual", 3),
                                   FunctionInfo("WNotEqual", 3),
                                   FunctionInfo("loop", 0),
                                   FunctionInfo("switch", 2),
                                   FunctionInfo("break", 0),
                                   FunctionInfo("endswitch", 0),
                                   FunctionInfo("Rand", 2),
                                   FunctionInfo("Sin", 2),
                                   FunctionInfo("Cos", 2),
                                   FunctionInfo("Sin256", 2),
                                   FunctionInfo("Cos256", 2),
                                   FunctionInfo("SinChange", 5),
                                   FunctionInfo("CosChange", 5),
                                   FunctionInfo("ATan2", 3),
                                   FunctionInfo("Interpolate", 4),
                                   FunctionInfo("InterpolateXY", 7),
                                   FunctionInfo("LoadSpriteSheet", 1),
                                   FunctionInfo("RemoveSpriteSheet", 1),
                                   FunctionInfo("DrawSprite", 1),
                                   FunctionInfo("DrawSpriteXY", 3),
                                   FunctionInfo("DrawSpriteScreenXY", 3),
                                   FunctionInfo("DrawSprite3D", 1),
                                   FunctionInfo("DrawNumbers", 7),
                                   FunctionInfo("DrawActName", 7),
                                   FunctionInfo("DrawMenu", 3),
                                   FunctionInfo("SpriteFrame", 6),
                                   FunctionInfo("SetDebugIcon", 6),
                                   FunctionInfo("LoadPalette", 3),
                                   FunctionInfo("RotatePalette", 3),
                                   FunctionInfo("SetFade", 6),
                                   FunctionInfo("SetWaterColor", 4),
                                   FunctionInfo("SetBlendTable", 4),
                                   FunctionInfo("SetTintTable", 6),
                                   FunctionInfo("ClearScreen", 1),
                                   FunctionInfo("DrawSpriteFX", 4),
                                   FunctionInfo("DrawSpriteScreenFX", 4),
                                   FunctionInfo("DrawLifeIcon", 2),
                                   FunctionInfo("SetupMenu", 4),
                                   FunctionInfo("AddMenuEntry", 3),
                                   FunctionInfo("EditMenuEntry", 4),
                                   FunctionInfo("LoadStage", 0),
                                   FunctionInfo("DrawTintRect", 5),
                                   FunctionInfo("ResetObjectEntity", 5),
                                   FunctionInfo("PlayerObjectCollision", 5),
                                   FunctionInfo("CreateTempObject", 4),
                                   FunctionInfo("DefaultGroundMovement", 0),
                                   FunctionInfo("DefaultAirMovement", 0),
                                   FunctionInfo("DefaultRollingMovement", 0),
                                   FunctionInfo("DefaultGravityTrue", 0),
                                   FunctionInfo("DefaultGravityFalse", 0),
                                   FunctionInfo("DefaultJumpAction", 0),
                                   FunctionInfo("SetMusicTrack", 3),
                                   FunctionInfo("PlayMusic", 1),
                                   FunctionInfo("StopMusic", 0),
                                   FunctionInfo("PlaySfx", 2),
                                   FunctionInfo("StopSfx", 1),
                                   FunctionInfo("SetSfxAttributes", 3),
                                   FunctionInfo("ObjectTileCollision", 4),
                                   FunctionInfo("ObjectTileGrip", 4),
                                   FunctionInfo("LoadVideo", 1),
                                   FunctionInfo("NextVideoFrame", 0),
                                   FunctionInfo("PlayStageSfx", 2),
                                   FunctionInfo("StopStageSfx", 1) };

AliasInfo aliases[0x80] = {
    AliasInfo("true", "1"),          AliasInfo("false", "0"),       AliasInfo("FX_SCALE", "0"),
    AliasInfo("FX_ROTATE", "1"),     AliasInfo("FX_INK", "2"),      AliasInfo("PRESENTATION_STAGE", "0"),
    AliasInfo("REGULAR_STAGE", "1"), AliasInfo("BONUS_STAGE", "2"), AliasInfo("SPECIAL_STAGE", "3"),
    AliasInfo("MENU_1", "0"),        AliasInfo("MENU_2", "1"),      AliasInfo("C_TOUCH", "0"),
    AliasInfo("C_BOX", "1"),         AliasInfo("C_PLATFORM", "2"),
#if !RETRO_USE_ORIGINAL_CODE
    AliasInfo("INK_NONE", "0"),      AliasInfo("INK_BLEND", "1"),   AliasInfo("INK_TINT", "2"),
    AliasInfo("FX_TINT", "3"),       AliasInfo("FLIP_NONE", "0"),   AliasInfo("FLIP_X", "1"),
    AliasInfo("FLIP_Y", "2"),        AliasInfo("FLIP_XY", "3"),
#endif
};

const char scriptEvaluationTokens[][0x4] = {
    "=", "+=", "-=", "++", "--", "*=", "/=", ">>=", "<<=", "&=", "|=", "^=", "==", ">", ">=", "<", "<=", "!="
};

enum ScriptReadModes { READMODE_NORMAL = 0, READMODE_STRING = 1, READMODE_COMMENTLINE = 2, READMODE_ENDLINE = 3, READMODE_EOF = 4 };
enum ScriptParseModes {
    PARSEMODE_SCOPELESS    = 0,
    PARSEMODE_PLATFORMSKIP = 1,
    PARSEMODE_FUNCTION     = 2,
    PARSEMODE_SWITCHREAD   = 3,
    PARSEMODE_ERROR        = 0xFF
};

enum ScriptVarTypes { SCRIPTVAR_VAR = 1, SCRIPTVAR_INTCONST = 2, SCRIPTVAR_STRCONST = 3 };
enum ScriptVarArrTypes { VARARR_NONE = 0, VARARR_ARRAY = 1, VARARR_ENTNOPLUS1 = 2, VARARR_ENTNOMINUS1 = 3 };

enum ScrVariable {
    VAR_OBJECTTYPE,
    VAR_OBJECTPROPERTYVALUE,
    VAR_OBJECTXPOS,
    VAR_OBJECTYPOS,
    VAR_OBJECTIXPOS,
    VAR_OBJECTIYPOS,
    VAR_OBJECTSTATE,
    VAR_OBJECTROTATION,
    VAR_OBJECTSCALE,
    VAR_OBJECTPRIORITY,
    VAR_OBJECTDRAWORDER,
    VAR_OBJECTDIRECTION,
    VAR_OBJECTINKEFFECT,
    VAR_OBJECTFRAME,
    VAR_OBJECTVALUE0,
    VAR_OBJECTVALUE1,
    VAR_OBJECTVALUE2,
    VAR_OBJECTVALUE3,
    VAR_OBJECTVALUE4,
    VAR_OBJECTVALUE5,
    VAR_OBJECTVALUE6,
    VAR_OBJECTVALUE7,
    VAR_TEMPVALUE0,
    VAR_TEMPVALUE1,
    VAR_TEMPVALUE2,
    VAR_TEMPVALUE3,
    VAR_TEMPVALUE4,
    VAR_TEMPVALUE5,
    VAR_TEMPVALUE6,
    VAR_TEMPVALUE7,
    VAR_CHECKRESULT,
    VAR_ARRAYPOS0,
    VAR_ARRAYPOS1,
    VAR_KEYDOWNUP,
    VAR_KEYDOWNDOWN,
    VAR_KEYDOWNLEFT,
    VAR_KEYDOWNRIGHT,
    VAR_KEYDOWNBUTTONA,
    VAR_KEYDOWNBUTTONB,
    VAR_KEYDOWNBUTTONC,
    VAR_KEYDOWNSTART,
    VAR_KEYPRESSUP,
    VAR_KEYPRESSDOWN,
    VAR_KEYPRESSLEFT,
    VAR_KEYPRESSRIGHT,
    VAR_KEYPRESSBUTTONA,
    VAR_KEYPRESSBUTTONB,
    VAR_KEYPRESSBUTTONC,
    VAR_KEYPRESSSTART,
    VAR_MENU1SELECTION,
    VAR_MENU2SELECTION,
    VAR_STAGEACTIVELIST,
    VAR_STAGELISTPOS,
    VAR_XSCROLLOFFSET,
    VAR_YSCROLLOFFSET,
    VAR_GLOBAL,
    VAR_STAGETIMEENABLED,
    VAR_STAGEMILLISECONDS,
    VAR_STAGESECONDS,
    VAR_STAGEMINUTES,
    VAR_STAGEACTNO,
    VAR_OBJECTENTITYNO,
    VAR_PLAYERTYPE,
    VAR_PLAYERSTATE,
    VAR_PLAYERCONTROLMODE,
    VAR_PLAYERCOLLISIONMODE,
    VAR_PLAYERCOLLISIONPLANE,
    VAR_PLAYERXPOS,
    VAR_PLAYERYPOS,
    VAR_PLAYERSCREENXPOS,
    VAR_PLAYERSCREENYPOS,
    VAR_PLAYERSPEED,
    VAR_PLAYERXVELOCITY,
    VAR_PLAYERYVELOCITY,
    VAR_PLAYERGRAVITY,
    VAR_PLAYERANGLE,
    VAR_PLAYERROTATION,
    VAR_PLAYERDIRECTION,
    VAR_PLAYERANIMATION,
    VAR_PLAYERFRAME,
    VAR_PLAYERSKIDDING,
    VAR_PLAYERPUSHING,
    VAR_PLAYERFRICTIONLOSS,
    VAR_PLAYERWALKINGSPEED,
    VAR_PLAYERRUNNINGSPEED,
    VAR_PLAYERJUMPINGSPEED,
    VAR_PLAYERTRACKSCROLL,
    VAR_PLAYERUP,
    VAR_PLAYERDOWN,
    VAR_PLAYERLEFT,
    VAR_PLAYERRIGHT,
    VAR_PLAYERJUMPPRESS,
    VAR_PLAYERJUMPHOLD,
    VAR_PLAYERFOLLOWPLAYER1,
    VAR_PLAYERLOOKPOS,
    VAR_PLAYERWATER,
    VAR_PLAYERTOPSPEED,
    VAR_PLAYERACCELERATION,
    VAR_PLAYERDECELERATION,
    VAR_PLAYERAIRACCELERATION,
    VAR_PLAYERAIRDECELERATION,
    VAR_PLAYERGRAVITYSTRENGTH,
    VAR_PLAYERJUMPSTRENGTH,
    VAR_PLAYERROLLINGACCELERATION,
    VAR_PLAYERROLLINGDECELERATION,
    VAR_PLAYERENTITYNO,
    VAR_PLAYERCOLLISIONLEFT,
    VAR_PLAYERCOLLISIONTOP,
    VAR_PLAYERCOLLISIONRIGHT,
    VAR_PLAYERCOLLISIONBOTTOM,
    VAR_PLAYERFLAILING,
    VAR_STAGEPAUSEENABLED,
    VAR_STAGELISTSIZE,
    VAR_PLAYERTIMER,
    VAR_PLAYERANIMATIONSPEED,
    VAR_PLAYERTILECOLLISIONS,
    VAR_PLAYEROBJECTINTERACTION,
    VAR_SCREENCAMERAENABLED,
    VAR_SCREENCAMERASTYLE,
    VAR_MUSICVOLUME,
    VAR_MUSICCURRENTTRACK,
    VAR_PLAYERVISIBLE,
    VAR_STAGENEWXBOUNDARY1,
    VAR_STAGENEWXBOUNDARY2,
    VAR_STAGENEWYBOUNDARY1,
    VAR_STAGENEWYBOUNDARY2,
    VAR_STAGEXBOUNDARY1,
    VAR_STAGEXBOUNDARY2,
    VAR_STAGEYBOUNDARY1,
    VAR_STAGEYBOUNDARY2,
    VAR_OBJECTOUTOFBOUNDS,
    VAR_MAX_CNT,
};

enum ScrFunction {
    FUNC_END,
    FUNC_EQUAL,
    FUNC_ADD,
    FUNC_SUB,
    FUNC_INC,
    FUNC_DEC,
    FUNC_MUL,
    FUNC_DIV,
    FUNC_SHR,
    FUNC_SHL,
    FUNC_AND,
    FUNC_OR,
    FUNC_XOR,
    FUNC_NOT,
    FUNC_FLIPSIGN,
    FUNC_CHECKEQUAL,
    FUNC_CHECKGREATER,
    FUNC_CHECKLOWER,
    FUNC_CHECKNOTEQUAL,
    FUNC_IFEQUAL,
    FUNC_IFGREATER,
    FUNC_IFGREATEROREQUAL,
    FUNC_IFLOWER,
    FUNC_IFLOWEROREQUAL,
    FUNC_IFNOTEQUAL,
    FUNC_ELSE,
    FUNC_ENDIF,
    FUNC_WEQUAL,
    FUNC_WGREATER,
    FUNC_WGREATEROREQUAL,
    FUNC_WLOWER,
    FUNC_WLOWEROREQUAL,
    FUNC_WNOTEQUAL,
    FUNC_LOOP,
    FUNC_SWITCH,
    FUNC_BREAK,
    FUNC_ENDSWITCH,
    FUNC_RAND,
    FUNC_SIN,
    FUNC_COS,
    FUNC_SIN256,
    FUNC_COS256,
    FUNC_SINCHANGE,
    FUNC_COSCHANGE,
    FUNC_ATAN2,
    FUNC_INTERPOLATE,
    FUNC_INTERPOLATEXY,
    FUNC_LOADSPRITESHEET,
    FUNC_REMOVESPRITESHEET,
    FUNC_DRAWSPRITE,
    FUNC_DRAWSPRITEXY,
    FUNC_DRAWSPRITESCREENXY,
    FUNC_DRAWSPRITE3D,
    FUNC_DRAWNUMBERS,
    FUNC_DRAWACTNAME,
    FUNC_DRAWMENU,
    FUNC_SPRITEFRAME,
    FUNC_SETDEBUGICON,
    FUNC_LOADPALETTE,
    FUNC_ROTATEPALETTE,
    FUNC_SETFADE,
    FUNC_SETWATERCOLOR,
    FUNC_SETBLENDTABLE,
    FUNC_SETTINTTABLE,
    FUNC_CLEARSCREEN,
    FUNC_DRAWSPRITEFX,
    FUNC_DRAWSPRITESCREENFX,
    FUNC_DRAWLIFEICON,
    FUNC_SETUPMENU,
    FUNC_ADDMENUENTRY,
    FUNC_EDITMENUENTRY,
    FUNC_LOADSTAGE,
    FUNC_DRAWTINTRECT,
    FUNC_RESETOBJECTENTITY,
    FUNC_PLAYEROBJECTCOLLISION,
    FUNC_CREATETEMPOBJECT,
    FUNC_DEFAULTGROUNDMOVEMENT,
    FUNC_DEFAULTAIRMOVEMENT,
    FUNC_DEFAULTROLLINGMOVEMENT,
    FUNC_DEFAULTGRAVITYTRUE,
    FUNC_DEFAULTGRAVITYFALSE,
    FUNC_DEFAULTJUMPACTION,
    FUNC_SETMUSICTRACK,
    FUNC_PLAYMUSIC,
    FUNC_STOPMUSIC,
    FUNC_PLAYSFX,
    FUNC_STOPSFX,
    FUNC_SETSFXATTRIBUTES,
    FUNC_OBJECTTILECOLLISION,
    FUNC_OBJECTTILEGRIP,
    FUNC_LOADVIDEO,
    FUNC_NEXTVIDEOFRAME,
    FUNC_PLAYSTAGESFX,
    FUNC_STOPSTAGESFX,
    FUNC_MAX_CNT
};

void CheckAliasText(char *text) {
    if (FindStringToken(text, "#alias", 1))
        return;
    int textPos     = 6;
    int aliasStrPos = 0;
    int aliasMatch  = 0;
    while (aliasMatch < 2) {
        if (aliasMatch) {
            if (aliasMatch == 1) {
                aliases[NO_ALIASES].name[aliasStrPos] = text[textPos];
                if (text[textPos]) {
                    aliasStrPos++;
                } else {
                    aliasStrPos = 0;
                    ++aliasMatch;
                }
            }
        } else if (text[textPos] == ':') {
            aliases[NO_ALIASES].value[aliasStrPos] = 0;
            aliasStrPos                            = 0;
            aliasMatch                             = 1;
        } else {
            aliases[NO_ALIASES].value[aliasStrPos++] = text[textPos];
        }
        ++textPos;
    }
    ++NO_ALIASES;
}
void ConvertArithmaticSyntax(char *text) {
    int token  = 0;
    int offset = 0;
    int findID = 0;
    char dest[260];

    for (int i = FUNC_EQUAL; i < FUNC_NOT; ++i) {
        findID = FindStringToken(text, scriptEvaluationTokens[i - 1], 1);
        if (findID > -1) {
            offset = findID;
            token  = i;
        }
    }
    if (token > 0) {
        StrCopy(dest, functions[token].name);
        StrAdd(dest, "(");
        findID = StrLength(dest);
        for (int i = 0; i < offset; ++i) dest[findID++] = text[i];
        if (functions[token].opcodeSize > 1) {
            dest[findID] = ',';
            int len      = StrLength(scriptEvaluationTokens[token - 1]);
            offset += len;
            ++findID;
            while (text[offset]) dest[findID++] = text[offset++];
        }
        dest[findID] = 0;
        StrAdd(dest, ")");
        StrCopy(text, dest);
    }
}
void ConvertIfWhileStatement(char *text) {
    char dest[260];
    int compareOp  = -1;
    int strPos     = 0;
    int destStrPos = 0;
    if (FindStringToken(text, "if", 1)) {
        if (!FindStringToken(text, "while", 1)) { // if no "if" but there is "while"
            for (int i = 0; i < 6; ++i) {
                destStrPos = FindStringToken(text, scriptEvaluationTokens[i + (FUNC_NOT - 1)], 1);
                if (destStrPos > -1) {
                    strPos    = destStrPos;
                    compareOp = i;
                }
            }
            if (compareOp > -1) {
                text[strPos] = ',';
                StrCopy(dest, functions[compareOp + FUNC_WEQUAL].name);
                StrAdd(dest, "(");
                AppendIntegerToString(dest, JumpTableDataPos - JumpTableOffset);
                StrAdd(dest, ",");
                destStrPos = StrLength(dest);
                for (int i = 5; text[i]; ++i) {
                    if (text[i] != '=' && text[i] != '(' && text[i] != ')')
                        dest[destStrPos++] = text[i];
                }
                dest[destStrPos] = 0;
                StrAdd(dest, ")");
                StrCopy(text, dest);
                JumpTableStack[++JumpTableStackPos] = JumpTableDataPos;
                JumpTableData[JumpTableDataPos++]   = ScriptDataPos - scriptDataOffset;
                JumpTableData[JumpTableDataPos++]   = 0;
            }
        }
    } else {
        for (int i = 0; i < 6; ++i) {
            destStrPos = FindStringToken(text, scriptEvaluationTokens[i + (FUNC_NOT - 1)], 1);
            if (destStrPos > -1) {
                strPos    = destStrPos;
                compareOp = i;
            }
        }
        if (compareOp > -1) {
            text[strPos] = ',';
            StrCopy(dest, functions[compareOp + FUNC_IFEQUAL].name);
            StrAdd(dest, "(");
            AppendIntegerToString(dest, JumpTableDataPos - JumpTableOffset);
            StrAdd(dest, ",");
            destStrPos = StrLength(dest);
            for (int i = 2; text[i]; ++i) {
                if (text[i] != '=' && text[i] != '(' && text[i] != ')')
                    dest[destStrPos++] = text[i];
            }
            dest[destStrPos] = 0;
            StrAdd(dest, ")");
            StrCopy(text, dest);
            JumpTableStack[++JumpTableStackPos] = JumpTableDataPos;
            JumpTableData[JumpTableDataPos++]   = -1;
            JumpTableData[JumpTableDataPos++]   = 0;
        }
    }
}
bool ConvertSwitchStatement(char *text) {
    if (FindStringToken(text, "switch", 1))
        return false;
    char switchText[260];
    StrCopy(switchText, "switch");
    StrAdd(switchText, "(");
    AppendIntegerToString(switchText, JumpTableDataPos - JumpTableOffset);
    StrAdd(switchText, ",");
    int pos = StrLength(switchText);
    for (int i = 6; text[i]; ++i) {
        if (text[i] != '=' && text[i] != '(' && text[i] != ')')
            switchText[pos++] = text[i];
    }
    switchText[pos] = 0;
    StrAdd(switchText, ")");
    StrCopy(text, switchText);
    JumpTableStack[++JumpTableStackPos] = JumpTableDataPos;
    JumpTableData[JumpTableDataPos++]   = 0x10000;
    JumpTableData[JumpTableDataPos++]   = -0x10000;
    JumpTableData[JumpTableDataPos++]   = -1;
    JumpTableData[JumpTableDataPos++]   = 0;
    return true;
}
void ConvertFunctionText(char *text) {
    char strBuffer[128];
    char funcName[132];
    int opcode     = 0;
    int opcodeSize = 0;
    int textPos    = 0;
    int namePos    = 0;
    for (namePos = 0; text[namePos] != '(' && text[namePos]; ++namePos) funcName[namePos] = text[namePos];
    funcName[namePos] = 0;
    for (int i = 0; i < FUNC_MAX_CNT; ++i) {
        if (StrComp(funcName, functions[i].name)) {
            opcode     = i;
            opcodeSize = functions[i].opcodeSize;
            textPos    = StrLength(functions[i].name);
            i          = FUNC_MAX_CNT;
        }
    }
    if (opcode <= 0) {
        // error lol
    } else {
        ScriptData[ScriptDataPos++] = opcode;
        if (StrComp("else", functions[opcode].name))
            JumpTableData[JumpTableStack[JumpTableStackPos]] = ScriptDataPos - scriptDataOffset;

        if (StrComp("endif", functions[opcode].name) == 1) {
            int jPos                = JumpTableStack[JumpTableStackPos];
            JumpTableData[jPos + 1] = ScriptDataPos - scriptDataOffset;
            if (JumpTableData[jPos] == -1)
                JumpTableData[jPos] = (ScriptDataPos - scriptDataOffset) - 1;
            --JumpTableStackPos;
        }

        if (StrComp("endswitch", functions[opcode].name)) {
            int jPos                = JumpTableStack[JumpTableStackPos];
            JumpTableData[jPos + 3] = ScriptDataPos - scriptDataOffset;
            if (JumpTableData[jPos + 2] == -1) {
                JumpTableData[jPos + 2] = (ScriptDataPos - scriptDataOffset) - 1;
                int caseCnt             = abs(JumpTableData[jPos + 1] - JumpTableData[jPos]) + 1;

                int jOffset = jPos + 4;
                for (int c = 0; c < caseCnt; ++c) {
                    if (JumpTableData[jOffset + c] < 0)
                        JumpTableData[jOffset + c] = JumpTableData[jPos + 2];
                }
            }
            --JumpTableStackPos;
        }

        if (StrComp("loop", functions[opcode].name)) {
            JumpTableData[JumpTableStack[JumpTableStackPos--] + 1] = ScriptDataPos - scriptDataOffset;
        }

        for (int i = 0; i < opcodeSize; ++i) {
            ++textPos;
            int funcNamePos      = 0;
            int value            = 0;
            int scriptTextByteID = 0;
            while (text[textPos] != ',' && text[textPos] != ')' && text[textPos]) {
                if (value) {
                    if (text[textPos] == ']')
                        value = 0;
                    else
                        strBuffer[scriptTextByteID++] = text[textPos];
                    ++textPos;
                } else {
                    if (text[textPos] == '[')
                        value = 1;
                    else
                        funcName[funcNamePos++] = text[textPos];
                    ++textPos;
                }
            }
            funcName[funcNamePos]       = 0;
            strBuffer[scriptTextByteID] = 0;
            // Eg: TempValue0 = FX_SCALE
            for (int a = 0; a < NO_ALIASES; ++a) {
                if (StrComp(funcName, aliases[a].name)) {
                    CopyAliasStr(funcName, aliases[a].value, 0);
                    if (FindStringToken(aliases[a].value, "[", 1) > -1)
                        CopyAliasStr(strBuffer, aliases[a].value, 1);
                }
            }
            // Eg: TempValue0 = Game.Variable
            for (int v = 0; v < NO_GLOBALVARIABLES; ++v) {
                if (StrComp(funcName, GlobalVariableNames[v])) {
                    StrCopy(funcName, "Global");
                    strBuffer[0] = 0;
                    AppendIntegerToString(strBuffer, v);
                }
            }
            if (ConvertStringToInteger(funcName, &value)) {
                ScriptData[ScriptDataPos++] = SCRIPTVAR_INTCONST;
                ScriptData[ScriptDataPos++] = value;
            } else if (funcName[0] == '"') {
                ScriptData[ScriptDataPos++] = SCRIPTVAR_STRCONST;
                ScriptData[ScriptDataPos++] = StrLength(funcName) - 2;
                int scriptTextPos           = 1;
                scriptTextByteID            = 0;
                while (scriptTextPos > -1) {
                    switch (scriptTextByteID) {
                        case 0:
                            ScriptData[ScriptDataPos] = funcName[scriptTextPos] << 24;
                            ++scriptTextByteID;
                            break;
                        case 1:
                            ScriptData[ScriptDataPos] += funcName[scriptTextPos] << 16;
                            ++scriptTextByteID;
                            break;
                        case 2:
                            ScriptData[ScriptDataPos] += funcName[scriptTextPos] << 8;
                            ++scriptTextByteID;
                            break;
                        case 3:
                            ScriptData[ScriptDataPos++] += funcName[scriptTextPos];
                            scriptTextByteID = 0;
                            break;
                        default: break;
                    }
                    if (funcName[scriptTextPos] == '"') {
                        if (scriptTextByteID > 0)
                            ++ScriptDataPos;
                        scriptTextPos = -1;
                    } else {
                        scriptTextPos++;
                    }
                }
            } else {
                ScriptData[ScriptDataPos++] = SCRIPTVAR_VAR;
                if (strBuffer[0]) {
                    ScriptData[ScriptDataPos] = VARARR_ARRAY;
                    if (strBuffer[0] == '+')
                        ScriptData[ScriptDataPos] = VARARR_ENTNOPLUS1;
                    if (strBuffer[0] == '-')
                        ScriptData[ScriptDataPos] = VARARR_ENTNOMINUS1;
                    ++ScriptDataPos;
                    if (strBuffer[0] == '-' || strBuffer[0] == '+') {
                        for (int i = 0; i < StrLength(strBuffer); ++i) strBuffer[i] = strBuffer[i + 1];
                    }
                    if (ConvertStringToInteger(strBuffer, &value) == 1) {
                        ScriptData[ScriptDataPos++] = 0;
                        ScriptData[ScriptDataPos++] = value;
                    } else {
                        if (StrComp(strBuffer, "ArrayPos0"))
                            value = 0;
                        if (StrComp(strBuffer, "ArrayPos1"))
                            value = 1;
                        if (StrComp(strBuffer, "TempObjectPos"))
                            value = 2;
                        ScriptData[ScriptDataPos++] = 1;
                        ScriptData[ScriptDataPos++] = value;
                    }
                } else {
                    ScriptData[ScriptDataPos++] = VARARR_NONE;
                }
                value = -1;
                for (int i = 0; i < VAR_MAX_CNT; ++i) {
                    if (StrComp(funcName, variableNames[i]))
                        value = i;
                }

                if (value == -1) {
                    // error
                    value = 0;
                }
                ScriptData[ScriptDataPos++] = value;
            }
        }
    }
}
void CheckCaseNumber(char *text) {
    if (FindStringToken(text, "case", 1))
        return;

    char caseString[128];
    int caseStrPos = 0;
    char caseChar  = text[4];
    if (text[4]) {
        int textPos = 5;
        do {
            if (caseChar != ':')
                caseString[caseStrPos++] = caseChar;
            caseChar = text[textPos++];
        } while (caseChar);
    } else {
        caseStrPos = 0;
    }
    caseString[caseStrPos] = 0;

    for (int a = 0; a < NO_ALIASES; ++a) {
        if (StrComp(aliases[a].name, caseString)) {
            StrCopy(caseString, aliases[a].value);
            break;
        }
    }

    int caseID = 0;
    if (ConvertStringToInteger(caseString, &caseID)) {
        int stackValue = JumpTableStack[JumpTableStackPos];
        if (caseID < JumpTableData[stackValue])
            JumpTableData[stackValue] = caseID;
        stackValue++;
        if (caseID > JumpTableData[stackValue])
            JumpTableData[stackValue] = caseID;
    } else {
        PrintLog("WARNING: unable to convert case string \"%s\" to int, on line %d", caseString, lineID);
    }
}
bool ReadSwitchCase(char *text) {
    char caseText[0x80];
    if (FindStringToken(text, "case", 1)) {
        if (FindStringToken(text, "default", 1)) {
            return false;
        } else {
            int jumpTablepos                = JumpTableStack[JumpTableStackPos];
            JumpTableData[jumpTablepos + 2] = ScriptDataPos - scriptDataOffset;
            int cnt                         = abs(JumpTableData[jumpTablepos + 1] - JumpTableData[jumpTablepos]) + 1;

            int jOffset = jumpTablepos + 4;
            for (int i = 0; i < cnt; ++i) {
                if (JumpTableData[jOffset + i] < 0)
                    JumpTableData[jOffset + i] = ScriptDataPos - scriptDataOffset;
            }
            return true;
        }
    } else {
        int textPos       = 4;
        int caseStringPos = 0;
        while (text[textPos]) {
            if (text[textPos] != ':')
                caseText[caseStringPos++] = text[textPos];
            ++textPos;
        }
        caseText[caseStringPos] = 0;
        for (int a = 0; a < NO_ALIASES; ++a) {
            if (StrComp(caseText, aliases[a].name))
                StrCopy(caseText, aliases[a].value);
        }

        int val = 0;

        int jPos    = JumpTableStack[JumpTableStackPos];
        int jOffset = jPos + 4;
        if (ConvertStringToInteger(caseText, &val))
            JumpTableData[val - JumpTableData[jPos] + jOffset] = ScriptDataPos - scriptDataOffset;
        return true;
    }
    return false;
}
void AppendIntegerToString(char *text, int value) {
    int textPos = 0;
    while (true) {
        if (!text[textPos])
            break;
        ++textPos;
    }

    int cnt = 0;
    int v   = value;
    while (v != 0) {
        v /= 10;
        cnt++;
    }

    v = 0;
    for (int i = cnt - 1; i >= 0; --i) {
        v = value / pow(10, i);
        v %= 10;

        int strValue = v + '0';
        if (strValue < '0' || strValue > '9') {
            // what
        }
        text[textPos++] = strValue;
    }
    if (value == 0)
        text[textPos++] = '0';
    text[textPos] = 0;
}
bool ConvertStringToInteger(char *text, int *value) {
    int charID    = 0;
    bool negative = false;
    int base      = 10;
    *value        = 0;
    if (*text != '+' && !(*text >= '0' && *text <= '9') && *text != '-')
        return false;
    int strLength = StrLength(text) - 1;
    uint charVal  = 0;
    if (*text == '-') {
        negative = true;
        charID   = 1;
        --strLength;
    } else if (*text == '+') {
        charID = 1;
        --strLength;
    }

    if (text[charID] == '0') {
        if (text[charID + 1] == 'x' || text[charID + 1] == 'X')
            base = 0x10;
        else if (text[charID + 1] == 'b' || text[charID + 1] == 'B')
            base = 0b10;
        else if (text[charID + 1] == 'o' || text[charID + 1] == 'O')
            base = 0010; // base 8

        if (base != 10) {
            charID += 2;
            strLength -= 2;
        }
    }

    while (strLength > -1) {
        bool flag = text[charID] < '0';
        if (!flag) {
            if (base == 0x10 && text[charID] > 'f')
                flag = true;
            if (base == 0010 && text[charID] > '7')
                flag = true;
            if (base == 0b10 && text[charID] > '1')
                flag = true;
        }

        if (flag) {
            return 0;
        }
        if (strLength <= 0) {
            if (text[charID] >= '0' && text[charID] <= '9') {
                *value = text[charID] + *value - '0';
            } else if (text[charID] >= 'a' && text[charID] <= 'f') {
                charVal = text[charID] - 'a';
                charVal += 10;
                *value += charVal;
            } else if (text[charID] >= 'A' && text[charID] <= 'F') {
                charVal = text[charID] - 'A';
                charVal += 10;
                *value += charVal;
            }
        } else {
            int strlen = strLength + 1;
            charVal    = 0;
            if (text[charID] >= '0' && text[charID] <= '9') {
                charVal = text[charID] - '0';
            } else if (text[charID] >= 'a' && text[charID] <= 'f') {
                charVal = text[charID] - 'a';
                charVal += 10;
            } else if (text[charID] >= 'A' && text[charID] <= 'F') {
                charVal = text[charID] - 'A';
                charVal += 10;
            }
            for (; --strlen; charVal *= base);
            *value += charVal;
        }
        --strLength;
        ++charID;
    }
    if (negative)
        *value = -*value;
    return true;
}
void CopyAliasStr(char *dest, char *text, bool arrayIndex) {
    int textPos     = 0;
    int destPos     = 0;
    bool arrayValue = false;
    if (arrayIndex) {
        while (text[textPos]) {
            if (arrayValue) {
                if (text[textPos] == ']')
                    arrayValue = false;
                else
                    dest[destPos++] = text[textPos];
                ++textPos;
            } else {
                if (text[textPos] == '[')
                    arrayValue = true;
                ++textPos;
            }
        }
    } else {
        while (text[textPos]) {
            if (arrayValue) {
                if (text[textPos] == ']')
                    arrayValue = false;
                ++textPos;
            } else {
                if (text[textPos] == '[')
                    arrayValue = true;
                else
                    dest[destPos++] = text[textPos];
                ++textPos;
            }
        }
    }
    dest[destPos] = 0;
}
bool CheckOpcodeType(char *text) {
    while (true) {
        int c = *text;
        if (!*text)
            break;
        ++text;
        if (c == '(')
            return false;
    }
    return true;
}

void ParseScriptFile(char *scriptName, int scriptID) {
    int currentSub    = -1;
    JumpTableStackPos = 0;
    lineID            = 0;
    NO_ALIASES        = COMMONALIAS_COUNT;
    for (int i = COMMONALIAS_COUNT; i < ALIAS_COUNT; ++i) {
        StrCopy(aliases[i].name, "");
        StrCopy(aliases[i].value, "");
    }

    char scriptPath[0x40];
    StrCopy(scriptPath, "Data/Scripts/");
    StrAdd(scriptPath, scriptName);
    FileInfo info;
    if (LoadFile(scriptPath, &info)) {
        int readMode   = READMODE_NORMAL;
        int parseMode  = PARSEMODE_SCOPELESS;
        char prevChar  = 0;
        char curChar   = 0;
        int switchDeep = 0;
        while (readMode < READMODE_EOF) {
            int textPos = 0;
            readMode    = READMODE_NORMAL;
            while (readMode < READMODE_ENDLINE) {
                prevChar = curChar;
                FileRead(&curChar, 1);
                if (readMode == READMODE_STRING) {
                    if (curChar == '\t' || curChar == '\r' || curChar == '\n' || curChar == ';' || readMode >= READMODE_COMMENTLINE) {
                        if ((curChar == '\n' && prevChar != '\r') || (curChar == '\n' && prevChar == '\r') || curChar == ';') {
                            readMode            = READMODE_ENDLINE;
                            ScriptText[textPos] = 0;
                        }
                    } else if (curChar != '/' || textPos <= 0) {
                        ScriptText[textPos++] = curChar;
                        if (curChar == '"')
                            readMode = READMODE_NORMAL;
                    } else if (curChar == '/' && prevChar == '/') {
                        readMode              = READMODE_COMMENTLINE;
                        ScriptText[--textPos] = 0;
                    } else {
                        ScriptText[textPos++] = curChar;
                    }
                } else if (curChar == ' ' || curChar == '\t' || curChar == '\r' || curChar == '\n' || curChar == ';'
                           || readMode >= READMODE_COMMENTLINE) {
                    if ((curChar == '\n' && prevChar != '\r') || (curChar == '\n' && prevChar == '\r') || curChar == ';') {
                        readMode            = READMODE_ENDLINE;
                        ScriptText[textPos] = 0;
                    }
                } else if (curChar != '/' || textPos <= 0) {
                    ScriptText[textPos++] = curChar;
                    if (curChar == '"' && !readMode)
                        readMode = READMODE_STRING;
                } else if (curChar == '/' && prevChar == '/') {
                    readMode              = READMODE_COMMENTLINE;
                    ScriptText[--textPos] = 0;
                } else {
                    ScriptText[textPos++] = curChar;
                }
                if (ReachedEndOfFile()) {
                    ScriptText[textPos] = 0;
                    readMode            = READMODE_EOF;
                }
            }

            switch (parseMode) {
                case PARSEMODE_SCOPELESS:
                    ++lineID;
                    CheckAliasText(ScriptText);
                    if (StrComp(ScriptText, "subObjectMain")) {
                        parseMode                                        = PARSEMODE_FUNCTION;
                        ObjectScriptList[scriptID].subMain.scriptCodePtr = ScriptDataPos;
                        ObjectScriptList[scriptID].subMain.jumpTablePtr  = JumpTableDataPos;
                        scriptDataOffset                                 = ScriptDataPos;
                        JumpTableOffset                                  = JumpTableDataPos;
                        currentSub                                       = SUB_MAIN;
                    }
                    if (StrComp(ScriptText, "subObjectPlayerInteraction")) {
                        parseMode                                                     = PARSEMODE_FUNCTION;
                        ObjectScriptList[scriptID].subPlayerInteraction.scriptCodePtr = ScriptDataPos;
                        ObjectScriptList[scriptID].subPlayerInteraction.jumpTablePtr  = JumpTableDataPos;
                        scriptDataOffset                                              = ScriptDataPos;
                        JumpTableOffset                                               = JumpTableDataPos;
                        currentSub                                                    = SUB_PLAYERINTERACTION;
                    }
                    if (StrComp(ScriptText, "subObjectDraw")) {
                        parseMode                                        = PARSEMODE_FUNCTION;
                        ObjectScriptList[scriptID].subDraw.scriptCodePtr = ScriptDataPos;
                        ObjectScriptList[scriptID].subDraw.jumpTablePtr  = JumpTableDataPos;
                        scriptDataOffset                                 = ScriptDataPos;
                        JumpTableOffset                                  = JumpTableDataPos;
                        currentSub                                       = SUB_DRAW;
                    }
                    if (StrComp(ScriptText, "subObjectStartup")) {
                        parseMode                                           = PARSEMODE_FUNCTION;
                        ObjectScriptList[scriptID].subStartup.scriptCodePtr = ScriptDataPos;
                        ObjectScriptList[scriptID].subStartup.jumpTablePtr  = JumpTableDataPos;
                        scriptDataOffset                                    = ScriptDataPos;
                        JumpTableOffset                                     = JumpTableDataPos;
                        currentSub                                          = SUB_SETUP;
                    }
                    if (StrComp(ScriptText, "subPlayerMain")) {
                        parseMode                                           = PARSEMODE_FUNCTION;
                        PlayerScriptList[scriptID].scriptCodePtr_PlayerMain = ScriptDataPos;
                        PlayerScriptList[scriptID].jumpTablePtr_PlayerMain  = JumpTableDataPos;
                        scriptDataOffset                                    = ScriptDataPos;
                        JumpTableOffset                                     = JumpTableDataPos;
                        currentSub                                          = SUB_PLAYERMAIN;
                    }
                    if (!FindStringToken(ScriptText, "subPlayerState", 1)) {
                        char stateName[0x20];
                        for (textPos = 14; ScriptText[textPos]; ++textPos) stateName[textPos - 14] = ScriptText[textPos];
                        stateName[textPos - 14] = 0;
                        for (int a = 0; a < NO_ALIASES; ++a) {
                            if (StrComp(stateName, aliases[a].name))
                                StrCopy(stateName, aliases[a].value);
                        }

                        int val = 0;
                        if (ConvertStringToInteger(stateName, &val) == 1) {
                            PlayerScriptList[scriptID].scriptCodePtr_PlayerState[val] = ScriptDataPos;
                            PlayerScriptList[scriptID].jumpTablePtr_PlayerState[val]  = JumpTablePos;
                            scriptDataOffset                                          = ScriptDataPos;
                            JumpTableOffset                                           = JumpTablePos;
                            parseMode                                                 = PARSEMODE_FUNCTION;
                            currentSub                                                = SUB_PLAYERSTATE;
                        } else {
                            parseMode = PARSEMODE_SCOPELESS;
                        }
                    }
                    break;
                case PARSEMODE_PLATFORMSKIP:
                    ++lineID;
                    if (!FindStringToken(ScriptText, "{", 1))
                        parseMode = PARSEMODE_FUNCTION;
                    break;
                case PARSEMODE_FUNCTION:
                    ++lineID;
                    if (ScriptText[0]) {
                        if (StrComp(ScriptText, "endsub")) {
                            ScriptData[ScriptDataPos++] = FUNC_END;
                            parseMode                   = PARSEMODE_SCOPELESS;
                        } else {
                            ConvertIfWhileStatement(ScriptText);
                            if (ConvertSwitchStatement(ScriptText)) {
                                parseMode    = PARSEMODE_SWITCHREAD;
                                info.readPos = (int)GetFilePosition();
                                switchDeep   = 0;
                            }
                            ConvertArithmaticSyntax(ScriptText);
                            if (!ReadSwitchCase(ScriptText)) {
                                ConvertFunctionText(ScriptText);
                                if (!ScriptText[0]) {
                                    parseMode = PARSEMODE_SCOPELESS;
                                    switch (currentSub) {
                                        case SUB_MAIN: ScriptData[ObjectScriptList[scriptID].subMain.scriptCodePtr] = FUNC_END; break;
                                        case SUB_PLAYERINTERACTION:
                                            ScriptData[ObjectScriptList[scriptID].subPlayerInteraction.scriptCodePtr] = FUNC_END;
                                            break;
                                        case SUB_DRAW: ScriptData[ObjectScriptList[scriptID].subDraw.scriptCodePtr] = FUNC_END; break;
                                        case SUB_SETUP: ScriptData[ObjectScriptList[scriptID].subStartup.scriptCodePtr] = FUNC_END; break;
                                        case SUB_PLAYERMAIN: ScriptData[PlayerScriptList[scriptID].scriptCodePtr_PlayerMain] = FUNC_END; break;
                                        case SUB_PLAYERSTATE:
                                            for (int i = 0; i < 256; ++i)
                                                ScriptData[PlayerScriptList[scriptID].scriptCodePtr_PlayerState[i]] = FUNC_END;
                                            break;
                                        default: break;
                                    }
                                }
                            }
                        }
                    }
                    break;
                case PARSEMODE_SWITCHREAD:
                    if (!FindStringToken(ScriptText, "switch", 1))
                        ++switchDeep;
                    if (switchDeep) {
                        if (!FindStringToken(ScriptText, "endswitch", 1))
                            --switchDeep;
                    } else if (FindStringToken(ScriptText, "endswitch", 1)) {
                        CheckCaseNumber(ScriptText);
                    } else {
                        SetFilePosition(info.readPos);
                        parseMode  = PARSEMODE_FUNCTION;
                        int jPos   = JumpTableStack[JumpTableStackPos];
                        switchDeep = abs(JumpTableData[jPos + 1] - JumpTableData[jPos]) + 1;
                        for (textPos = 0; textPos < switchDeep; ++textPos) JumpTableData[JumpTableDataPos++] = -1;
                    }
                    break;
                default: break;
            }
        }

        CloseFile();
    }
}

void ClearScriptData() {
    memset(ScriptData, 0, SCRIPTDATA_COUNT * sizeof(int));
    memset(JumpTableData, 0, JUMPTABLE_COUNT * sizeof(int));

    ScriptFramesNo = 0;

    JumpTablePos      = 0;
    JumpTableStackPos = 0;

    ScriptDataPos    = 0;
    scriptDataOffset = 0;
    JumpTableDataPos = 0;
    JumpTableOffset  = 0;

    NO_ALIASES = COMMONALIAS_COUNT;
    lineID     = 0;

    for (int p = 0; p < PLAYER_COUNT; ++p) {
        for (int s = 0; s < 256; ++s) {
            PlayerScriptList[p].scriptCodePtr_PlayerState[s] = SCRIPTDATA_COUNT - 1;
            PlayerScriptList[p].jumpTablePtr_PlayerState[s]  = JUMPTABLE_COUNT - 1;
        }
        PlayerScriptList[p].scriptCodePtr_PlayerMain = SCRIPTDATA_COUNT - 1;
        PlayerScriptList[p].jumpTablePtr_PlayerMain  = JUMPTABLE_COUNT - 1;
    }

    for (int o = 0; o < OBJECT_COUNT; ++o) {
        ObjectScript *scriptInfo                       = &ObjectScriptList[o];
        scriptInfo->subMain.scriptCodePtr              = SCRIPTDATA_COUNT - 1;
        scriptInfo->subMain.jumpTablePtr               = JUMPTABLE_COUNT - 1;
        scriptInfo->subPlayerInteraction.scriptCodePtr = SCRIPTDATA_COUNT - 1;
        scriptInfo->subPlayerInteraction.jumpTablePtr  = JUMPTABLE_COUNT - 1;
        scriptInfo->subDraw.scriptCodePtr              = SCRIPTDATA_COUNT - 1;
        scriptInfo->subDraw.jumpTablePtr               = JUMPTABLE_COUNT - 1;
        scriptInfo->subStartup.scriptCodePtr           = SCRIPTDATA_COUNT - 1;
        scriptInfo->subStartup.jumpTablePtr            = JUMPTABLE_COUNT - 1;
        scriptInfo->frameStartPtr                      = ScriptFrames;
        scriptInfo->spriteSheetID                      = 0;
    }
}

void ProcessScript(int scriptCodePtr, int jumpTablePtr, byte scriptSub) {
    bool running      = true;
    int scriptDataPtr = scriptCodePtr;
    JumpTableStackPos = 0;
    while (running) {
        int opcode           = ScriptData[scriptDataPtr++];
        int opcodeSize       = functions[opcode].opcodeSize;
        int scriptCodeOffset = scriptDataPtr;

        // Get Valuess
        for (int i = 0; i < opcodeSize; ++i) {
            int opcodeType = ScriptData[scriptDataPtr++];

            if (opcodeType == SCRIPTVAR_VAR) {
                int arrayVal = 0;
                switch (ScriptData[scriptDataPtr++]) {
                    case VARARR_NONE: arrayVal = ObjectLoop; break;
                    case VARARR_ARRAY:
                        if (ScriptData[scriptDataPtr++] == 1)
                            arrayVal = ScriptEng.arrayPosition[ScriptData[scriptDataPtr++]];
                        else
                            arrayVal = ScriptData[scriptDataPtr++];
                        break;
                    case VARARR_ENTNOPLUS1:
                        if (ScriptData[scriptDataPtr++] == 1)
                            arrayVal = ScriptEng.arrayPosition[ScriptData[scriptDataPtr++]] + ObjectLoop;
                        else
                            arrayVal = ScriptData[scriptDataPtr++] + ObjectLoop;
                        break;
                    case VARARR_ENTNOMINUS1:
                        if (ScriptData[scriptDataPtr++] == 1)
                            arrayVal = ObjectLoop - ScriptEng.arrayPosition[ScriptData[scriptDataPtr++]];
                        else
                            arrayVal = ObjectLoop - ScriptData[scriptDataPtr++];
                        break;
                    default: break;
                }

                // Variables
                switch (ScriptData[scriptDataPtr++]) {
                    default: break;
                    case VAR_OBJECTTYPE: {
                        ScriptEng.operands[i] = ObjectEntityList[arrayVal].type;
                        break;
                    }
                    case VAR_OBJECTPROPERTYVALUE: {
                        ScriptEng.operands[i] = ObjectEntityList[arrayVal].propertyValue;
                        break;
                    }
                    case VAR_OBJECTXPOS: {
                        ScriptEng.operands[i] = ObjectEntityList[arrayVal].XPos;
                        break;
                    }
                    case VAR_OBJECTYPOS: {
                        ScriptEng.operands[i] = ObjectEntityList[arrayVal].YPos;
                        break;
                    }
                    case VAR_OBJECTIXPOS: {
                        ScriptEng.operands[i] = ObjectEntityList[arrayVal].XPos >> 16;
                        break;
                    }
                    case VAR_OBJECTIYPOS: {
                        ScriptEng.operands[i] = ObjectEntityList[arrayVal].YPos >> 16;
                        break;
                    }
                    case VAR_OBJECTSTATE: {
                        ScriptEng.operands[i] = ObjectEntityList[arrayVal].state;
                        break;
                    }
                    case VAR_OBJECTROTATION: {
                        ScriptEng.operands[i] = ObjectEntityList[arrayVal].rotation;
                        break;
                    }
                    case VAR_OBJECTSCALE: {
                        ScriptEng.operands[i] = ObjectEntityList[arrayVal].scale;
                        break;
                    }
                    case VAR_OBJECTPRIORITY: {
                        ScriptEng.operands[i] = ObjectEntityList[arrayVal].priority;
                        break;
                    }
                    case VAR_OBJECTDRAWORDER: {
                        ScriptEng.operands[i] = ObjectEntityList[arrayVal].drawOrder;
                        break;
                    }
                    case VAR_OBJECTDIRECTION: {
                        ScriptEng.operands[i] = ObjectEntityList[arrayVal].direction;
                        break;
                    }
                    case VAR_OBJECTINKEFFECT: {
                        ScriptEng.operands[i] = ObjectEntityList[arrayVal].inkEffect;
                        break;
                    }
                    case VAR_OBJECTFRAME: {
                        ScriptEng.operands[i] = ObjectEntityList[arrayVal].frame;
                        break;
                    }
                    case VAR_OBJECTVALUE0: {
                        ScriptEng.operands[i] = ObjectEntityList[arrayVal].values[0];
                        break;
                    }
                    case VAR_OBJECTVALUE1: {
                        ScriptEng.operands[i] = ObjectEntityList[arrayVal].values[1];
                        break;
                    }
                    case VAR_OBJECTVALUE2: {
                        ScriptEng.operands[i] = ObjectEntityList[arrayVal].values[2];
                        break;
                    }
                    case VAR_OBJECTVALUE3: {
                        ScriptEng.operands[i] = ObjectEntityList[arrayVal].values[3];
                        break;
                    }
                    case VAR_OBJECTVALUE4: {
                        ScriptEng.operands[i] = ObjectEntityList[arrayVal].values[4];
                        break;
                    }
                    case VAR_OBJECTVALUE5: {
                        ScriptEng.operands[i] = ObjectEntityList[arrayVal].values[5];
                        break;
                    }
                    case VAR_OBJECTVALUE6: {
                        ScriptEng.operands[i] = ObjectEntityList[arrayVal].values[6];
                        break;
                    }
                    case VAR_OBJECTVALUE7: {
                        ScriptEng.operands[i] = ObjectEntityList[arrayVal].values[7];
                        break;
                    }
                    case VAR_TEMPVALUE0: ScriptEng.operands[i] = ScriptEng.tempValue[0]; break;
                    case VAR_TEMPVALUE1: ScriptEng.operands[i] = ScriptEng.tempValue[1]; break;
                    case VAR_TEMPVALUE2: ScriptEng.operands[i] = ScriptEng.tempValue[2]; break;
                    case VAR_TEMPVALUE3: ScriptEng.operands[i] = ScriptEng.tempValue[3]; break;
                    case VAR_TEMPVALUE4: ScriptEng.operands[i] = ScriptEng.tempValue[4]; break;
                    case VAR_TEMPVALUE5: ScriptEng.operands[i] = ScriptEng.tempValue[5]; break;
                    case VAR_TEMPVALUE6: ScriptEng.operands[i] = ScriptEng.tempValue[6]; break;
                    case VAR_TEMPVALUE7: ScriptEng.operands[i] = ScriptEng.tempValue[7]; break;
                    case VAR_CHECKRESULT: ScriptEng.operands[i] = ScriptEng.checkResult; break;
                    case VAR_ARRAYPOS0: ScriptEng.operands[i] = ScriptEng.arrayPosition[0]; break;
                    case VAR_ARRAYPOS1: ScriptEng.operands[i] = ScriptEng.arrayPosition[1]; break;
                    case VAR_KEYDOWNUP: ScriptEng.operands[i] = GKeyDown.up; break;
                    case VAR_KEYDOWNDOWN: ScriptEng.operands[i] = GKeyDown.down; break;
                    case VAR_KEYDOWNLEFT: ScriptEng.operands[i] = GKeyDown.left; break;
                    case VAR_KEYDOWNRIGHT: ScriptEng.operands[i] = GKeyDown.right; break;
                    case VAR_KEYDOWNBUTTONA: ScriptEng.operands[i] = GKeyDown.A; break;
                    case VAR_KEYDOWNBUTTONB: ScriptEng.operands[i] = GKeyDown.B; break;
                    case VAR_KEYDOWNBUTTONC: ScriptEng.operands[i] = GKeyDown.C; break;
                    case VAR_KEYDOWNSTART: ScriptEng.operands[i] = GKeyDown.start; break;
                    case VAR_KEYPRESSUP: ScriptEng.operands[i] = GKeyPress.up; break;
                    case VAR_KEYPRESSDOWN: ScriptEng.operands[i] = GKeyPress.down; break;
                    case VAR_KEYPRESSLEFT: ScriptEng.operands[i] = GKeyPress.left; break;
                    case VAR_KEYPRESSRIGHT: ScriptEng.operands[i] = GKeyPress.right; break;
                    case VAR_KEYPRESSBUTTONA: ScriptEng.operands[i] = GKeyPress.A; break;
                    case VAR_KEYPRESSBUTTONB: ScriptEng.operands[i] = GKeyPress.B; break;
                    case VAR_KEYPRESSBUTTONC: ScriptEng.operands[i] = GKeyPress.C; break;
                    case VAR_KEYPRESSSTART: ScriptEng.operands[i] = GKeyPress.start; break;
                    case VAR_MENU1SELECTION: ScriptEng.operands[i] = GameMenu[0].selection1; break;
                    case VAR_MENU2SELECTION: ScriptEng.operands[i] = GameMenu[1].selection1; break;
                    case VAR_STAGEACTIVELIST: ScriptEng.operands[i] = ActiveStageList; break;
                    case VAR_STAGELISTPOS: ScriptEng.operands[i] = StageListPosition; break;
                    case VAR_XSCROLLOFFSET: ScriptEng.operands[i] = XScrollOffset; break;
                    case VAR_YSCROLLOFFSET: ScriptEng.operands[i] = YScrollOffset; break;
                    case VAR_GLOBAL: ScriptEng.operands[i] = GlobalVariables[arrayVal]; break;
                    case VAR_STAGETIMEENABLED: ScriptEng.operands[i] = TimeEnabled; break;
                    case VAR_STAGEMILLISECONDS: ScriptEng.operands[i] = MilliSeconds; break;
                    case VAR_STAGESECONDS: ScriptEng.operands[i] = Seconds; break;
                    case VAR_STAGEMINUTES: ScriptEng.operands[i] = Minutes; break;
                    case VAR_STAGEACTNO: ScriptEng.operands[i] = ActNumber; break;
                    case VAR_OBJECTENTITYNO: ScriptEng.operands[i] = arrayVal; break;
                    case VAR_PLAYERTYPE: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].type;
                        break;
                    }
                    case VAR_PLAYERSTATE: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].state;
                        break;
                    }
                    case VAR_PLAYERCONTROLMODE: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].controlMode;
                        break;
                    }
                    case VAR_PLAYERCOLLISIONMODE: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].collisionMode;
                        break;
                    }
                    case VAR_PLAYERCOLLISIONPLANE: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].collisionPlane;
                        break;
                    }
                    case VAR_PLAYERXPOS: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].XPos;
                        break;
                    }
                    case VAR_PLAYERYPOS: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].YPos;
                        break;
                    }
                    case VAR_PLAYERSCREENXPOS: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].screenXPos;
                        break;
                    }
                    case VAR_PLAYERSCREENYPOS: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].screenYPos;
                        break;
                    }
                    case VAR_PLAYERSPEED: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].speed;
                        break;
                    }
                    case VAR_PLAYERXVELOCITY: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].XVelocity;
                        break;
                    }
                    case VAR_PLAYERYVELOCITY: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].YVelocity;
                        break;
                    }
                    case VAR_PLAYERGRAVITY: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].gravity;
                        break;
                    }
                    case VAR_PLAYERANGLE: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].angle;
                        break;
                    }
                    case VAR_PLAYERSKIDDING: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].skidding;
                        break;
                    }
                    case VAR_PLAYERPUSHING: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].pushing;
                        break;
                    }
                    case VAR_PLAYERFRICTIONLOSS: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].frictionLoss;
                        break;
                    }
                    case VAR_PLAYERWALKINGSPEED: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].walkingSpeed;
                        break;
                    }
                    case VAR_PLAYERRUNNINGSPEED: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].runningSpeed;
                        break;
                    }
                    case VAR_PLAYERJUMPINGSPEED: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].jumpingSpeed;
                        break;
                    }
                    case VAR_PLAYERTRACKSCROLL: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].trackScroll;
                        break;
                    }
                    case VAR_PLAYERUP: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].up;
                        break;
                    }
                    case VAR_PLAYERDOWN: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].down;
                        break;
                    }
                    case VAR_PLAYERLEFT: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].left;
                        break;
                    }
                    case VAR_PLAYERRIGHT: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].right;
                        break;
                    }
                    case VAR_PLAYERJUMPPRESS: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].jumpPress;
                        break;
                    }
                    case VAR_PLAYERJUMPHOLD: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].jumpHold;
                        break;
                    }
                    case VAR_PLAYERFOLLOWPLAYER1: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].followPlayer1;
                        break;
                    }
                    case VAR_PLAYERLOOKPOS: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].lookPos;
                        break;
                    }
                    case VAR_PLAYERWATER: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].water;
                        break;
                    }
                    case VAR_PLAYERTOPSPEED: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].stats.topSpeed;
                        break;
                    }
                    case VAR_PLAYERACCELERATION: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].stats.acceleration;
                        break;
                    }
                    case VAR_PLAYERDECELERATION: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].stats.deceleration;
                        break;
                    }
                    case VAR_PLAYERAIRACCELERATION: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].stats.airAcceleration;
                        break;
                    }
                    case VAR_PLAYERAIRDECELERATION: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].stats.airDeceleration;
                        break;
                    }
                    case VAR_PLAYERGRAVITYSTRENGTH: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].stats.gravityStrength;
                        break;
                    }
                    case VAR_PLAYERJUMPSTRENGTH: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].stats.jumpStrength;
                        break;
                    }
                    case VAR_PLAYERROLLINGACCELERATION: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].stats.rollingAcceleration;
                        break;
                    }
                    case VAR_PLAYERROLLINGDECELERATION: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].stats.rollingDeceleration;
                        break;
                    }
                    case VAR_PLAYERENTITYNO: {
                        ScriptEng.operands[i] = PlayerNo;
                        break;
                    }
                    case VAR_PLAYERCOLLISIONLEFT: {
                        ScriptEng.operands[i] = GetPlayerCBox(&PlayerScriptList[PlayerNo])->left[0];
                        break;
                    }
                    case VAR_PLAYERCOLLISIONTOP: {
                        ScriptEng.operands[i] = GetPlayerCBox(&PlayerScriptList[PlayerNo])->top[0];
                        break;
                    }
                    case VAR_PLAYERCOLLISIONRIGHT: {
                        ScriptEng.operands[i] = GetPlayerCBox(&PlayerScriptList[PlayerNo])->right[0];
                        break;
                    }
                    case VAR_PLAYERCOLLISIONBOTTOM: {
                        ScriptEng.operands[i] = GetPlayerCBox(&PlayerScriptList[PlayerNo])->bottom[0];
                        break;
                    }
                    case VAR_PLAYERFLAILING: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].flailing[arrayVal];
                        break;
                    }
                    case VAR_PLAYERTIMER: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].timer;
                        break;
                    }
                    case VAR_PLAYERTILECOLLISIONS: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].tileCollisions;
                        break;
                    }
                    case VAR_PLAYEROBJECTINTERACTION: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].objectInteraction;
                        break;
                    }
                    case VAR_PLAYERROTATION: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].rotation;
                        break;
                    }
                    case VAR_PLAYERDIRECTION: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].direction;
                        break;
                    }
                    case VAR_PLAYERFRAME: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].frame;
                        break;
                    }
                    case VAR_PLAYERANIMATION: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].animation;
                        break;
                    }
                    case VAR_PLAYERANIMATIONSPEED: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].animationSpeed;
                        break;
                    }
                    case VAR_STAGEPAUSEENABLED: ScriptEng.operands[i] = PauseEnabled; break;
                    case VAR_STAGELISTSIZE: ScriptEng.operands[i] = stageListCount[ActiveStageList]; break;
                    case VAR_SCREENCAMERAENABLED: ScriptEng.operands[i] = CameraEnabled; break;
                    case VAR_SCREENCAMERASTYLE: ScriptEng.operands[i] = CameraStyle; break;
                    case VAR_MUSICVOLUME: ScriptEng.operands[i] = MusicVolume; break;
                    case VAR_MUSICCURRENTTRACK: ScriptEng.operands[i] = CurrentMusicTrack; break;
                    case VAR_PLAYERVISIBLE: {
                        ScriptEng.operands[i] = PlayerList[PlayerNo].visible;
                        break;
                    }
                    case VAR_STAGENEWXBOUNDARY1: ScriptEng.operands[i] = NewXBoundary1; break;
                    case VAR_STAGENEWXBOUNDARY2: ScriptEng.operands[i] = NewXBoundary2; break;
                    case VAR_STAGENEWYBOUNDARY1: ScriptEng.operands[i] = NewYBoundary1; break;
                    case VAR_STAGENEWYBOUNDARY2: ScriptEng.operands[i] = NewYBoundary2; break;
                    case VAR_STAGEXBOUNDARY1: ScriptEng.operands[i] = XBoundary1; break;
                    case VAR_STAGEXBOUNDARY2: ScriptEng.operands[i] = XBoundary2; break;
                    case VAR_STAGEYBOUNDARY1: ScriptEng.operands[i] = YBoundary1; break;
                    case VAR_STAGEYBOUNDARY2: ScriptEng.operands[i] = YBoundary2; break;
                    case VAR_OBJECTOUTOFBOUNDS: {
                        int pos = ObjectEntityList[arrayVal].XPos >> 16;
                        if (pos <= XScrollOffset - OBJECT_BORDER_X1 || pos >= OBJECT_BORDER_X2 + XScrollOffset) {
                            ScriptEng.operands[i] = 1;
                        } else {
                            int pos               = ObjectEntityList[arrayVal].YPos >> 16;
                            ScriptEng.operands[i] = pos <= YScrollOffset - OBJECT_BORDER_Y1 || pos >= YScrollOffset + OBJECT_BORDER_Y2;
                        }
                        break;
                    }
                }
            } else if (opcodeType == SCRIPTVAR_INTCONST) { // int constant
                ScriptEng.operands[i] = ScriptData[scriptDataPtr++];
            } else if (opcodeType == SCRIPTVAR_STRCONST) { // string constant
                int strLen         = ScriptData[scriptDataPtr++];
                ScriptText[strLen] = 0;
                for (int c = 0; c < strLen; ++c) {
                    switch (c % 4) {
                        case 0: {
                            ScriptText[c] = ScriptData[scriptDataPtr] >> 24;
                            break;
                        }
                        case 1: {
                            ScriptText[c] = (0x00FFFFFF & ScriptData[scriptDataPtr]) >> 16;
                            break;
                        }
                        case 2: {
                            ScriptText[c] = (0x0000FFFF & ScriptData[scriptDataPtr]) >> 8;
                            break;
                        }
                        case 3: {
                            ScriptText[c] = (0x000000FF & ScriptData[scriptDataPtr++]) >> 0;
                            break;
                        }
                        default: break;
                    }
                }
                scriptDataPtr++;
            }
        }

        ObjectScript *scriptInfo = &ObjectScriptList[ObjectEntityList[ObjectLoop].type];
        Entity *entity           = &ObjectEntityList[ObjectLoop];
        Player *player           = &PlayerList[PlayerNo];
        SpriteFrame *spriteFrame = nullptr;

        // Functions
        switch (opcode) {
            default: break;
            case FUNC_END: running = false; break;
            case FUNC_EQUAL: ScriptEng.operands[0] = ScriptEng.operands[1]; break;
            case FUNC_ADD: ScriptEng.operands[0] += ScriptEng.operands[1]; break;
            case FUNC_SUB: ScriptEng.operands[0] -= ScriptEng.operands[1]; break;
            case FUNC_INC: ++ScriptEng.operands[0]; break;
            case FUNC_DEC: --ScriptEng.operands[0]; break;
            case FUNC_MUL: ScriptEng.operands[0] *= ScriptEng.operands[1]; break;
            case FUNC_DIV: ScriptEng.operands[0] /= ScriptEng.operands[1]; break;
            case FUNC_SHR: ScriptEng.operands[0] >>= ScriptEng.operands[1]; break;
            case FUNC_SHL: ScriptEng.operands[0] <<= ScriptEng.operands[1]; break;
            case FUNC_AND: ScriptEng.operands[0] &= ScriptEng.operands[1]; break;
            case FUNC_OR: ScriptEng.operands[0] |= ScriptEng.operands[1]; break;
            case FUNC_XOR: ScriptEng.operands[0] ^= ScriptEng.operands[1]; break;
            case FUNC_NOT: ScriptEng.operands[0] = ~ScriptEng.operands[0]; break;
            case FUNC_FLIPSIGN: ScriptEng.operands[0] = -ScriptEng.operands[0]; break;
            case FUNC_CHECKEQUAL:
                ScriptEng.checkResult = ScriptEng.operands[0] == ScriptEng.operands[1];
                opcodeSize            = 0;
                break;
            case FUNC_CHECKGREATER:
                ScriptEng.checkResult = ScriptEng.operands[0] > ScriptEng.operands[1];
                opcodeSize            = 0;
                break;
            case FUNC_CHECKLOWER:
                ScriptEng.checkResult = ScriptEng.operands[0] < ScriptEng.operands[1];
                opcodeSize            = 0;
                break;
            case FUNC_CHECKNOTEQUAL:
                ScriptEng.checkResult = ScriptEng.operands[0] != ScriptEng.operands[1];
                opcodeSize            = 0;
                break;
            case FUNC_IFEQUAL:
                if (ScriptEng.operands[1] != ScriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + JumpTableData[jumpTablePtr + ScriptEng.operands[0]];
                JumpTableStack[++JumpTableStackPos] = ScriptEng.operands[0];
                opcodeSize                          = 0;
                break;
            case FUNC_IFGREATER:
                if (ScriptEng.operands[1] <= ScriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + JumpTableData[jumpTablePtr + ScriptEng.operands[0]];
                JumpTableStack[++JumpTableStackPos] = ScriptEng.operands[0];
                opcodeSize                          = 0;
                break;
            case FUNC_IFGREATEROREQUAL:
                if (ScriptEng.operands[1] < ScriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + JumpTableData[jumpTablePtr + ScriptEng.operands[0]];
                JumpTableStack[++JumpTableStackPos] = ScriptEng.operands[0];
                opcodeSize                          = 0;
                break;
            case FUNC_IFLOWER:
                if (ScriptEng.operands[1] >= ScriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + JumpTableData[jumpTablePtr + ScriptEng.operands[0]];
                JumpTableStack[++JumpTableStackPos] = ScriptEng.operands[0];
                opcodeSize                          = 0;
                break;
            case FUNC_IFLOWEROREQUAL:
                if (ScriptEng.operands[1] > ScriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + JumpTableData[jumpTablePtr + ScriptEng.operands[0]];
                JumpTableStack[++JumpTableStackPos] = ScriptEng.operands[0];
                opcodeSize                          = 0;
                break;
            case FUNC_IFNOTEQUAL:
                if (ScriptEng.operands[1] == ScriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + JumpTableData[jumpTablePtr + ScriptEng.operands[0]];
                JumpTableStack[++JumpTableStackPos] = ScriptEng.operands[0];
                opcodeSize                          = 0;
                break;
            case FUNC_ELSE:
                opcodeSize    = 0;
                scriptDataPtr = scriptCodePtr + JumpTableData[jumpTablePtr + JumpTableStack[JumpTableStackPos--] + 1];
                break;
            case FUNC_ENDIF:
                opcodeSize = 0;
                --JumpTableStackPos;
                break;
            case FUNC_WEQUAL:
                if (ScriptEng.operands[1] != ScriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + JumpTableData[jumpTablePtr + ScriptEng.operands[0] + 1];
                else
                    JumpTableStack[++JumpTableStackPos] = ScriptEng.operands[0];
                opcodeSize = 0;
                break;
            case FUNC_WGREATER:
                if (ScriptEng.operands[1] <= ScriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + JumpTableData[jumpTablePtr + ScriptEng.operands[0] + 1];
                else
                    JumpTableStack[++JumpTableStackPos] = ScriptEng.operands[0];
                opcodeSize = 0;
                break;
            case FUNC_WGREATEROREQUAL:
                if (ScriptEng.operands[1] < ScriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + JumpTableData[jumpTablePtr + ScriptEng.operands[0] + 1];
                else
                    JumpTableStack[++JumpTableStackPos] = ScriptEng.operands[0];
                opcodeSize = 0;
                break;
            case FUNC_WLOWER:
                if (ScriptEng.operands[1] >= ScriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + JumpTableData[jumpTablePtr + ScriptEng.operands[0] + 1];
                else
                    JumpTableStack[++JumpTableStackPos] = ScriptEng.operands[0];
                opcodeSize = 0;
                break;
            case FUNC_WLOWEROREQUAL:
                if (ScriptEng.operands[1] > ScriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + JumpTableData[jumpTablePtr + ScriptEng.operands[0] + 1];
                else
                    JumpTableStack[++JumpTableStackPos] = ScriptEng.operands[0];
                opcodeSize = 0;
                break;
            case FUNC_WNOTEQUAL:
                if (ScriptEng.operands[1] == ScriptEng.operands[2])
                    scriptDataPtr = scriptCodePtr + JumpTableData[jumpTablePtr + ScriptEng.operands[0] + 1];
                else
                    JumpTableStack[++JumpTableStackPos] = ScriptEng.operands[0];
                opcodeSize = 0;
                break;
            case FUNC_LOOP:
                opcodeSize    = 0;
                scriptDataPtr = scriptCodePtr + JumpTableData[jumpTablePtr + JumpTableStack[JumpTableStackPos--]];
                break;
            case FUNC_SWITCH:
                JumpTableStack[++JumpTableStackPos] = ScriptEng.operands[0];
                if (ScriptEng.operands[1] < JumpTableData[jumpTablePtr + ScriptEng.operands[0]]
                    || ScriptEng.operands[1] > JumpTableData[jumpTablePtr + ScriptEng.operands[0] + 1])
                    scriptDataPtr = scriptCodePtr + JumpTableData[jumpTablePtr + ScriptEng.operands[0] + 2];
                else
                    scriptDataPtr = scriptCodePtr
                                    + JumpTableData[jumpTablePtr + ScriptEng.operands[0] + 4
                                                    + (ScriptEng.operands[1] - JumpTableData[jumpTablePtr + ScriptEng.operands[0]])];
                opcodeSize = 0;
                break;
            case FUNC_BREAK:
                opcodeSize    = 0;
                scriptDataPtr = scriptCodePtr + JumpTableData[jumpTablePtr + JumpTableStack[JumpTableStackPos--] + 3];
                break;
            case FUNC_ENDSWITCH:
                opcodeSize = 0;
                --JumpTableStackPos;
                break;
            case FUNC_RAND: ScriptEng.operands[0] = rand() % ScriptEng.operands[1]; break;
            case FUNC_SIN: {
                ScriptEng.operands[0] = Sin512(ScriptEng.operands[1]);
                break;
            }
            case FUNC_COS: {
                ScriptEng.operands[0] = Cos512(ScriptEng.operands[1]);
                break;
            }
            case FUNC_SIN256: {
                ScriptEng.operands[0] = Sin256(ScriptEng.operands[1]);
                break;
            }
            case FUNC_COS256: {
                ScriptEng.operands[0] = Cos256(ScriptEng.operands[1]);
                break;
            }
            case FUNC_SINCHANGE: {
                ScriptEng.operands[0] = ScriptEng.operands[3] + (Sin512(ScriptEng.operands[1]) >> ScriptEng.operands[2]) - ScriptEng.operands[4];
                break;
            }
            case FUNC_COSCHANGE: {
                ScriptEng.operands[0] = ScriptEng.operands[3] + (Cos512(ScriptEng.operands[1]) >> ScriptEng.operands[2]) - ScriptEng.operands[4];
                break;
            }
            case FUNC_ATAN2: {
                opcodeSize = 0;
                // doesn't exist
                break;
            }
            case FUNC_INTERPOLATE:
                ScriptEng.operands[0] =
                    (ScriptEng.operands[2] * (0x100 - ScriptEng.operands[3]) + ScriptEng.operands[3] * ScriptEng.operands[1]) >> 8;
                break;
            case FUNC_INTERPOLATEXY:
                ScriptEng.operands[0] =
                    (ScriptEng.operands[3] * (0x100 - ScriptEng.operands[6]) >> 8) + ((ScriptEng.operands[6] * ScriptEng.operands[2]) >> 8);
                ScriptEng.operands[1] =
                    (ScriptEng.operands[5] * (0x100 - ScriptEng.operands[6]) >> 8) + (ScriptEng.operands[6] * ScriptEng.operands[4] >> 8);
                break;
            case FUNC_LOADSPRITESHEET:
                opcodeSize                = 0;
                scriptInfo->spriteSheetID = AddGraphicsFile(ScriptText);
                break;
            case FUNC_REMOVESPRITESHEET:
                opcodeSize = 0;
                RemoveGraphicsFile(ScriptText, -1);
                break;
            case FUNC_DRAWSPRITE:
                opcodeSize  = 0;
                spriteFrame = &scriptInfo->frameStartPtr[ScriptEng.operands[0]];
                DrawSprite((entity->XPos >> 16) - XScrollOffset + spriteFrame->pivotX, (entity->YPos >> 16) - YScrollOffset + spriteFrame->pivotY,
                           spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                break;
            case FUNC_DRAWSPRITEXY:
                opcodeSize  = 0;
                spriteFrame = &scriptInfo->frameStartPtr[ScriptEng.operands[0]];
                DrawSprite((ScriptEng.operands[1] >> 16) - XScrollOffset + spriteFrame->pivotX,
                           (ScriptEng.operands[2] >> 16) - YScrollOffset + spriteFrame->pivotY, spriteFrame->width, spriteFrame->height,
                           spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                break;
            case FUNC_DRAWSPRITESCREENXY:
                opcodeSize  = 0;
                spriteFrame = &scriptInfo->frameStartPtr[ScriptEng.operands[0]];
                DrawSprite(ScriptEng.operands[1] + spriteFrame->pivotX, ScriptEng.operands[2] + spriteFrame->pivotY, spriteFrame->width,
                           spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                break;
            case FUNC_DRAWSPRITE3D:
                opcodeSize = 0;
                // Does not exist
                break;
            case FUNC_DRAWNUMBERS: {
                opcodeSize = 0;
                int i      = 10;
                if (ScriptEng.operands[6]) {
                    while (ScriptEng.operands[4] > 0) {
                        int frameID = ScriptEng.operands[3] % i / (i / 10) + ScriptEng.operands[0];
                        spriteFrame = &scriptInfo->frameStartPtr[frameID];
                        DrawSprite(spriteFrame->pivotX + ScriptEng.operands[1], spriteFrame->pivotY + ScriptEng.operands[2], spriteFrame->width,
                                   spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                        ScriptEng.operands[1] -= ScriptEng.operands[5];
                        i *= 10;
                        --ScriptEng.operands[4];
                    }
                } else {
                    int extra = 10;
                    if (ScriptEng.operands[3])
                        extra = 10 * ScriptEng.operands[3];
                    while (ScriptEng.operands[4] > 0) {
                        if (extra >= i) {
                            int frameID = ScriptEng.operands[3] % i / (i / 10) + ScriptEng.operands[0];
                            spriteFrame = &scriptInfo->frameStartPtr[frameID];
                            DrawSprite(spriteFrame->pivotX + ScriptEng.operands[1], spriteFrame->pivotY + ScriptEng.operands[2], spriteFrame->width,
                                       spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                        }
                        ScriptEng.operands[1] -= ScriptEng.operands[5];
                        i *= 10;
                        --ScriptEng.operands[4];
                    }
                }
                break;
            }
            case FUNC_DRAWACTNAME: {
                opcodeSize = 0;
                int charID = 0;

                switch (ScriptEng.operands[3]) {
                    default: break;

                    case 1: // Draw Word 1
                        charID = 0;

                        // Draw the first letter as a capital letter, the rest are lowercase (if ScriptEng.operands[4] is true, otherwise they're all
                        // uppercase)
                        if (ScriptEng.operands[4] == 1 && titleCardText[charID] != 0) {
                            int character = titleCardText[charID];
                            if (character == ' ')
                                character = 0;
                            if (character == '-')
                                character = 0;
                            if (character >= '0' && character <= '9')
                                character -= 22;
                            if (character > '9' && character < 'f')
                                character -= 'A';

                            if (character <= -1) {
                                ScriptEng.operands[1] += ScriptEng.operands[5] + ScriptEng.operands[6];
                            } else {
                                character += ScriptEng.operands[0];
                                spriteFrame = &scriptInfo->frameStartPtr[character];
                                DrawSprite(ScriptEng.operands[1] + spriteFrame->pivotX, ScriptEng.operands[2] + spriteFrame->pivotY,
                                           spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                                ScriptEng.operands[1] += spriteFrame->width + ScriptEng.operands[6];
                            }

                            ScriptEng.operands[0] += 26;
                            charID++;
                        }

                        while (titleCardText[charID] != 0 && titleCardText[charID] != '-') {
                            int character = titleCardText[charID];
                            if (character == ' ')
                                character = 0;
                            if (character == '-')
                                character = 0;
                            if (character > '/' && character < ':')
                                character -= 22;
                            if (character > '9' && character < 'f')
                                character -= 'A';
                            if (character <= -1) {
                                ScriptEng.operands[1] += ScriptEng.operands[5] + ScriptEng.operands[6];
                            } else {
                                character += ScriptEng.operands[0];
                                spriteFrame = &scriptInfo->frameStartPtr[character];
                                DrawSprite(ScriptEng.operands[1] + spriteFrame->pivotX, ScriptEng.operands[2] + spriteFrame->pivotY,
                                           spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                                ScriptEng.operands[1] += spriteFrame->width + ScriptEng.operands[6];
                            }
                            charID++;
                        }
                        break;

                    case 2: // Draw Word 2
                        charID = titleCardWord2;

                        // Draw the first letter as a capital letter, the rest are lowercase (if ScriptEng.operands[4] is true, otherwise they're all
                        // uppercase)
                        if (ScriptEng.operands[4] == 1 && titleCardText[charID] != 0) {
                            int character = titleCardText[charID];
                            if (character == ' ')
                                character = 0;
                            if (character == '-')
                                character = 0;
                            if (character >= '0' && character <= '9')
                                character -= 22;
                            if (character > '9' && character < 'f')
                                character -= 'A';
                            if (character <= -1) {
                                ScriptEng.operands[1] += ScriptEng.operands[5] + ScriptEng.operands[6];
                            } else {
                                character += ScriptEng.operands[0];
                                spriteFrame = &scriptInfo->frameStartPtr[character];
                                DrawSprite(ScriptEng.operands[1] + spriteFrame->pivotX, ScriptEng.operands[2] + spriteFrame->pivotY,
                                           spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                                ScriptEng.operands[1] += spriteFrame->width + ScriptEng.operands[6];
                            }
                            ScriptEng.operands[0] += 26;
                            charID++;
                        }

                        while (titleCardText[charID] != 0) {
                            int character = titleCardText[charID];
                            if (character == ' ')
                                character = 0;
                            if (character == '-')
                                character = 0;
                            if (character >= '0' && character <= '9')
                                character -= 22;
                            if (character > '9' && character < 'f')
                                character -= 'A';
                            if (character <= -1) {
                                ScriptEng.operands[1] += ScriptEng.operands[5] + ScriptEng.operands[6];
                            } else {
                                character += ScriptEng.operands[0];
                                spriteFrame = &scriptInfo->frameStartPtr[character];
                                DrawSprite(ScriptEng.operands[1] + spriteFrame->pivotX, ScriptEng.operands[2] + spriteFrame->pivotY,
                                           spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                                ScriptEng.operands[1] += spriteFrame->width + ScriptEng.operands[6];
                            }
                            charID++;
                        }
                        break;
                }
                break;
            }
            case FUNC_DRAWMENU:
                opcodeSize        = 0;
                TextMenuSurfaceNo = scriptInfo->spriteSheetID;
                DrawTextMenu(&GameMenu[ScriptEng.operands[0]], ScriptEng.operands[1], ScriptEng.operands[2]);
                break;
            case FUNC_SPRITEFRAME:
                opcodeSize = 0;
                if (scriptSub == SUB_SETUP && ScriptFramesNo < SPRITEFRAME_COUNT) {
                    ScriptFrames[ScriptFramesNo].pivotX = ScriptEng.operands[0];
                    ScriptFrames[ScriptFramesNo].pivotY = ScriptEng.operands[1];
                    ScriptFrames[ScriptFramesNo].width  = ScriptEng.operands[2];
                    ScriptFrames[ScriptFramesNo].height = ScriptEng.operands[3];
                    ScriptFrames[ScriptFramesNo].sprX   = ScriptEng.operands[4];
                    ScriptFrames[ScriptFramesNo].sprY   = ScriptEng.operands[5];
                    ++ScriptFramesNo;
                }
                break;
            case FUNC_SETDEBUGICON: opcodeSize = 0; break;
            case FUNC_LOADPALETTE:
                opcodeSize = 0;
                LoadPalette(ScriptText, ScriptEng.operands[1], ScriptEng.operands[2]);
                break;
            case FUNC_ROTATEPALETTE:
                opcodeSize = 0;
                RotatePalette(ScriptEng.operands[0], ScriptEng.operands[1], ScriptEng.operands[2]);
                break;
            case FUNC_SETFADE:
                opcodeSize = 0;
                SetFade(ScriptEng.operands[0], ScriptEng.operands[1], ScriptEng.operands[2], ScriptEng.operands[3], ScriptEng.operands[4],
                        ScriptEng.operands[5]);
                break;
            case FUNC_SETWATERCOLOR:
                opcodeSize = 0;
                // Exists but never called
                break;
            case FUNC_SETBLENDTABLE:
                opcodeSize = 0;
                GenerateBlendTable(ScriptEng.operands[0], ScriptEng.operands[1], ScriptEng.operands[2], ScriptEng.operands[3]);
                break;
            case FUNC_SETTINTTABLE:
                opcodeSize = 0;
                GenerateTintTable(ScriptEng.operands[0], ScriptEng.operands[1], ScriptEng.operands[2], ScriptEng.operands[3], ScriptEng.operands[4],
                             ScriptEng.operands[5]);
                break;
            case FUNC_CLEARSCREEN:
                opcodeSize = 0;
                ClearScreen(ScriptEng.operands[0]);
                break;
            case FUNC_DRAWSPRITEFX:
                opcodeSize  = 0;
                spriteFrame = &scriptInfo->frameStartPtr[ScriptEng.operands[0]];
                switch (ScriptEng.operands[1]) {
                    default: break;
                    case FX_SCALE:
                        DrawScaledSprite(entity->direction, (ScriptEng.operands[2] >> 16) - XScrollOffset,
                                         (ScriptEng.operands[3] >> 16) - YScrollOffset, -spriteFrame->pivotX, -spriteFrame->pivotY, entity->scale,
                                         entity->scale, spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY,
                                         scriptInfo->spriteSheetID);
                        break;
                    case FX_ROTATE:
                        DrawRotatedSprite(entity->direction, (ScriptEng.operands[2] >> 16) - XScrollOffset,
                                          (ScriptEng.operands[3] >> 16) - YScrollOffset, -spriteFrame->pivotX, -spriteFrame->pivotY,
                                          spriteFrame->sprX, spriteFrame->sprY, spriteFrame->width, spriteFrame->height, entity->rotation,
                                          scriptInfo->spriteSheetID);
                        break;
                    case FX_INK:
                        switch (entity->inkEffect) {
                            case INK_NONE:
                                DrawSprite((ScriptEng.operands[2] >> 16) - XScrollOffset + spriteFrame->pivotX,
                                           (ScriptEng.operands[3] >> 16) - YScrollOffset + spriteFrame->pivotY, spriteFrame->width,
                                           spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                                break;
                            case INK_BLEND:
                                DrawBlendedSprite((ScriptEng.operands[2] >> 16) - XScrollOffset + spriteFrame->pivotX,
                                                  (ScriptEng.operands[3] >> 16) - YScrollOffset + spriteFrame->pivotY, spriteFrame->width,
                                                  spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                                break;
                        }
                        break;
                    case FX_TINT:
                        if (entity->inkEffect == INK_TINT) {
                            DrawScaledTintMask(entity->direction, (ScriptEng.operands[2] >> 16) - XScrollOffset,
                                               (ScriptEng.operands[3] >> 16) - YScrollOffset, -spriteFrame->pivotX, -spriteFrame->pivotY,
                                               entity->scale, entity->scale, spriteFrame->width, spriteFrame->height, spriteFrame->sprX,
                                               spriteFrame->sprY, 0, scriptInfo->spriteSheetID);
                        } else {
                            DrawScaledSprite(entity->direction, (ScriptEng.operands[2] >> 16) - XScrollOffset,
                                             (ScriptEng.operands[3] >> 16) - YScrollOffset, -spriteFrame->pivotX, -spriteFrame->pivotY, entity->scale,
                                             entity->scale, spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY,
                                             scriptInfo->spriteSheetID);
                        }
                        break;
                }
                break;
            case FUNC_DRAWSPRITESCREENFX:
                opcodeSize  = 0;
                spriteFrame = &scriptInfo->frameStartPtr[ScriptEng.operands[0]];
                switch (ScriptEng.operands[1]) {
                    default: break;
                    case FX_SCALE:
                        DrawScaledSprite(entity->direction, ScriptEng.operands[2], ScriptEng.operands[3], -spriteFrame->pivotX, -spriteFrame->pivotY,
                                         entity->scale, entity->scale, spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY,
                                         scriptInfo->spriteSheetID);
                        break;
                    case FX_ROTATE:
                        DrawRotatedSprite(entity->direction, ScriptEng.operands[2], ScriptEng.operands[3], -spriteFrame->pivotX, -spriteFrame->pivotY,
                                          spriteFrame->sprX, spriteFrame->sprY, spriteFrame->width, spriteFrame->height, entity->rotation,
                                          scriptInfo->spriteSheetID);
                        break;
                    case FX_INK:
                        switch (entity->inkEffect) {
                            case INK_NONE:
                                DrawSprite(ScriptEng.operands[2] + spriteFrame->pivotX, ScriptEng.operands[3] + spriteFrame->pivotY,
                                           spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                                break;
                            case INK_BLEND:
                                DrawBlendedSprite(ScriptEng.operands[2] + spriteFrame->pivotX, ScriptEng.operands[3] + spriteFrame->pivotY,
                                                  spriteFrame->width, spriteFrame->height, spriteFrame->sprX, spriteFrame->sprY,
                                                  scriptInfo->spriteSheetID);
                                break;
                        }
                        break;
                    case FX_TINT:
                        if (entity->inkEffect == INK_TINT) {
                            DrawScaledTintMask(entity->direction, ScriptEng.operands[2], ScriptEng.operands[3], -spriteFrame->pivotX,
                                               -spriteFrame->pivotY, entity->scale, entity->scale, spriteFrame->width, spriteFrame->height,
                                               spriteFrame->sprX, spriteFrame->sprY, 0, scriptInfo->spriteSheetID);
                        } else {
                            DrawScaledSprite(entity->direction, ScriptEng.operands[2], ScriptEng.operands[3], -spriteFrame->pivotX,
                                             -spriteFrame->pivotY, entity->scale, entity->scale, spriteFrame->width, spriteFrame->height,
                                             spriteFrame->sprX, spriteFrame->sprY, scriptInfo->spriteSheetID);
                        }
                        break;
                }
                break;
            case FUNC_DRAWLIFEICON: {
                opcodeSize            = 0;
                SpriteAnimation *anim = &PlayerScriptList[PlayerList[0].type].animations[ANI_LIFEICON];

                DrawSprite(anim->frames[0].pivotX + ScriptEng.operands[0], anim->frames[0].pivotY + ScriptEng.operands[1], anim->frames[0].width,
                           anim->frames[0].height, anim->frames[0].sprX, anim->frames[0].sprY, anim->frames[0].sheetID);
                break;
            }
            case FUNC_SETUPMENU: {
                opcodeSize     = 0;
                TextMenu *menu = &GameMenu[ScriptEng.operands[0]];
                SetupTextMenu(menu, ScriptEng.operands[1]);
                menu->selectionCount = ScriptEng.operands[2];
                menu->alignment      = ScriptEng.operands[3];
                break;
            }
            case FUNC_ADDMENUENTRY: {
                opcodeSize                           = 0;
                TextMenu *menu                       = &GameMenu[ScriptEng.operands[0]];
                menu->entryHighlight[menu->rowCount] = ScriptEng.operands[2];
                AddTextMenuEntry(menu, ScriptText);
                break;
            }
            case FUNC_EDITMENUENTRY: {
                opcodeSize     = 0;
                TextMenu *menu = &GameMenu[ScriptEng.operands[0]];
                EditTextMenuEntry(menu, ScriptText, ScriptEng.operands[2]);
                menu->entryHighlight[ScriptEng.operands[2]] = ScriptEng.operands[3];
                break;
            }
            case FUNC_LOADSTAGE:
                opcodeSize = 0;
                StageMode  = STAGEMODE_LOAD;
                break;
            case FUNC_DRAWTINTRECT:
                opcodeSize = 0;
                DrawTintRect(ScriptEng.operands[0], ScriptEng.operands[1], ScriptEng.operands[2], ScriptEng.operands[3], ScriptEng.operands[4]);
                break;
            case FUNC_RESETOBJECTENTITY: {
                opcodeSize            = 0;
                Entity *newEnt        = &ObjectEntityList[ScriptEng.operands[0]];
                newEnt->type          = ScriptEng.operands[1];
                newEnt->propertyValue = ScriptEng.operands[2];
                newEnt->XPos          = ScriptEng.operands[3];
                newEnt->YPos          = ScriptEng.operands[4];
                newEnt->direction     = FLIP_NONE;
                newEnt->frame         = 0;
                newEnt->priority      = PRIORITY_BOUNDS;
                newEnt->rotation      = 0;
                newEnt->state         = 0;
                newEnt->drawOrder     = 3;
                newEnt->scale         = 512;
                newEnt->inkEffect     = INK_NONE;
                newEnt->values[0]     = 0;
                newEnt->values[1]     = 0;
                newEnt->values[2]     = 0;
                newEnt->values[3]     = 0;
                newEnt->values[4]     = 0;
                newEnt->values[5]     = 0;
                newEnt->values[6]     = 0;
                newEnt->values[7]     = 0;
                break;
            }
            case FUNC_PLAYEROBJECTCOLLISION:
                opcodeSize = 0;
                switch (ScriptEng.operands[0]) {
                    default: break;
                    case C_TOUCH:
                        ScriptEng.operands[5] = entity->XPos >> 16;
                        ScriptEng.operands[6] = entity->YPos >> 16;
                        BasicCollision(ScriptEng.operands[5] + ScriptEng.operands[1], ScriptEng.operands[6] + ScriptEng.operands[2],
                                       ScriptEng.operands[5] + ScriptEng.operands[3], ScriptEng.operands[6] + ScriptEng.operands[4]);
                        break;
                    case C_BOX:
                        BoxCollision(entity->XPos + (ScriptEng.operands[1] << 16), entity->YPos + (ScriptEng.operands[2] << 16),
                                     entity->XPos + (ScriptEng.operands[3] << 16), entity->YPos + (ScriptEng.operands[4] << 16));
                        break;
                    case C_PLATFORM:
                        PlatformCollision(entity->XPos + (ScriptEng.operands[1] << 16), entity->YPos + (ScriptEng.operands[2] << 16),
                                          entity->XPos + (ScriptEng.operands[3] << 16), entity->YPos + (ScriptEng.operands[4] << 16));
                        break;
                }
                break;
            case FUNC_CREATETEMPOBJECT: {
                opcodeSize = 0;
                if (ObjectEntityList[ScriptEng.arrayPosition[2]].type > 0 && ++ScriptEng.arrayPosition[2] == ENTITY_COUNT)
                    ScriptEng.arrayPosition[2] = TEMPENTITY_START;
                Entity *temp        = &ObjectEntityList[ScriptEng.arrayPosition[2]];
                temp->type          = ScriptEng.operands[0];
                temp->propertyValue = ScriptEng.operands[1];
                temp->XPos          = ScriptEng.operands[2];
                temp->YPos          = ScriptEng.operands[3];
                temp->direction     = FLIP_NONE;
                temp->frame         = 0;
                temp->priority      = PRIORITY_ALWAYS;
                temp->rotation      = 0;
                temp->state         = 0;
                temp->drawOrder     = 3;
                temp->scale         = 512;
                temp->inkEffect     = INK_NONE;
                temp->values[0]     = 0;
                temp->values[1]     = 0;
                temp->values[2]     = 0;
                temp->values[3]     = 0;
                temp->values[4]     = 0;
                temp->values[5]     = 0;
                temp->values[6]     = 0;
                temp->values[7]     = 0;
                break;
            }
            case FUNC_DEFAULTGROUNDMOVEMENT:
                opcodeSize = 0;
                ProcessDefaultGroundMovement(&PlayerList[PlayerNo]);
                break;
            case FUNC_DEFAULTAIRMOVEMENT:
                opcodeSize = 0;
                ProcessDefaultAirMovement(&PlayerList[PlayerNo]);
                break;
            case FUNC_DEFAULTROLLINGMOVEMENT:
                opcodeSize = 0;
                ProcessDefaultRollingMovement(&PlayerList[PlayerNo]);
                break;
            case FUNC_DEFAULTGRAVITYTRUE:
                opcodeSize = 0;
                ProcessDefaultGravityTrue(&PlayerList[PlayerNo]);
                break;
            case FUNC_DEFAULTGRAVITYFALSE:
                opcodeSize = 0;
                ProcessDefaultGravityFalse(&PlayerList[PlayerNo]);
                break;
            case FUNC_DEFAULTJUMPACTION:
                opcodeSize = 0;
                ProcessDefaultJumpAction(&PlayerList[PlayerNo]);
                break;
            case FUNC_SETMUSICTRACK:
                opcodeSize = 0;
                SetMusicTrack(ScriptText, ScriptEng.operands[1], ScriptEng.operands[2]);
                break;
            case FUNC_PLAYMUSIC:
                opcodeSize = 0;
                PlayMusic(ScriptEng.operands[0]);
                break;
            case FUNC_STOPMUSIC:
                opcodeSize = 0;
                StopMusic();
                break;
            case FUNC_PLAYSFX:
                opcodeSize = 0;
                PlaySfx(ScriptEng.operands[0], ScriptEng.operands[1]);
                break;
            case FUNC_STOPSFX:
                opcodeSize = 0;
                StopSfx(ScriptEng.operands[0]);
                break;
            case FUNC_SETSFXATTRIBUTES:
                opcodeSize = 0;
                SetSfxAttributes(ScriptEng.operands[0], ScriptEng.operands[1], ScriptEng.operands[2]);
                break;
            case FUNC_OBJECTTILECOLLISION:
                opcodeSize = 0;
                if (ScriptEng.operands[0] == CSIDE_FLOOR)
                    ObjectFloorCollision(ScriptEng.operands[1], ScriptEng.operands[2], ScriptEng.operands[3]);
                break;
            case FUNC_OBJECTTILEGRIP:
                opcodeSize = 0;
                if (ScriptEng.operands[0] == CSIDE_FLOOR)
                    ObjectFloorGrip(ScriptEng.operands[1], ScriptEng.operands[2], ScriptEng.operands[3]);
                break;
            case FUNC_LOADVIDEO:
                opcodeSize = 0;
                PauseSound();
                scriptInfo->spriteSheetID = AddGraphicsFile(ScriptText);
                ResumeSound();
                break;
            case FUNC_NEXTVIDEOFRAME:
                opcodeSize = 0;
                UpdateVideoFrame();
                break;
            case FUNC_PLAYSTAGESFX:
                opcodeSize = 0;
                PlaySfx(NoGlobalSFX + ScriptEng.operands[0], ScriptEng.operands[1]);
                break;
            case FUNC_STOPSTAGESFX:
                opcodeSize = 0;
                StopSfx(NoGlobalSFX + ScriptEng.operands[0]);
                break;
        }

        // Set Values
        if (opcodeSize > 0)
            scriptDataPtr -= scriptDataPtr - scriptCodeOffset;
        for (int i = 0; i < opcodeSize; ++i) {
            int opcodeType = ScriptData[scriptDataPtr++];
            if (opcodeType == SCRIPTVAR_VAR) {
                int arrayVal = 0;
                switch (ScriptData[scriptDataPtr++]) { // variable
                    case VARARR_NONE: arrayVal = ObjectLoop; break;
                    case VARARR_ARRAY:
                        if (ScriptData[scriptDataPtr++] == 1)
                            arrayVal = ScriptEng.arrayPosition[ScriptData[scriptDataPtr++]];
                        else
                            arrayVal = ScriptData[scriptDataPtr++];
                        break;
                    case VARARR_ENTNOPLUS1:
                        if (ScriptData[scriptDataPtr++] == 1)
                            arrayVal = ScriptEng.arrayPosition[ScriptData[scriptDataPtr++]] + ObjectLoop;
                        else
                            arrayVal = ScriptData[scriptDataPtr++] + ObjectLoop;
                        break;
                    case VARARR_ENTNOMINUS1:
                        if (ScriptData[scriptDataPtr++] == 1)
                            arrayVal = ObjectLoop - ScriptEng.arrayPosition[ScriptData[scriptDataPtr++]];
                        else
                            arrayVal = ObjectLoop - ScriptData[scriptDataPtr++];
                        break;
                    default: break;
                }

                // Variables
                switch (ScriptData[scriptDataPtr++]) {
                    default: break;
                    case VAR_OBJECTTYPE: {
                        ObjectEntityList[arrayVal].type = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTPROPERTYVALUE: {
                        ObjectEntityList[arrayVal].propertyValue = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTXPOS: {
                        ObjectEntityList[arrayVal].XPos = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTYPOS: {
                        ObjectEntityList[arrayVal].YPos = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTIXPOS: {
                        ObjectEntityList[arrayVal].XPos = ScriptEng.operands[i] << 16;
                        break;
                    }
                    case VAR_OBJECTIYPOS: {
                        ObjectEntityList[arrayVal].YPos = ScriptEng.operands[i] << 16;
                        break;
                    }
                    case VAR_OBJECTSTATE: {
                        ObjectEntityList[arrayVal].state = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTROTATION: {
                        ObjectEntityList[arrayVal].rotation = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTSCALE: {
                        ObjectEntityList[arrayVal].scale = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTPRIORITY: {
                        ObjectEntityList[arrayVal].priority = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTDRAWORDER: {
                        ObjectEntityList[arrayVal].drawOrder = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTDIRECTION: {
                        ObjectEntityList[arrayVal].direction = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTINKEFFECT: {
                        ObjectEntityList[arrayVal].inkEffect = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTFRAME: {
                        ObjectEntityList[arrayVal].frame = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE0: {
                        ObjectEntityList[arrayVal].values[0] = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE1: {
                        ObjectEntityList[arrayVal].values[1] = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE2: {
                        ObjectEntityList[arrayVal].values[2] = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE3: {
                        ObjectEntityList[arrayVal].values[3] = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE4: {
                        ObjectEntityList[arrayVal].values[4] = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE5: {
                        ObjectEntityList[arrayVal].values[5] = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE6: {
                        ObjectEntityList[arrayVal].values[6] = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_OBJECTVALUE7: {
                        ObjectEntityList[arrayVal].values[7] = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_TEMPVALUE0: ScriptEng.tempValue[0] = ScriptEng.operands[i]; break;
                    case VAR_TEMPVALUE1: ScriptEng.tempValue[1] = ScriptEng.operands[i]; break;
                    case VAR_TEMPVALUE2: ScriptEng.tempValue[2] = ScriptEng.operands[i]; break;
                    case VAR_TEMPVALUE3: ScriptEng.tempValue[3] = ScriptEng.operands[i]; break;
                    case VAR_TEMPVALUE4: ScriptEng.tempValue[4] = ScriptEng.operands[i]; break;
                    case VAR_TEMPVALUE5: ScriptEng.tempValue[5] = ScriptEng.operands[i]; break;
                    case VAR_TEMPVALUE6: ScriptEng.tempValue[6] = ScriptEng.operands[i]; break;
                    case VAR_TEMPVALUE7: ScriptEng.tempValue[7] = ScriptEng.operands[i]; break;
                    case VAR_CHECKRESULT: ScriptEng.checkResult = ScriptEng.operands[i]; break;
                    case VAR_ARRAYPOS0: ScriptEng.arrayPosition[0] = ScriptEng.operands[i]; break;
                    case VAR_ARRAYPOS1: ScriptEng.arrayPosition[1] = ScriptEng.operands[i]; break;
                    case VAR_KEYDOWNUP: GKeyDown.up = ScriptEng.operands[i]; break;
                    case VAR_KEYDOWNDOWN: GKeyDown.down = ScriptEng.operands[i]; break;
                    case VAR_KEYDOWNLEFT: GKeyDown.left = ScriptEng.operands[i]; break;
                    case VAR_KEYDOWNRIGHT: GKeyDown.right = ScriptEng.operands[i]; break;
                    case VAR_KEYDOWNBUTTONA: GKeyDown.A = ScriptEng.operands[i]; break;
                    case VAR_KEYDOWNBUTTONB: GKeyDown.B = ScriptEng.operands[i]; break;
                    case VAR_KEYDOWNBUTTONC: GKeyDown.C = ScriptEng.operands[i]; break;
                    case VAR_KEYDOWNSTART: GKeyDown.start = ScriptEng.operands[i]; break;
                    case VAR_KEYPRESSUP: GKeyPress.up = ScriptEng.operands[i]; break;
                    case VAR_KEYPRESSDOWN: GKeyPress.down = ScriptEng.operands[i]; break;
                    case VAR_KEYPRESSLEFT: GKeyPress.left = ScriptEng.operands[i]; break;
                    case VAR_KEYPRESSRIGHT: GKeyPress.right = ScriptEng.operands[i]; break;
                    case VAR_KEYPRESSBUTTONA: GKeyPress.A = ScriptEng.operands[i]; break;
                    case VAR_KEYPRESSBUTTONB: GKeyPress.B = ScriptEng.operands[i]; break;
                    case VAR_KEYPRESSBUTTONC: GKeyPress.C = ScriptEng.operands[i]; break;
                    case VAR_KEYPRESSSTART: GKeyPress.start = ScriptEng.operands[i]; break;
                    case VAR_MENU1SELECTION: GameMenu[0].selection1 = ScriptEng.operands[i]; break;
                    case VAR_MENU2SELECTION: GameMenu[1].selection1 = ScriptEng.operands[i]; break;
                    case VAR_STAGEACTIVELIST: ActiveStageList = ScriptEng.operands[i]; break;
                    case VAR_STAGELISTPOS: StageListPosition = ScriptEng.operands[i]; break;
                    case VAR_XSCROLLOFFSET: XScrollOffset = ScriptEng.operands[i]; break;
                    case VAR_YSCROLLOFFSET: YScrollOffset = ScriptEng.operands[i]; break;
                    case VAR_GLOBAL: GlobalVariables[arrayVal] = ScriptEng.operands[i]; break;
                    case VAR_STAGETIMEENABLED: TimeEnabled = ScriptEng.operands[i]; break;
                    case VAR_STAGEMILLISECONDS: MilliSeconds = ScriptEng.operands[i]; break;
                    case VAR_STAGESECONDS: Seconds = ScriptEng.operands[i]; break;
                    case VAR_STAGEMINUTES: Minutes = ScriptEng.operands[i]; break;
                    case VAR_STAGEACTNO: ActNumber = ScriptEng.operands[i]; break;
                    case VAR_OBJECTENTITYNO: break;
                    case VAR_PLAYERTYPE: {
                        PlayerList[PlayerNo].type = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERSTATE: {
                        PlayerList[PlayerNo].state = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERCONTROLMODE: {
                        PlayerList[PlayerNo].controlMode = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERCOLLISIONMODE: {
                        PlayerList[PlayerNo].collisionMode = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERCOLLISIONPLANE: {
                        PlayerList[PlayerNo].collisionPlane = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERXPOS: {
                        PlayerList[PlayerNo].XPos = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERYPOS: {
                        PlayerList[PlayerNo].YPos = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERSCREENXPOS: {
                        PlayerList[PlayerNo].screenXPos = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERSCREENYPOS: {
                        PlayerList[PlayerNo].screenYPos = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERSPEED: {
                        PlayerList[PlayerNo].speed = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERXVELOCITY: {
                        PlayerList[PlayerNo].XVelocity = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERYVELOCITY: {
                        PlayerList[PlayerNo].YVelocity = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERGRAVITY: {
                        PlayerList[PlayerNo].gravity = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERANGLE: {
                        PlayerList[PlayerNo].angle = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERROTATION: {
                        PlayerList[PlayerNo].rotation = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERDIRECTION: {
                        PlayerList[PlayerNo].direction = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERANIMATION: {
                        PlayerList[PlayerNo].animation = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERFRAME: {
                        PlayerList[PlayerNo].frame = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERSKIDDING: {
                        PlayerList[PlayerNo].skidding = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERPUSHING: {
                        PlayerList[PlayerNo].pushing = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERFRICTIONLOSS: {
                        PlayerList[PlayerNo].frictionLoss = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERWALKINGSPEED: {
                        PlayerList[PlayerNo].walkingSpeed = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERRUNNINGSPEED: {
                        PlayerList[PlayerNo].runningSpeed = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERJUMPINGSPEED: {
                        PlayerList[PlayerNo].jumpingSpeed = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERTRACKSCROLL: {
                        PlayerList[PlayerNo].trackScroll = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERUP: {
                        PlayerList[PlayerNo].up = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERDOWN: {
                        PlayerList[PlayerNo].down = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERLEFT: {
                        PlayerList[PlayerNo].left = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERRIGHT: {
                        PlayerList[PlayerNo].right = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERJUMPPRESS: {
                        PlayerList[PlayerNo].jumpPress = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERJUMPHOLD: {
                        PlayerList[PlayerNo].jumpHold = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERFOLLOWPLAYER1: {
                        PlayerList[PlayerNo].followPlayer1 = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERLOOKPOS: {
                        PlayerList[PlayerNo].lookPos = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERWATER: {
                        PlayerList[PlayerNo].water = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERTOPSPEED: {
                        PlayerList[PlayerNo].stats.topSpeed = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERACCELERATION: {
                        PlayerList[PlayerNo].stats.acceleration = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERDECELERATION: {
                        PlayerList[PlayerNo].stats.deceleration = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERAIRACCELERATION: {
                        PlayerList[PlayerNo].stats.airAcceleration = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERAIRDECELERATION: {
                        PlayerList[PlayerNo].stats.airDeceleration = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERGRAVITYSTRENGTH: {
                        PlayerList[PlayerNo].stats.gravityStrength = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERJUMPSTRENGTH: {
                        PlayerList[PlayerNo].stats.jumpStrength = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERROLLINGACCELERATION: {
                        PlayerList[PlayerNo].stats.rollingAcceleration = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERROLLINGDECELERATION: {
                        PlayerList[PlayerNo].stats.rollingDeceleration = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERENTITYNO: break;
                    case VAR_PLAYERCOLLISIONLEFT: break;
                    case VAR_PLAYERCOLLISIONTOP: break;
                    case VAR_PLAYERCOLLISIONRIGHT: break;
                    case VAR_PLAYERCOLLISIONBOTTOM: break;
                    case VAR_PLAYERFLAILING: {
                        PlayerList[PlayerNo].flailing[arrayVal] = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_STAGEPAUSEENABLED: {
                        PauseEnabled = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_STAGELISTSIZE: {
                        break;
                    }
                    case VAR_PLAYERTIMER: {
                        PlayerList[PlayerNo].timer = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERANIMATIONSPEED: {
                        PlayerList[PlayerNo].animationSpeed = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYERTILECOLLISIONS: {
                        PlayerList[PlayerNo].tileCollisions = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_PLAYEROBJECTINTERACTION: {
                        PlayerList[PlayerNo].objectInteraction = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_SCREENCAMERAENABLED: CameraEnabled = ScriptEng.operands[i]; break;
                    case VAR_SCREENCAMERASTYLE: CameraStyle = ScriptEng.operands[i]; break;
                    case VAR_MUSICVOLUME: SetMusicVolume(ScriptEng.operands[i]); break;
                    case VAR_MUSICCURRENTTRACK: CurrentMusicTrack = ScriptEng.operands[i]; break;
                    case VAR_PLAYERVISIBLE: {
                        PlayerList[PlayerNo].visible = ScriptEng.operands[i];
                        break;
                    }
                    case VAR_STAGENEWXBOUNDARY1: NewXBoundary1 = ScriptEng.operands[i]; break;
                    case VAR_STAGENEWXBOUNDARY2: NewXBoundary2 = ScriptEng.operands[i]; break;
                    case VAR_STAGENEWYBOUNDARY1: NewYBoundary1 = ScriptEng.operands[i]; break;
                    case VAR_STAGENEWYBOUNDARY2: NewYBoundary2 = ScriptEng.operands[i]; break;
                    case VAR_STAGEXBOUNDARY1:
                        if (XBoundary1 != ScriptEng.operands[i]) {
                            XBoundary1    = ScriptEng.operands[i];
                            NewXBoundary1 = ScriptEng.operands[i];
                        }
                        break;
                    case VAR_STAGEXBOUNDARY2:
                        if (XBoundary2 != ScriptEng.operands[i]) {
                            XBoundary2    = ScriptEng.operands[i];
                            NewXBoundary2 = ScriptEng.operands[i];
                        }
                        break;
                    case VAR_STAGEYBOUNDARY1:
                        if (YBoundary1 != ScriptEng.operands[i]) {
                            YBoundary1    = ScriptEng.operands[i];
                            NewYBoundary1 = ScriptEng.operands[i];
                        }
                        break;
                    case VAR_STAGEYBOUNDARY2:
                        if (YBoundary2 != ScriptEng.operands[i]) {
                            YBoundary2    = ScriptEng.operands[i];
                            NewYBoundary2 = ScriptEng.operands[i];
                        }
                        break;
                    case VAR_OBJECTOUTOFBOUNDS: break;
                }
            } else if (opcodeType == SCRIPTVAR_INTCONST) { // int constant
                scriptDataPtr++;
            } else if (opcodeType == SCRIPTVAR_STRCONST) { // string constant
                int strLen = ScriptData[scriptDataPtr++];
                for (int c = 0; c < strLen; ++c) {
                    switch (c % 4) {
                        case 0: break;
                        case 1: break;
                        case 2: break;
                        case 3: ++scriptDataPtr; break;
                        default: break;
                    }
                }
                scriptDataPtr++;
            }
        }
    }
}
