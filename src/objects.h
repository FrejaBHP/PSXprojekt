#ifndef __OBJECTS_H
#define __OBJECTS_H

#include <libgte.h>
#include <libgpu.h>
#include <stdbool.h>

enum DrawPriority {
    DRP_Neutral,
    DRP_First,
    DRP_Highest,
    DRP_Higher,
    DRP_High,
    DRP_Low,
    DRP_Lower,
    DRP_Lowest,
    DRP_Last
};

enum CollectibleType {
    CT_Coin
};

typedef struct PolyData {
	u_short tpage, clut;
	u_char u0, v0, um, vm;
	u_char r, g, b, pad;
} PolyData;

typedef struct CollisionOverlaps {
    bool x;
    bool y;
    bool z;
    u_char pad;
} CollisionOverlaps;

typedef struct CollisionBox {
    VECTOR gridPos;
    SVECTOR dimensions;
} CollisionBox;

// Same as GameObject, except uses a VECTOR for rotation instead of SVECTOR
typedef struct CameraObject {
    VECTOR position;
    VECTOR rotation;
    MATRIX transform;
} CameraObject;

typedef struct CollectibleObject {
    VECTOR position; // Position to update the Transform with. Position is ONE (4096) bigger than the actual values stored in the Transform
    SVECTOR rotation;
    MATRIX transform;
    enum CollectibleType cType;
} CollectibleObject;

typedef struct StaticWorldGeometry {
    PolyData* polyDataPtr;
    SVECTOR* verticesPtr;
    long* indicesPtr;
    enum DrawPriority drPrio;
    TIM_IMAGE* tim;
    size_t totalPolys;
} StaticWorldGeometry;

typedef struct StaticWorldPolyBox {
    PolyData* polyDataPtr;
    SVECTOR vertices[8];
    enum DrawPriority drPrio;
    CollisionBox colBox;
    size_t polyMask;
} StaticWorldPolyBox;

// ============= LEGACY, CLEAN UP =============

// Holds Position, Rotation and Transform for an "object". Probably could use a better name.
typedef struct GameObject {
    VECTOR position; // Position to update the Transform with. Position is ONE (4096) bigger than the actual values stored in the Transform
    SVECTOR rotation;
    MATRIX transform;
    VECTOR velocity; // Velocity, expressed in fixed-point integers (* ONE)
    long maxSpeed;
} GameObject;

// Extends GameObject and can also hold all the data needed to draw a polygon
typedef struct PolyObject {
    GameObject obj;

    u_char polySides;
    bool collides;
    ushort polyLength;
    PolyData* polyDataPtr;
    SVECTOR* verticesPtr;
    long* indicesPtr;
    enum DrawPriority drPrio;

    // Used for player collision. Should be moved elsewhere, since no objects actually use this
    int boxHeight;
    int boxWidth;

    //void (*add)(struct PolyObject* self, u_long* ot);
} PolyObject;

typedef struct TexturedPolyObject {
    PolyObject polyObj;
    TIM_IMAGE* tim;
    RECT trect;
} TexturedPolyObject;

typedef struct TestTileMultiPoly {
    GameObject obj; // Compatibility

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

#endif
