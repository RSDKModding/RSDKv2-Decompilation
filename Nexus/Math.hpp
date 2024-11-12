#ifndef MATH_H
#define MATH_H

//#define M_PI (3.1415927)

#define MEM_ZERO(x)  memset(&(x), 0, sizeof((x)))
#define MEM_ZEROP(x) memset((x), 0, sizeof(*(x)))

extern int sinVal512[0x200];
extern int cosVal512[0x200];

extern int sinVal256[0x100];
extern int cosVal256[0x100];

// Setup Angles
void CalculateTrigAngles();

inline int Sin512(int angle) {
    if (angle < 0)
        angle = 0x200 - angle;
    angle &= 0x1FF;
    return sinVal512[angle];
}

inline int Cos512(int angle)
{
    if (angle < 0)
        angle = 0x200 - angle;
    angle &= 0x1FF;
    return cosVal512[angle];
}

inline int Sin256(int angle)
{
    if (angle < 0)
        angle = 0x100 - angle;
    angle &= 0xFF;
    return sinVal256[angle];
}

inline int Cos256(int angle)
{
    if (angle < 0)
        angle = 0x100 - angle;
    angle &= 0xFF;
    return cosVal256[angle];
}

#endif // !MATH_H
