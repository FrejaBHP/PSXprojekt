#ifndef __INPUT_H
#define __INPUT_H

#include <types.h>
#include <stdbool.h>

#define DATABUFFER_SIZE 34

typedef struct Vector2UB {
    u_char x; // Left = neg, Right = pos
    u_char y; // Up = neg, Down = pos
} Vector2UB;

// Holds the pad data stream from the system (dataBuffer), and divides it into the other, more readable members
typedef struct GamePad {
    u_char port;
    u_char status;
    u_char type;
    u_char state;

    bool isAnaloguePad;
    bool isAnalogueActive;
    bool allowVibration;
    u_char vibrationTime;
    
    char delay;
    u_char padding[3];

    char motors[2];
    ushort buttons;

    Vector2UB leftstick;
    Vector2UB rightstick;
} GamePad;

extern char padDataBuffer[2][DATABUFFER_SIZE];
extern GamePad pads[2];

extern bool isSelectHeld;
extern bool isStartHeld;
extern bool isCrossHeld;
extern bool isSquareHeld;
extern bool isTriangleHeld;
extern bool isCircleHeld;

void InitControllers();
void StopControllers();
void UpdatePad(int pad);
void ClearPad(int pad);

#endif
