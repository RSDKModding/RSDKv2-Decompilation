#ifndef MATH_H
#define MATH_H

#define MEM_ZERO(x)  memset(&(x), 0, sizeof((x)))
#define MEM_ZEROP(x) memset((x), 0, sizeof(*(x)))

extern int SinValue512[0x200];
extern int CosValue512[0x200];

extern int SinValue256[0x100];
extern int CosValue256[0x100];

// Setup Angles
void CalculateTrigAngles();

inline int Sin512(int angle) {
    if (angle < 0)
        angle = 0x200 - angle;
    angle &= 0x1FF;
    return SinValue512[angle];
}

inline int Cos512(int angle)
{
    if (angle < 0)
        angle = 0x200 - angle;
    angle &= 0x1FF;
    return CosValue512[angle];
}

inline int Sin256(int angle)
{
    if (angle < 0)
        angle = 0x100 - angle;
    angle &= 0xFF;
    return SinValue256[angle];
}

inline int Cos256(int angle)
{
    if (angle < 0)
        angle = 0x100 - angle;
    angle &= 0xFF;
    return CosValue256[angle];
}

#endif // !MATH_H
