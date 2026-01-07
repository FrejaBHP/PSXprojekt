#ifndef __OBJECTS_H
#define __OBJECTS_H

#include <libgte.h>
#include <libgpu.h>

enum DrawPriority {
    DRP_Neutral,
    DRP_Low,
    DRP_High
};

typedef struct CollisionBox {
    SVECTOR dimensions;
} CollisionBox;

typedef struct RemoteCollisionBox {
    SVECTOR gridPosition;
    CollisionBox cbox;
} RemoteCollisionBox;

typedef struct StaticCollisionPolyBox {
    VECTOR position;
    SVECTOR rotation;
    MATRIX transform;
    CollisionBox colBox;

    POLY_FT4* polys[6];
    SVECTOR* vertices;
    long* indices;
} StaticCollisionPolyBox;



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
    long* indicesPtr;
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

typedef struct TestTileMultiPoly {
    GameObject obj; // Compatibility
    MATRIX renderTransform; // Holdover

    u_char repeats;
    u_char rx;
    u_char ry;
    u_char rz;
    
    u_char width;
    u_char height;
    u_char depth;

    SVECTOR* verticesPtr;
    long* indicesPtr;
    POLY_FT4* polyPtr;

    u_char subdivs;
    ushort totalPolys;

    TIM_IMAGE* tim;
    //u_char u0;
    //u_char v0;
    //u_char uw;
    //u_char vh;

    bool reverseOrder;

} TestTileMultiPoly;

typedef struct PlayerObject {
    PolyObject poly;
    CameraObject* cameraPtr;
    VECTOR velocity;
} PlayerObject;

#endif
