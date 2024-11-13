#ifndef SCRIPT_H
#define SCRIPT_H

#define SCRIPTDATA_COUNT (0x40000)
#define JUMPTABLE_COUNT  (0x4000)
#define JUMPSTACK_COUNT (0x400)

struct ScriptPtr {
    int scriptCodePtr;
    int jumpTablePtr;
};

struct ObjectScript {
    byte frameCount;
    byte spriteSheetID;
    ScriptPtr subMain;
    ScriptPtr subPlayerInteraction;
    ScriptPtr subDraw;
    ScriptPtr subStartup;
    SpriteFrame *frameStartPtr;
};

struct ScriptEngine {
    int operands[10];
    int tempValue[8];
    int arrayPosition[3];
    int checkResult;
};

enum ScriptSubs { SUB_MAIN = 0, SUB_PLAYERINTERACTION = 1, SUB_DRAW = 2, SUB_SETUP = 3, SUB_PLAYERMAIN = 4, SUB_PLAYERSTATE = 5 };

extern ObjectScript ObjectScriptList[OBJECT_COUNT];

extern int ScriptData[SCRIPTDATA_COUNT];
extern int JumpTableData[JUMPTABLE_COUNT];

extern int JumpTableStack[JUMPSTACK_COUNT];

extern int JumpTableStackPos;

extern ScriptEngine ScriptEng;
extern char ScriptText[0x100];

extern int ScriptDataPos;
extern int scriptDataOffset;
extern int JumpTableDataPos;
extern int JumpTableOffset;

extern int NO_ALIASES;
extern int lineID;

void CheckAliasText(char *text);
void ConvertArithmaticSyntax(char *text);
void ConvertIfWhileStatement(char *text);
bool ConvertSwitchStatement(char *text);
void ConvertFunctionText(char *text);
void CheckCaseNumber(char *text);
bool ReadSwitchCase(char *text);
void AppendIntegerToString(char *text, int value);
bool ConvertStringToInteger(char *text, int *value);
void CopyAliasStr(char *dest, char *text, bool arrayIndex);
bool CheckOpcodeType(char *text); // Never actually used

void ParseScriptFile(char *scriptName, int scriptID);

void ProcessScript(int scriptCodePtr, int jumpTablePtr, byte scriptSub);

void ClearScriptData();

#endif // !SCRIPT_H
