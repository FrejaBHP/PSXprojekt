#ifndef __OBJECTS_H
#define __OBJECTS_H

#include <libgte.h>
#include <libgpu.h>

enum DrawPriority {
    DRP_Neutral,
    DRP_Low,
    DRP_High
};

// Holds Position, Rotation and Transform for an "object". Probably could use a better name.
typedef struct GameObject {
    VECTOR position; // Position to update the Transform with. Position is ONE (4096) bigger than the actual values stored in the Transform
    SVECTOR rotation;
    MATRIX transform;
    VECTOR velocity; // Velocity, expressed in fixed-point integers (* ONE)
    long maxSpeed;
    bool isStatic;
} GameObject;

// Same as GameObject, except uses a VECTOR for rotation instead of SVECTOR
typedef struct CameraObject {
    VECTOR position;
    VECTOR rotation;
    MATRIX transform;
} CameraObject;

// Extends GameObject and can also hold all the data needed to draw a polygon
typedef struct PolyObject {
    GameObject obj;
    MATRIX renderTransform;

    u_char polySides;
    ushort polyLength;
    void* polyPtr;
    SVECTOR* verticesPtr;
    int* indicesPtr;
    enum DrawPriority drPrio;

    int boxHeight;
    int boxWidth;

    bool collides;

    //void (*add)(struct PolyObject* self, u_long* ot);
} PolyObject;

typedef struct TexturedPolyObject {
    PolyObject polyObj;
    TIM_IMAGE* tim;
    RECT trect;
    bool repeating;
} TexturedPolyObject;

typedef struct PlayerObject {
    PolyObject poly;
    CameraObject* cameraPtr;
    VECTOR velocity;
} PlayerObject;

#endif
