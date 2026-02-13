#include <stdlib.h>
#include <stdbool.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
#include <libapi.h>
#include <libspu.h>
#include <inline_n.h>
#include <gtemac.h>

#include "graphics.h"
#include "objects.h"
#include "physics.h"
#include "player.h"

#include "clist.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))
#define setPosVToGrid(v, _x, _y, _z) \
	(v)->vx = _x >> 12, (v)->vy = _y >> 12, (v)->vz = _z >> 12

#define DISTTHING 512

#define PLAYERHEIGHT 36
#define PLAYERWIDTHHALF 16
#define CAMERADISTANCE 144 // 160

#define CUBESIZE 80
#define CUBEHALF CUBESIZE / 2
#define FLOORSIZE 128
#define FLOORHALF FLOORSIZE / 2
#define LONGFLOORLENGTH 320
#define LONGFLOORHALF LONGFLOORLENGTH / 2
#define COLBOXHEIGHT 12
#define COLBOXHALFWIDTH 32

#define WALLHEIGHT 128
#define WALLHALF 64
#define DOORHALF 32

#define ANALOGUE_MID 127
#define ANALOGUE_DEADZONE 24
#define ANALOGUE_MINPOS ANALOGUE_MID + ANALOGUE_DEADZONE
#define ANALOGUE_MINNEG ANALOGUE_MID - ANALOGUE_DEADZONE

#define ACTIVEPOLYGONCOUNT 3
#define ACTIVETEXPOLYGONCOUNT 5
#define ACTIVETILEDTEXPOLYGONCOUNT 4

extern u_long __heap_start;
u_long* heapStart = &__heap_start;

typedef struct Vector2UB {
    u_char x; // Left = neg, Right = pos
    u_char y; // Up = neg, Down = pos
} Vector2UB;

// Holds the pad data stream from the engine (dataBuffer), and divides it into the other, more readable members
typedef struct GamePad {
    u_char dataBuffer[34];
    u_char status;
    u_char type;
    ushort buttons;
    Vector2UB leftstick;
    Vector2UB rightstick;
} GamePad;


static SVECTOR colBoxVertices[] = {
    { -COLBOXHALFWIDTH, -COLBOXHEIGHT, -COLBOXHALFWIDTH, 0 }, {  COLBOXHALFWIDTH, -COLBOXHEIGHT, -COLBOXHALFWIDTH, 0 },
    {  COLBOXHALFWIDTH,  0,            -COLBOXHALFWIDTH, 0 }, { -COLBOXHALFWIDTH,  0,            -COLBOXHALFWIDTH, 0 },
    { -COLBOXHALFWIDTH, -COLBOXHEIGHT,  COLBOXHALFWIDTH, 0 }, {  COLBOXHALFWIDTH, -COLBOXHEIGHT,  COLBOXHALFWIDTH, 0 },
    {  COLBOXHALFWIDTH,  0,             COLBOXHALFWIDTH, 0 }, { -COLBOXHALFWIDTH,  0,             COLBOXHALFWIDTH, 0 },
};

static SVECTOR playerBoxVertices[] = {
    { -PLAYERWIDTHHALF, -PLAYERHEIGHT, -PLAYERWIDTHHALF, 0 }, {  PLAYERWIDTHHALF, -PLAYERHEIGHT, -PLAYERWIDTHHALF, 0 },
    {  PLAYERWIDTHHALF, 0, -PLAYERWIDTHHALF, 0 }, { -PLAYERWIDTHHALF, 0, -PLAYERWIDTHHALF, 0 },
    { -PLAYERWIDTHHALF, -PLAYERHEIGHT,  PLAYERWIDTHHALF, 0 }, {  PLAYERWIDTHHALF, -PLAYERHEIGHT,  PLAYERWIDTHHALF, 0 },
    {  PLAYERWIDTHHALF, 0,  PLAYERWIDTHHALF, 0 }, { -PLAYERWIDTHHALF, 0,  PLAYERWIDTHHALF, 0 },
};

static SVECTOR cubeVertices[] = {
    { -CUBEHALF, -CUBEHALF, -CUBEHALF, 0 }, { CUBEHALF , -CUBEHALF, -CUBEHALF, 0 },
    { CUBEHALF , CUBEHALF , -CUBEHALF, 0 }, { -CUBEHALF, CUBEHALF , -CUBEHALF, 0 },
    { -CUBEHALF, -CUBEHALF, CUBEHALF , 0 }, { CUBEHALF , -CUBEHALF, CUBEHALF , 0 },
    { CUBEHALF , CUBEHALF , CUBEHALF , 0 }, { -CUBEHALF, CUBEHALF , CUBEHALF , 0 }
};

static SVECTOR tinyHouseVertices[] = {
    { 0, -WALLHEIGHT, 0, 0 },               { WALLHALF, -WALLHEIGHT, 0, 0 },
    { WALLHALF,  0,  0, 0 },                { 0,  0,          0, 0 },
    { 0, -WALLHEIGHT,  WALLHALF * 2, 0 },   { WALLHALF, -WALLHEIGHT,  WALLHALF * 2, 0 },
    { WALLHALF,  0,  WALLHALF * 2, 0 },     { 0,  0,           WALLHALF * 2, 0 },
};

static SVECTOR tinyBoxVertices[] = {
    { 0, -DOORHALF, 0, 0 },               { DOORHALF, -DOORHALF, 0, 0 },
    { DOORHALF,  0,  0, 0 },                { 0,  0,          0, 0 },
    { 0, -DOORHALF,  DOORHALF, 0 },   { DOORHALF, -DOORHALF,  DOORHALF, 0 },
    { DOORHALF,  0,  DOORHALF, 0 },     { 0,  0,           DOORHALF, 0 },
};

static SVECTOR boxVertices[] = {
    { 0, -WALLHALF, 0, 0 },               { WALLHALF, -WALLHALF, 0, 0 },
    { WALLHALF,  0,  0, 0 },                { 0,  0,          0, 0 },
    { 0, -WALLHALF,  WALLHALF, 0 },   { WALLHALF, -WALLHALF,  WALLHALF, 0 },
    { WALLHALF,  0,  WALLHALF, 0 },     { 0,  0,           WALLHALF, 0 },
};

static SVECTOR platformVertices[] = {
    { 0, -16, 0, 0 }, { WALLHALF, -16, 0, 0 },
    { WALLHALF, 0,  0, 0 }, { 0, 0, 0, 0 },
    { 0, -16,  WALLHALF, 0 }, { WALLHALF, -16,  WALLHALF, 0 },
    { WALLHALF, 0,  WALLHALF, 0 }, { 0, 0, WALLHALF, 0 },
};

static SVECTOR tWallVertices[] = {
    { 0, -WALLHEIGHT, 0, 0 },               { WALLHALF * 2, -WALLHEIGHT, 0, 0 },
    { WALLHALF * 2,  0,  0, 0 },    { 0,  0,          0, 0 },
    { 0, -WALLHEIGHT,  WALLHALF, 0 },       { WALLHALF * 2, -WALLHEIGHT,  WALLHALF, 0 },
    { WALLHALF * 2,  0,  WALLHALF, 0 },     { 0,  0,           WALLHALF, 0 }
};

static SVECTOR tDoorVertices[] = {
    { 0, -WALLHEIGHT, 0, 0 }, {  WALLHALF, -WALLHEIGHT, 0, 0 },
    { WALLHALF,  0,          0, 0 }, { 0,  0,          0, 0 },
    { 0, -WALLHEIGHT,  WALLHALF, 0 }, {  WALLHALF, -WALLHEIGHT,  WALLHALF, 0 },
    { WALLHALF,  0,           WALLHALF, 0 }, { 0,  0,           WALLHALF, 0 },
};

static long cubeIndices[] = {
    0, 1, 2, 3, // Back?
    1, 5, 6, 2, // Right?
    5, 4, 7, 6, // Front?
    4, 0, 3, 7, // Left?
    4, 5, 1, 0, // Top
    6, 7, 3, 2  // Bottom
};

static long tubeIndices[] = {
    0, 1, 2, 3, 
    1, 5, 6, 2, 
    5, 4, 7, 6, 
    4, 0, 3, 7
};

static SVECTOR floorVertices[] = {
    { -FLOORHALF, 0, -FLOORHALF, 0 }, {  FLOORHALF, 0, -FLOORHALF, 0 },
    {  FLOORHALF, 0,  FLOORHALF, 0 }, { -FLOORHALF, 0,  FLOORHALF, 0 }
};

static SVECTOR longFloorVertices[] = {
    { 0, 0, 0, 0 },                       { LONGFLOORLENGTH, 0, 0, 0 },
    { LONGFLOORLENGTH, 0, FLOORSIZE, 0 }, { 0, 0, FLOORSIZE, 0 }
};

static SVECTOR tiledHalfPanelVertices[] = {
    { 0, -WALLHEIGHT, 0, 0 }, { WALLHALF, -WALLHEIGHT, 0, 0 },
    { WALLHALF, 0, 0, 0 }, { 0, 0, 0, 0 }
};

static SVECTOR tiledPanelVertices[] = {
    { 0, -WALLHEIGHT, 0, 0 }, { WALLHEIGHT, -WALLHEIGHT, 0, 0 },
    { WALLHEIGHT, 0, 0, 0 }, { 0, 0, 0, 0 }
};

static SVECTOR collectibleVertices[] = {
    { -16, -16, 0, 0 }, { 16, -16, 0, 0 },
    { 16, 16, 0, 0 },   { -16, 16, 0, 0 }
};

static long reverseWindingIndices[] = {
    0, 3, 2, 1
};

static long windingIndices[] = {
    0, 1, 2, 3
};

PolyData coinPolyData;

PolyObject* activePolygons[ACTIVEPOLYGONCOUNT];
TexturedPolyObject* activeTexPolygons[ACTIVETEXPOLYGONCOUNT];
TiledTexturedPolyObject* activeTiledTexPolygons[ACTIVETILEDTEXPOLYGONCOUNT];

LinkedList* collectibleList;

MATRIX cameraRotationMatrix = { 0 };
SVECTOR cameraRotation = { 0 };

// Splits the dataBuffer into the other members for readability and ease of use
void UpdatePad(GamePad* pad) {
    pad->status = pad->dataBuffer[0];
    pad->type = pad->dataBuffer[1];
    pad->buttons = 0xFFFF - ((pad->dataBuffer[2] << 8) | (pad->dataBuffer[3])); // Stores buffer[2] in the upper 8 bits and [3] in the lower 8 bits. Eases parsing later
    pad->leftstick.x = pad->dataBuffer[6];
    pad->leftstick.y = pad->dataBuffer[7];
    pad->rightstick.x = pad->dataBuffer[4];
    pad->rightstick.y = pad->dataBuffer[5];
}

// Lazy and likely fragile attempt at influencing sorting order
static void OrderThing(long* otz, int dp) {
    switch (dp) {
        case DRP_First:
            *otz = 0;
            break;

        case DRP_Highest:
            if (*otz - 16 > 0) {
                *otz -= 16;
            }
            else {
                *otz = 1;
            }
            break;

        case DRP_Higher:
            if (*otz - 8 > 0) {
                *otz -= 8;
            }
            else if (*otz > 2) {
                *otz = 2;
            }
            break;

        case DRP_High:
            if (*otz - 4 > 0) {
                *otz -= 4;
            }
            else if (*otz > 3) {
                *otz = 3;
            }
            break;

        case DRP_Low:
            if (*otz + 8 < (OTSIZE - 1)) {
                *otz += 8;
            }
            else if (*otz < (OTSIZE - 4)) {
                *otz = OTSIZE - 4;
            }
            break;

        case DRP_Lower:
            if (*otz + 16 < (OTSIZE - 1)) {
                *otz += 16;
            }
            else if (*otz < (OTSIZE - 3)) {
                *otz = OTSIZE - 3;
            }
            break;

        case DRP_Lowest:
            if (*otz + 32 < (OTSIZE - 1)) {
                *otz += 32;
            }
            else if (*otz < (OTSIZE - 2)) {
                *otz = OTSIZE - 2;
            }
            break;

        case DRP_Last:
            *otz = OTSIZE - 1;
            break;
        
        default:
            break;
    }
}

long GetVectorPlaneLength(VECTOR* vec) {
    long cA;
    long cB;
    long cC;

    cA = abs(vec->vx);
    cB = abs(vec->vy);

    cA *= cA;
    cB *= cB;

    cC = cA + cB;
    cC = SquareRoot0(cC);

    return cC;
}

long GetVectorPlaneLength64(VECTOR* vec) {
    long cA;
    long cB;
    long cC;

    cA = abs(vec->vx >> 6);
    cB = abs(vec->vy >> 6);

    cA *= cA;
    cB *= cB;

    cC = cA + cB;
    cC = SquareRoot0(cC);

    return cC;
}

void CreatePlayer(CVECTOR* col) {
    player = calloc(1, sizeof(PlayerObject));

    CameraObject* camera = calloc(1, sizeof(CameraObject));
    PolyData* polyData = calloc(6, sizeof(PolyData));
    VECTOR pos = { 0, 0, 0 };

    if (player != NULL) {
        setVector(&player->poly.obj.position, pos.vx * ONE, pos.vy * ONE, pos.vz * ONE);
        player->poly.polyLength = 6;
        player->poly.polySides = 4;
        player->poly.verticesPtr = playerBoxVertices;
        player->poly.indicesPtr = cubeIndices;
        player->poly.polyDataPtr = polyData;
        player->poly.drPrio = DRP_High;
        player->poly.collides = false;
        player->poly.boxHeight = PLAYERHEIGHT;
        player->poly.boxWidth = PLAYERWIDTHHALF * 2;
        player->poly.obj.maxSpeed = 5 * ONE;
        player->poly.obj.isStatic = false;
        //player->poly.add = &AddPolyF;

        if (camera != NULL) {
            player->cameraPtr = camera;
        }

        for (size_t i = 0; i < 6; ++i) {
            polyData[i].r = col[i].r;
            polyData[i].g = col[i].g;
            polyData[i].b = col[i].b;
        }

        RotMatrix_gte(&player->poly.obj.rotation, &player->poly.obj.transform);
        TransMatrix(&player->poly.obj.transform, &pos);
    }
}

PolyObject* CreatePolyObjectF4(long posX, long posY, long posZ, short rotX, short rotY, short rotZ, ushort plen, ushort psides, SVECTOR* vertPtr, long* indPtr, enum DrawPriority drprio, bool coll, int collH, int collW, bool fixed, CVECTOR* col) {
    PolyObject* pobj = calloc(1, sizeof(PolyObject));
    PolyData* polyData = calloc(plen, sizeof(PolyData));
    VECTOR pos = { posX, posY, posZ };

    if (pobj != NULL) {
        setVector(&pobj->obj.position, pos.vx * ONE, pos.vy * ONE, pos.vz * ONE);
        setVector(&pobj->obj.rotation, rotX, rotY, rotZ);
        pobj->polyLength = plen;
        pobj->polySides = psides;
        pobj->verticesPtr = vertPtr;
        pobj->indicesPtr = indPtr;
        pobj->polyDataPtr = polyData;
        pobj->drPrio = drprio;
        pobj->collides = coll;
        pobj->boxHeight = collH;
        pobj->boxWidth = collW;
        pobj->obj.isStatic = fixed;
        //platform->add = &AddPolyF;

        for (size_t i = 0; i < plen; i++) {
            polyData[i].r = col[i].r;
            polyData[i].g = col[i].g;
            polyData[i].b = col[i].b;
            /* code */
        }
        
        RotMatrix_gte(&pobj->obj.rotation, &pobj->obj.transform);
        TransMatrix(&pobj->obj.transform, &pos);
    }

    return pobj;
}

POLY_FT4* CreateTexturedPolygon4(TIM_IMAGE* tim, u_char u0, u_char v0, u_char u1, u_char v1) {
    POLY_FT4* poly = calloc(1, sizeof(POLY_FT4));
    setPolyFT4(poly);

    poly->tpage = getTPage(tim->mode & 0x3, 0, tim->prect->x, tim->prect->y);
    poly->clut = getClut(tim->crect->x, tim->crect->y);
    setRGB0(poly, 128, 128, 128);
    setUVWH(poly, u0, v0, u1, v1);

    return poly;
}

PolyData SetupPolyData(TIM_IMAGE* tim, u_char u0, u_char v0, u_char u1, u_char v1) {
    PolyData data = {
        .tpage = getTPage(tim->mode & 0x3, 0, tim->prect->x, tim->prect->y),
        .clut = getClut(tim->crect->x, tim->crect->y),
        .u0 = u0,   .v0 = v0,
        .um = u1,   .vm = v1,
        .r = 128,   .g = 128,   .b = 128
    };

    return data;
}

PolyData SetupPolyDataRGB(TIM_IMAGE* tim, u_char u0, u_char v0, u_char u1, u_char v1, u_char r, u_char g, u_char b) {
    PolyData data = {
        .tpage = getTPage(tim->mode & 0x3, 0, tim->prect->x, tim->prect->y),
        .clut = getClut(tim->crect->x, tim->crect->y),
        .u0 = u0,   .v0 = v0,
        .um = u1,   .vm = v1,
        .r = r,     .g = g,     .b = b
    };

    return data;
}

// Used for creating a PolyObject out of a number of POLY_FT4 with the same textures
TexturedPolyObject* CreateTexturedPolyObjectFT4(
    long posX, long posY, long posZ, 
    short rotX, short rotY, short rotZ, 
    ushort plen, ushort psides, SVECTOR* vertPtr, long* indPtr, 
    enum DrawPriority drprio, 
    bool coll, int collH, int collW, bool fixed, 
    TIM_IMAGE* tim, 
    u_char u0, u_char v0, u_char uvwidth, u_char uvheight, 
    bool repeating,
    u_char twx, u_char twy, u_char tww, u_char twh) {
    
    TexturedPolyObject* tpobj = calloc(1, sizeof(TexturedPolyObject));
    PolyData* polyData = calloc(1, sizeof(PolyData));
    VECTOR pos = { posX, posY, posZ };
    CVECTOR colour = { 128, 128, 128, 0 };

    if (tpobj != NULL) {
        setVector(&tpobj->polyObj.obj.position, pos.vx * ONE, pos.vy * ONE, pos.vz * ONE);
        setVector(&tpobj->polyObj.obj.rotation, rotX, rotY, rotZ);
        tpobj->polyObj.polyLength = plen;
        tpobj->polyObj.polySides = psides;
        tpobj->polyObj.verticesPtr = vertPtr;
        tpobj->polyObj.indicesPtr = indPtr;
        tpobj->polyObj.polyDataPtr = polyData;
        tpobj->polyObj.drPrio = drprio;
        tpobj->polyObj.collides = coll;
        tpobj->polyObj.boxHeight = collH;
        tpobj->polyObj.boxWidth = collW;
        tpobj->polyObj.obj.isStatic = fixed;
        tpobj->tim = tim;
        tpobj->repeating = repeating;
        setRECT(&tpobj->trect, twx, twy, tww, twh);
        //platform->add = &AddPolyF;

        *polyData = SetupPolyData(tim, u0, v0, uvwidth, uvheight);

        RotMatrix_gte(&tpobj->polyObj.obj.rotation, &tpobj->polyObj.obj.transform);
        TransMatrix(&tpobj->polyObj.obj.transform, &pos);
    }

    return tpobj;
}

TiledTexturedPolyObject* CreateTiledTexturedPolyObjectFT4(
    long posX, long posY, long posZ, 
    short rotX, short rotY, short rotZ, 
    long* indPtr, 
    u_char segX, u_char segY, u_char segZ,
    u_char tileX, u_char tileY, u_char tileZ,
    enum DrawPriority drprio, 
    TIM_IMAGE* tim, 
    u_char u0, u_char v0, u_char uvwidth, u_char uvheight) {
    
    TiledTexturedPolyObject* ttpobj = calloc(1, sizeof(TiledTexturedPolyObject));
    VECTOR pos = { posX, posY, posZ };
    CVECTOR colour = { 128, 128, 128, 0 };

    if (ttpobj != NULL) {
        setVector(&ttpobj->polyObj.obj.position, pos.vx * ONE, pos.vy * ONE, pos.vz * ONE);
        setVector(&ttpobj->polyObj.obj.rotation, rotX, rotY, rotZ);
        ttpobj->polyObj.polySides = 4;
        ttpobj->polyObj.indicesPtr = indPtr;
        ttpobj->polyObj.drPrio = drprio;
        ttpobj->tim = tim;

        ttpobj->totalPolys = tileX * tileY * tileZ;

        PolyData* polyData = malloc(sizeof(PolyData));
        *polyData = SetupPolyData(tim, u0, v0, uvwidth, uvheight);

        // Can be heavily optimised by reusing vertices. Check notebook!
        SVECTOR* tiledVerts = calloc(ttpobj->totalPolys * 4, sizeof(SVECTOR));

        // Temp workaround until objects are redone
        ttpobj->polyObj.polyLength = ttpobj->totalPolys;
        
        ttpobj->polyObj.polyDataPtr = polyData;
        ttpobj->polyObj.verticesPtr = tiledVerts;

        RotMatrix_gte(&ttpobj->polyObj.obj.rotation, &ttpobj->polyObj.obj.transform);
        TransMatrix(&ttpobj->polyObj.obj.transform, &pos);

        long vertIndex = 0;
        
        if (segX != 0 && segZ == 0) {
            for (size_t y = 0; y < tileY; y++) {
                for (size_t x = 0; x < tileX; x++) {
                    setVector(&tiledVerts[vertIndex + 0], segX * x,             (-segY * y) - segY,  0);
                    setVector(&tiledVerts[vertIndex + 1], (segX * x) + segX,    (-segY * y) - segY,  0);
                    setVector(&tiledVerts[vertIndex + 2], (segX * x) + segX,    -segY * y,           0);
                    setVector(&tiledVerts[vertIndex + 3], segX * x,             -segY * y,           0);

                    vertIndex += 4;
                }
            }
        }
        else if (segZ != 0 && segX == 0) {
            for (size_t y = 0; y < tileY; y++) {
                for (size_t z = 0; z < tileZ; z++) {
                    setVector(&tiledVerts[vertIndex + 0], 0, (-segY * y) - segY,    segZ * z);
                    setVector(&tiledVerts[vertIndex + 1], 0, (-segY * y) - segY,    (segZ * z) + segZ);
                    setVector(&tiledVerts[vertIndex + 2], 0, -segY * y,             (segZ * z) + segZ);
                    setVector(&tiledVerts[vertIndex + 3], 0, -segY * y,             segZ * z);

                    vertIndex += 4;
                }
            }
        }
        else if (segY == 0 && segX != 0 && segZ != 0) {
            for (size_t z = 0; z < tileZ; z++) {
                for (size_t x = 0; x < tileX; x++) {
                    setVector(&tiledVerts[vertIndex + 0], segX * x,             0,  (segZ * z) + segZ);
                    setVector(&tiledVerts[vertIndex + 1], (segX * x) + segX,    0,  (segZ * z) + segZ);
                    setVector(&tiledVerts[vertIndex + 2], (segX * x) + segX,    0,  segZ * z);
                    setVector(&tiledVerts[vertIndex + 3], segX * x,             0,  segZ * z);

                    vertIndex += 4;
                }
            }
        }
    }

    return ttpobj;
}

StaticCollisionPolyBox* CreateCollisionPolyBox(
    long posX, long posY, long posZ,
    short rotX, short rotY, short rotZ,
    SVECTOR* vertPtr,
    PolyData pd0, PolyData pd1, PolyData pd2,
    PolyData pd3, PolyData pd4, PolyData pd5) {

    StaticCollisionPolyBox* scpolybox = malloc(sizeof(StaticCollisionPolyBox));
    VECTOR pos = { posX, posY, posZ };

    setVector(&scpolybox->position, pos.vx * ONE, pos.vy * ONE, pos.vz * ONE);
    setVector(&scpolybox->rotation, rotX, rotY, rotZ);
    scpolybox->vertices = vertPtr;
    setVector(&scpolybox->colBox.gridPos, pos.vx, pos.vy, pos.vz);
    scpolybox->colBox.dimensions = scpolybox->vertices[5];
    scpolybox->colBox.dimensions.vy = -scpolybox->colBox.dimensions.vy;
    scpolybox->indices = cubeIndices;

    scpolybox->polyData[0] = pd0;
    scpolybox->polyData[1] = pd1;
    scpolybox->polyData[2] = pd2;
    scpolybox->polyData[3] = pd3;
    scpolybox->polyData[4] = pd4;
    scpolybox->polyData[5] = pd5;

    RotMatrix_gte(&scpolybox->rotation, &scpolybox->transform);
    TransMatrix(&scpolybox->transform, &pos);

    RegisterColPolyBox(scpolybox);

    return scpolybox;
}

CollectibleObject* CreateCollectibleObject(long posX, long posY, long posZ, enum CollectibleType cType) {
    CollectibleObject* cobj = calloc(1, sizeof(CollectibleObject));
    VECTOR pos = { posX, posY, posZ };
    CVECTOR colour = { 128, 128, 128, 0 };

    if (cobj != NULL) {
        setVector(&cobj->position, pos.vx * ONE, pos.vy * ONE, pos.vz * ONE);
        cobj->cType = cType;

        RotMatrix_gte(&cobj->rotation, &cobj->transform);
        TransMatrix(&cobj->transform, &pos);
    }

    return cobj;
}

TestTileMultiPoly* CreateTestMultiPoly(
    long posX, long posY, long posZ, 
    short rotX, short rotY, short rotZ, 
    u_char repeats, u_char subdivs, bool reverseOrder,
    u_char rx, u_char ry, u_char rz,
    u_char width, u_char height, u_char depth,
    TIM_IMAGE* tim, 
    u_char u0, u_char v0, u_char uvwidth, u_char uvheight) {
    
    TestTileMultiPoly* tmp = calloc(1, sizeof(TestTileMultiPoly));

    SVECTOR* vertices = malloc(sizeof(SVECTOR) * 4);
    if (vertices != NULL) {
        if (width == 0) {
            setVector(&vertices[0], 0, -height, 0);
            setVector(&vertices[1], 0, -height, depth);
            setVector(&vertices[2], 0, 0, depth);
            setVector(&vertices[3], 0, 0, 0);
        }
        else if (height == 0) {
            //setVector(&vertices[0], width, 0, 0);
            //setVector(&vertices[1], width, 0, depth);
            //setVector(&vertices[2], 0, 0, depth);
            //setVector(&vertices[3], 0, 0, 0);

            setVector(&vertices[0], 0, 0, depth);
            setVector(&vertices[1], width, 0, depth);
            setVector(&vertices[2], width, 0, 0);
            setVector(&vertices[3], 0, 0, 0);
            
        }
        else {
            setVector(&vertices[0], 0, -height, 0);
            setVector(&vertices[1], width, -height, 0);
            setVector(&vertices[2], width, 0, 0);
            setVector(&vertices[3], 0, 0, 0);
        }
    }

    VECTOR pos = { posX, posY, posZ };
    CVECTOR colour = { 128, 128, 128, 0 };

    if (tmp != NULL) {
        setVector(&tmp->obj.position, pos.vx * ONE, pos.vy * ONE, pos.vz * ONE);
        setVector(&tmp->obj.rotation, rotX, rotY, rotZ);
        tmp->repeats = repeats;
        tmp->subdivs = subdivs;
        tmp->totalPolys = repeats * (subdivs * subdivs);
        tmp->width = width;
        tmp->height = height;
        tmp->depth = depth;
        tmp->tim = tim;
        tmp->reverseOrder = reverseOrder;
        tmp->verticesPtr = vertices;

        if (tmp->reverseOrder) {
            tmp->indicesPtr = reverseWindingIndices;
        }
        else {
            tmp->indicesPtr = windingIndices;
        }

        POLY_FT4* poly = calloc(tmp->totalPolys, sizeof(POLY_FT4));
        for (size_t i = 0; i < tmp->totalPolys; ++i) {
            setPolyFT4(&poly[i]);
            poly[i].tpage = getTPage(tim->mode & 0x3, 0, tim->prect->x, tim->prect->y);
            poly[i].clut = getClut(tim->crect->x, tim->crect->y);
            setRGB0(&poly[i], colour.r, colour.g, colour.b);
        }

        u_char utemp = u0;
        u_char vtemp = v0;
        ushort uvtemp = 0;

        if (subdivs > 1) {
            ushort polyCount = 0;
            u_char divuwidth = uvwidth / subdivs;
            u_char divvheight = uvheight / subdivs;
            
            for (size_t i = 0; i < subdivs; i++) {
                for (size_t j = 0; j < repeats * subdivs; j++) {
                    setUVWH(&poly[polyCount], utemp, vtemp, divuwidth, divvheight);
                    polyCount++;

                    uvtemp = utemp + divuwidth;
                    if ((uvtemp - u0) >= uvwidth || uvtemp > 256) {
                        utemp = u0;
                    }
                    else if (uvtemp == 256) {
                        utemp = 255;
                    }
                    else {
                        utemp += divuwidth;
                    }
                }

                uvtemp = vtemp + divvheight;
                if ((uvtemp - v0) >= uvheight || uvtemp > 256) {
                    vtemp = v0;
                }
                else if (uvtemp == 256) {
                    vtemp = 255;
                }
                else {
                    vtemp += divvheight;
                }

                utemp = u0;
            }
        }
        else {
            for (size_t i = 0; i < tmp->totalPolys; i++) {
                setUVWH(&poly[i], u0, v0, uvwidth, uvheight);
            }
        }

        tmp->polyPtr = poly;

        RotMatrix_gte(&tmp->obj.rotation, &tmp->obj.transform);
        TransMatrix(&tmp->obj.transform, &pos);
    }

    return tmp;
}

// Update poly matrix
static void UpdatePolyObject(PolyObject* pobj) {
    // If object can move in any way, apply velocity
    if (!pobj->obj.isStatic) {
        /*
        if (GetVectorPlaneLength64(&pobj->obj.velocity) > (pobj->obj.maxSpeed << 6)) {
            VECTOR appliedVelocity = { 0 };

            VectorNormal(&pobj->obj.velocity, &appliedVelocity);
            appliedVelocity.vx *= pobj->obj.maxSpeed * ONE;
            appliedVelocity.vz *= pobj->obj.maxSpeed * ONE;

            addVector(&pobj->obj.position, &appliedVelocity);
        }
        else {
            addVector(&pobj->obj.position, &pobj->obj.velocity);
        }
        */
        
        VECTOR gridPos = { 
            pobj->obj.position.vx >> 12, 
            pobj->obj.position.vy >> 12, 
            pobj->obj.position.vz >> 12 
        };

        RotMatrix_gte(&pobj->obj.rotation, &pobj->obj.transform);
        TransMatrix(&pobj->obj.transform, &gridPos);
    }
}

static void CameraTransformMatrix(CameraObject* camera, MATRIX* matrix) {
    gte_CompMatrix(&camera->transform, matrix, &globalRenderTransform);
        
    gte_SetRotMatrix(&globalRenderTransform);
    gte_SetTransMatrix(&globalRenderTransform);
}

static void UpdatePlayerCamera(SVECTOR* cRot) {
    VECTOR cameraOrbitPos = {
        .vx = -player->poly.obj.position.vx >> 12,
        .vy = (-player->poly.obj.position.vy >> 12) + 54,       // 54 = PLAYERHEIGHT (36) * 1.5
        .vz = -player->poly.obj.position.vz >> 12
    };

    RotMatrix_gte(cRot, &cameraRotationMatrix);

    MATRIX pivotMatrix = identity;
    MATRIX invPlayerRotationMatrix;
    InvertMatrix(&player->poly.obj.transform, &invPlayerRotationMatrix);
    MulMatrix(&cameraRotationMatrix, &invPlayerRotationMatrix);
    TransMatrix(&pivotMatrix, &cameraOrbitPos);

    CompMatrix(&cameraRotationMatrix, &pivotMatrix, &player->cameraPtr->transform);

    gte_SetRotMatrix(&player->cameraPtr->transform);
    gte_SetTransMatrix(&player->cameraPtr->transform);
}

static void AddPolyF(PolyObject* pobj) {
    POLY_F4* poly;
    long p, otz, flg;
    int nclip;
    size_t colIndex = 0;

    if (pobj->polySides == 4) {
        for (size_t i = 0; i < (pobj->polyLength * pobj->polySides); i += pobj->polySides, ++poly, ++colIndex) {
            poly = (POLY_F4*)primPtr;

            gte_ldv3(&pobj->verticesPtr[pobj->indicesPtr[i + 0]], &pobj->verticesPtr[pobj->indicesPtr[i + 1]], &pobj->verticesPtr[pobj->indicesPtr[i + 3]]);
            gte_rtpt();
            gte_nclip();
            gte_stopz(&nclip);

            if (nclip <= 0) {
                continue;
            }

            setPolyF4(poly);
            setRGB0(poly, pobj->polyDataPtr[colIndex].r, pobj->polyDataPtr[colIndex].g, pobj->polyDataPtr[colIndex].b);

            gte_stsxy3(&poly->x0, &poly->x1, &poly->x2);
            gte_ldv0(&pobj->verticesPtr[pobj->indicesPtr[i + 2]]);
            gte_rtps();

            gte_avsz4();
            gte_stotz(&otz);
            gte_stsxy(&poly->x3);
            
            if ((otz > 0) && (otz < OTSIZE)) {
                OrderThing(&otz, pobj->drPrio);
                addPrim(cdb->ot[otz], poly);
            }

            primPtr += sizeof(POLY_F4);
        }
    }
    /*
    else if (pobj->polySides == 3) {
        POLY_F3* poly = (POLY_F3*)pobj->polyPtr;

        for (size_t i = 0; i < (pobj->polyLength * pobj->polySides); i += pobj->polySides, ++poly) {
            nclip = RotAverageNclip3(
                &pobj->verticesPtr[pobj->indicesPtr[i + 0]], &pobj->verticesPtr[pobj->indicesPtr[i + 1]],
                &pobj->verticesPtr[pobj->indicesPtr[i + 2]],
                (long*)&poly->x0, (long*)&poly->x1, (long*)&poly->x2, &p, &otz, &flg
            );

            if (nclip <= 0) {
                continue;
            }

            if ((otz > 0) && (otz < OTSIZE)) {
                OrderThing(&otz, pobj->drPrio);
                addPrim(&ot[otz], poly);
            }
        }
    }
    */
}

static void AddPolyFT(TexturedPolyObject* tpobj) {
    POLY_FT4* poly;
    long p, otz, flg;
    int nclip;

    if (tpobj->polyObj.polySides == 4) {
        for (size_t i = 0; i < (tpobj->polyObj.polyLength * tpobj->polyObj.polySides); i += tpobj->polyObj.polySides, ++poly) {
            poly = (POLY_FT4*)primPtr;

            gte_ldv3(&tpobj->polyObj.verticesPtr[tpobj->polyObj.indicesPtr[i + 0]], &tpobj->polyObj.verticesPtr[tpobj->polyObj.indicesPtr[i + 1]], &tpobj->polyObj.verticesPtr[tpobj->polyObj.indicesPtr[i + 3]]);
            gte_rtpt();
            gte_nclip();
            gte_stopz(&nclip);

            if (nclip <= 0) {
                continue;
            }

            setPolyFT4(poly);
            poly->tpage = tpobj->polyObj.polyDataPtr->tpage;
            poly->clut = tpobj->polyObj.polyDataPtr->clut;
            setRGB0(poly, tpobj->polyObj.polyDataPtr->r, tpobj->polyObj.polyDataPtr->g, tpobj->polyObj.polyDataPtr->b);
            setUVWH(poly, tpobj->polyObj.polyDataPtr->u0, tpobj->polyObj.polyDataPtr->v0, tpobj->polyObj.polyDataPtr->um, tpobj->polyObj.polyDataPtr->vm);

            gte_stsxy3(&poly->x0, &poly->x1, &poly->x2);
            gte_ldv0(&tpobj->polyObj.verticesPtr[tpobj->polyObj.indicesPtr[i + 2]]);
            gte_rtps();

            gte_avsz4();
            gte_stotz(&otz);
            gte_stsxy(&poly->x3);
            
            if ((otz > 0) && (otz < OTSIZE)) {
                OrderThing(&otz, tpobj->polyObj.drPrio);
                addPrim(cdb->ot[otz], poly);
            }

            primPtr += sizeof(POLY_FT4);
        }
    }
    /*
    else if (tpobj->polyObj.polySides == 3) {
        POLY_FT3* poly = (POLY_FT3*)tpobj->polyObj.polyPtr;

        for (size_t i = 0; i < (tpobj->polyObj.polyLength * tpobj->polyObj.polySides); i += tpobj->polyObj.polySides, ++poly) {
            nclip = RotAverageNclip3(
                &tpobj->polyObj.verticesPtr[tpobj->polyObj.indicesPtr[i + 0]], &tpobj->polyObj.verticesPtr[tpobj->polyObj.indicesPtr[i + 1]],
                &tpobj->polyObj.verticesPtr[tpobj->polyObj.indicesPtr[i + 2]],
                (long*)&poly->x0, (long*)&poly->x1, (long*)&poly->x2, &p, &otz, &flg
            );

            if (nclip <= 0) {
                continue;
            }

            if ((otz > 0) && (otz < OTSIZE)) {
                OrderThing(&otz, tpobj->polyObj.drPrio);
                addPrim(&ot[otz], poly);
            }
        }
    }
    */
}

// Tiles polys side by side
static void AddTiledPolyFT(TiledTexturedPolyObject* ttpobj) {
    POLY_FT4* poly;
    long p, otz, flg;
    int nclip;

    if (ttpobj->polyObj.polySides == 4) {
        for (size_t i = 0; i < (ttpobj->polyObj.polyLength * ttpobj->polyObj.polySides); i += ttpobj->polyObj.polySides, ++poly) {
            poly = (POLY_FT4*)primPtr;

            gte_ldv3(&ttpobj->polyObj.verticesPtr[i + 0], &ttpobj->polyObj.verticesPtr[i + 1], &ttpobj->polyObj.verticesPtr[i + 3]);
            gte_rtpt();
            gte_nclip();
            gte_stopz(&nclip);
            
            if (nclip <= 0) {
                continue;
            }

            setPolyFT4(poly);
            poly->tpage = ttpobj->polyObj.polyDataPtr->tpage;
            poly->clut = ttpobj->polyObj.polyDataPtr->clut;
            setRGB0(poly, ttpobj->polyObj.polyDataPtr->r, ttpobj->polyObj.polyDataPtr->g, ttpobj->polyObj.polyDataPtr->b);
            setUVWH(poly, ttpobj->polyObj.polyDataPtr->u0, ttpobj->polyObj.polyDataPtr->v0, ttpobj->polyObj.polyDataPtr->um, ttpobj->polyObj.polyDataPtr->vm);

            gte_stsxy3(&poly->x0, &poly->x1, &poly->x2);
            gte_ldv0(&ttpobj->polyObj.verticesPtr[i + 2]);
            gte_rtps();

            gte_avsz4();
            gte_stotz(&otz);
            gte_stsxy(&poly->x3);
            
            if ((otz > 0) && (otz < OTSIZE)) {
                OrderThing(&otz, ttpobj->polyObj.drPrio);
                addPrim(cdb->ot[otz], poly);
            }

            primPtr += sizeof(POLY_FT4);
        }
    }
}

// Polygon box wrapped around collision box
static void AddStaticPolyBox(StaticCollisionPolyBox* scpolybox) {
    POLY_FT4* poly;
    long p, otz, flg;
    int nclip;

    for (size_t i = 0; i < 6; ++i) {
        if (scpolybox->polyData[i].tpage == 0) {
            continue;
        }

        poly = (POLY_FT4*)primPtr;

        gte_ldv3(&scpolybox->vertices[scpolybox->indices[(4 * i) + 0]], &scpolybox->vertices[scpolybox->indices[(4 * i) + 1]], &scpolybox->vertices[scpolybox->indices[(4 * i) + 3]]);
        gte_rtpt();
        gte_nclip();
        gte_stopz(&nclip);

        if (nclip <= 0) {
            continue;
        }

        setPolyFT4(poly);
        poly->tpage = scpolybox->polyData[i].tpage;
        poly->clut = scpolybox->polyData[i].clut;
        setRGB0(poly, scpolybox->polyData[i].r, scpolybox->polyData[i].g, scpolybox->polyData[i].b);
        setUVWH(poly, scpolybox->polyData[i].u0, scpolybox->polyData[i].v0, scpolybox->polyData[i].um, scpolybox->polyData[i].vm);

        gte_stsxy3(&poly->x0, &poly->x1, &poly->x2);
        gte_ldv0(&scpolybox->vertices[scpolybox->indices[(4 * i) + 2]]);
        gte_rtps();

        gte_avsz4();
        gte_stotz(&otz);
        gte_stsxy(&poly->x3);
        
        if ((otz > 0) && (otz < OTSIZE)) {
            addPrim(cdb->ot[otz], poly);
        }

        primPtr += sizeof(POLY_FT4);
    }
}

static void AddCollectible(CollectibleObject* cobj) {
    POLY_FT4* poly;
    long p, otz, flg;

    poly = (POLY_FT4*)primPtr;
    SVECTOR* verticesPtr;
    PolyData* polyData;

    if (cobj->cType == CT_Coin) {
        verticesPtr = collectibleVertices;
        polyData = &coinPolyData;
    }

    gte_ldv3(&verticesPtr[windingIndices[0]], &verticesPtr[windingIndices[1]], &verticesPtr[windingIndices[3]]);
    gte_rtpt();

    setPolyFT4(poly);
    poly->tpage = polyData->tpage;
    poly->clut = polyData->clut;
    setRGB0(poly, polyData->r, polyData->g, polyData->b);
    setUVWH(poly, polyData->u0, polyData->v0, polyData->um, polyData->vm);

    gte_stsxy3(&poly->x0, &poly->x1, &poly->x2);
    gte_ldv0(&verticesPtr[windingIndices[2]]);
    gte_rtps();

    gte_avsz4();
    gte_stotz(&otz);
    gte_stsxy(&poly->x3);
    
    if ((otz > 0) && (otz < OTSIZE)) {
        if ((otz - 1) > 0) {
            otz -= 1;
        }
        addPrim(cdb->ot[otz], poly);
    }

    primPtr += sizeof(POLY_FT4);
}


static void IterateAddCollectibles(LinkedList* list) {
    LLNode* node = list->head;
    CollectibleObject* cobj;

    while (node != NULL) {
        cobj = (CollectibleObject*)node->data;

        CameraTransformMatrix(player->cameraPtr, &cobj->transform);
        AddCollectible(cobj);

        node = node->next;
    }
}

// Tiles and passively tessellates polys side by side (VERY WIP!!!)
static void AddMultiPoly(TestTileMultiPoly* tmp, u_long* ot) {
    long p, otz, flg;
    int nclip;

    POLY_FT4* poly = (POLY_FT4*)tmp->polyPtr;

    u_char vertX = 0;
    u_char vertY = 0;
    u_char vertYinv = tmp->subdivs - 1;

    u_char sw = tmp->width / tmp->subdivs;
    u_char sh = tmp->height / tmp->subdivs;
    u_char sd = tmp->depth / tmp->subdivs;

    for (size_t i = 0; i < tmp->totalPolys; ++i, ++poly) {
        SVECTOR modVertices[4];

        if (tmp->depth == 0) {
            for (size_t v = 0; v < 4; v++) {
                modVertices[v] = tmp->verticesPtr[tmp->indicesPtr[v]];

                modVertices[v].vx /= tmp->subdivs;
                if (modVertices[v].vy == 0) {
                    modVertices[v].vy = -sh * vertYinv;
                }
                else {
                    modVertices[v].vy = -sh * (vertYinv + 1);
                }

                modVertices[v].vx += sw * vertX;
            }
        }
        else if (tmp->height == 0) {
            for (size_t v = 0; v < 4; v++) {
                modVertices[v] = tmp->verticesPtr[tmp->indicesPtr[v]];

                modVertices[v].vx /= tmp->subdivs;

                if (modVertices[v].vz == 0) {
                    modVertices[v].vz = sd * vertYinv;
                }
                else {
                    modVertices[v].vz = sd * (vertYinv + 1);
                }
                
                modVertices[v].vx += sw * vertX;
            }
        }

        if (vertX == (tmp->repeats * tmp->subdivs) - 1) {
            vertX = 0;
            vertY++;
            vertYinv--;
        }
        else {
            vertX++;
        }

        nclip = RotAverageNclip4(
            &modVertices[tmp->indicesPtr[0]], &modVertices[tmp->indicesPtr[1]],
            &modVertices[tmp->indicesPtr[2]], &modVertices[tmp->indicesPtr[3]],
            (long*)&poly->x0, (long*)&poly->x1, (long*)&poly->x3, (long*)&poly->x2, &p, &otz, &flg
        );
        
        if (nclip <= 0) {
            continue;
        }
        
        if ((otz > 0) && (otz < OTSIZE)) {
            //OrderThing(&otz, tpobj->polyObj.drPrio);
            addPrim(&ot[otz], poly);
        }
    }
}


void resetCube(SVECTOR* rot, VECTOR* trans) {
    setVector(rot, 0, 0, 0);
    setVector(trans, 0, (-CUBEHALF - 32) * ONE, DISTTHING * ONE);
}

void SpawnCoin(u_char num) {
    const long x = 64;
    CollectibleObject* coin;

    switch (num) {
        case 1:
            coin = CreateCollectibleObject(
                x + 272, -108, -80, CT_Coin
            );
            break;

        case 2:
            coin = CreateCollectibleObject(
                x + 272, -68, -128, CT_Coin
            );
            break;

        default:
            coin = CreateCollectibleObject(
                x + 272, -148, -32, CT_Coin
            );
            break;
    }

    AppendItemToLinkedList(collectibleList, coin);
}


int main(void) {
    // The PlayStation does not provide a usable heap to the program. Instead, it has to be assigned/claimed by the program
    // The system's main RAM is found at 0x80000000 through 0x80200000 (or 0x80800000 with the 8MB RAM in debug mode over the 2MB standard)
    // First 0x10000 bytes are taken up by the kernel, followed by libraries and data, until finally the rest is available
    // Where the program heap can reasonably be placed should be determined from the Linker Address Map (.map)
    // It is set to start at 0x80040000 here to give hopefully more than enough space to the previous data
    // Function signature for InitHeap() takes a starting address and a size for the heap in bytes (size needs to be a multiple of 4)
    // InitHeap() only allows standard malloc(), calloc(), free(), etc. The numbered versions, eg malloc<2 or 3>(), require use of InitHeap<2 or 3>() instead

    //InitHeap((u_long*)0x80040000, (u_long)0x40000);

    // Revision: End of data segments grabbed directly from linker map and used here, so no need to manually adjust
    u_long heapBuffer;

    if ((__heap_start + (u_long)0x40000) > (u_long)0x80200000) {
        heapBuffer = (u_long)0x80200000 - __heap_start;
    }
    else {
        heapBuffer = (u_long)0x40000;
    }

    InitHeap(heapStart, heapBuffer);

    InitGraphics();
    SpuInit();

    GamePad pad0 = { 0 };
    GamePad pad1 = { 0 };

    CVECTOR col[6];

    // Rotation still works with 4096 (ONE) = 360 degrees
    SVECTOR cameraFixedRot = { 0 };
    cameraFixedRot.vx = 256;
    
    int PadStatus;
    int TPressed = 0;
    int AutoRotate = 1;

    InitPhysicsLists();

    // Initialises the controllers with the Kernel library function. Max data buffer size is 34B
    InitPAD(pad0.dataBuffer, 34, pad1.dataBuffer, 34);
    StartPAD();

    // Seed rand for same result every time
    srand(0);
    for (size_t i = 0; i < ARRAY_SIZE(col); ++i) {
        col[i].r = rand();
        col[i].g = rand();
        col[i].b = rand();
    }

    CreatePlayer(col);
    cameraRotationMatrix = identity;
    //cameraRotationMatrix.t[1] = (PLAYERHEIGHT * 2);
    cameraRotationMatrix.t[2] = CAMERADISTANCE;

    collectibleList = CreateGenericLinkedList();
    coinPolyData = SetupPolyData(&goldCoin_tim, 0, 128, 32, 32);

    PolyObject* colPlatform = CreatePolyObjectF4(
        0, -24, DISTTHING / 2, 
        0, 0, 0,
        6, 4, colBoxVertices, cubeIndices,
        DRP_Neutral, 
        true, 12, 64, true, col
    );

    PolyObject* cube = CreatePolyObjectF4(
        0, -CUBEHALF - 32, DISTTHING, 
        0, 0, 0,
        6, 4, cubeVertices, cubeIndices,
        DRP_Neutral, 
        false, 0, 0, false, col
    );


    TexturedPolyObject* floor = CreateTexturedPolyObjectFT4(
        0, 0, DISTTHING, 
        0, 0, 0,
        1, 4, floorVertices, reverseWindingIndices,
        DRP_Low, 
        false, 0, 0, true, 
        &cobble_tim, 
        0, 127, 128, 128, 
        false,
        0, 127, 128, 128
    );


    TexturedPolyObject* tWallLeft = CreateTexturedPolyObjectFT4(
        -512, 0, 96, 
        0, 0, 0,
        //6, 4, tWallVertices, cubeIndices,
        4, 4, tWallVertices, tubeIndices,
        DRP_Neutral, 
        false, 0, 0, true, 
        &woodPanel_tim, 
        //0, 0, 128, 128, 
        0, 0, 64, 128, 
        true,
        0, 0, 64, 128
    );

    TexturedPolyObject* tDoor = CreateTexturedPolyObjectFT4(
        -512 + WALLHALF * 2, 0, 96, 
        0, 0, 0,
        6, 4, tDoorVertices, cubeIndices,
        DRP_Neutral, 
        false, 0, 0, true, 
        &woodDoor_tim, 
        63, 0, 64, 128, 
        false,
        63, 0, 64, 128
    );

    TexturedPolyObject* tWallRight = CreateTexturedPolyObjectFT4(
        -512 + WALLHALF * 2 + DOORHALF * 2, 0, 96, 
        0, 0, 0,
        //6, 4, tWallVertices, cubeIndices,
        4, 4, tWallVertices, tubeIndices,
        DRP_Neutral, 
        false, 0, 0, true, 
        &woodPanel_tim, 
        //0, 0, 128, 128, 
        0, 0, 64, 128, 
        true,
        0, 0, 64, 128
    );

    TexturedPolyObject* longFloor = CreateTexturedPolyObjectFT4(
        -512, 0, -32, 
        0, 0, 0,
        1, 4, longFloorVertices, reverseWindingIndices,
        DRP_Low, 
        false, 0, 0, true, 
        &cobble_tim, 
        //0, 127, 255, 128,
        0, 127, 128, 128,
        true,
        0, 127, 128, 128
    );

    long startX = 64;

    TiledTexturedPolyObject* tiledWall = CreateTiledTexturedPolyObjectFT4(
        startX, 0, 96, 
        0, 0, 0,
        windingIndices,
        128, 128, 0,
        3, 1, 1, 
        DRP_Neutral,
        &dlv_stonebrick_tim,
        0, 0, 128, 128
    );

    TiledTexturedPolyObject* tiledWall1 = CreateTiledTexturedPolyObjectFT4(
        startX + 384, 0, 96, 
        0, 2048, 0,
        windingIndices,
        0, 128, 128,
        1, 1, 2, 
        DRP_Neutral,
        &dlv_stonebrick_tim,
        0, 0, 128, 128
    );

    TiledTexturedPolyObject* slateFloor = CreateTiledTexturedPolyObjectFT4(
        startX, 0, -160, 
        0, 0, 0,
        windingIndices,
        128, 0, 128,
        3, 1, 2, 
        DRP_Lower,
        &dlv_slate_tim,
        0, 127, 128, 128
    );

    TiledTexturedPolyObject* slateFloor1 = CreateTiledTexturedPolyObjectFT4(
        startX, -128, 96, 
        0, 0, 0,
        windingIndices,
        128, 0, 128,
        3, 1, 1, 
        DRP_Lower,
        &dlv_slate_tim,
        0, 127, 128, 128
    );

    TestTileMultiPoly* testPoly = CreateTestMultiPoly(
        -640, 0, 96,
        0, 0, 0,
        6, 2, false,
        1, 0, 0,
        64, 128, 0,
        &woodPanel_tim, 
        0, 0, 64, 128
    );

    TestTileMultiPoly* testPolyFloor = CreateTestMultiPoly(
        -640, 0, -32,
        0, 0, 0,
        3, 2, false,
        1, 0, 0,
        128, 0, 128,
        &cobble_tim, 
        0, 127, 128, 128
    );
    

    activePolygons[0] = &player->poly;
    activePolygons[1] = cube;
    activePolygons[2] = colPlatform;

    activeTexPolygons[0] = floor;
    activeTexPolygons[1] = tWallLeft;
    activeTexPolygons[2] = tWallRight;
    activeTexPolygons[3] = tDoor;
    activeTexPolygons[4] = longFloor;

    activeTiledTexPolygons[0] = tiledWall;
    activeTiledTexPolygons[1] = tiledWall1;
    activeTiledTexPolygons[2] = slateFloor;
    activeTiledTexPolygons[3] = slateFloor1;


    StaticCollisionPolyBox* testPolyBox = CreateCollisionPolyBox(
        startX + 96, 0, -88,
        0, 0, 0,
        tinyHouseVertices,
        SetupPolyData(&woodDoor_tim, 63, 0, 64, 128),
        SetupPolyData(&woodPanel_tim, 0, 0, 64, 128),
        SetupPolyData(&woodPanel_tim, 0, 0, 64, 128),
        SetupPolyData(&woodPanel_tim, 0, 0, 64, 128),
        SetupPolyData(&woodPanel_tim, 0, 0, 64, 128),
        SetupPolyData(&woodPanel_tim, 0, 0, 64, 128)
    );

    StaticCollisionPolyBox* testPolyBox2 = CreateCollisionPolyBox(
        startX, 0, -88,
        0, 0, 0,
        tinyBoxVertices,
        SetupPolyData(&cobble_tim, 0, 127, 128, 128),
        SetupPolyData(&cobble_tim, 0, 127, 128, 128),
        SetupPolyData(&cobble_tim, 0, 127, 128, 128),
        SetupPolyData(&cobble_tim, 0, 127, 128, 128),
        SetupPolyData(&cobble_tim, 0, 127, 128, 128),
        SetupPolyData(&cobble_tim, 0, 127, 128, 128)
    );

    StaticCollisionPolyBox* testPolyBox3 = CreateCollisionPolyBox(
        startX + 32, 0, -88,
        0, 0, 0,
        boxVertices,
        SetupPolyData(&cobble_tim, 0, 127, 128, 128),
        SetupPolyData(&cobble_tim, 0, 127, 128, 128),
        SetupPolyData(&cobble_tim, 0, 127, 128, 128),
        SetupPolyData(&cobble_tim, 0, 127, 128, 128),
        SetupPolyData(&cobble_tim, 0, 127, 128, 128),
        SetupPolyData(&cobble_tim, 0, 127, 128, 128)
    );

    StaticCollisionPolyBox* testPolyBox4 = CreateCollisionPolyBox(
        startX + 240, -112, -64,
        0, 0, 0,
        platformVertices,
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64),
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64),
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64),
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64),
        SetupPolyData(&dlv_metalpanel_tim, 0, 0, 128, 128),
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64)
    );

    StaticCollisionPolyBox* testPolyBox5 = CreateCollisionPolyBox(
        startX + 240, -72, -112,
        0, 0, 0,
        platformVertices,
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64),
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64),
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64),
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64),
        SetupPolyData(&dlv_metalpanel_tim, 0, 0, 128, 128),
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64)
    );

    StaticCollisionPolyBox* testPolyBox6 = CreateCollisionPolyBox(
        startX + 240, -32, -160,
        0, 0, 0,
        platformVertices,
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64),
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64),
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64),
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64),
        SetupPolyData(&dlv_metalpanel_tim, 0, 0, 128, 128),
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64)
    );

    StaticCollisionPolyBox* testPolyBox7 = CreateCollisionPolyBox(
        startX + 240, -112, 256,
        0, 0, 0,
        platformVertices,
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64),
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64),
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64),
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64),
        SetupPolyData(&dlv_metalpanel_tim, 0, 0, 128, 128),
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64)
    );

    CollisionBox* wallTestCollision = CreateCollisionBox(
        startX, 0, 96,
        384, 128, 128
    );

    CollisionBox* wallTestCollision1 = CreateCollisionBox(
        startX + 384, 0, -160,
        64, 128, 256
    );

    CollisionBox* nostalgia = CreateCollisionBox(
        -32, -12, 224,
        64, 24, 64
    );


    // Wait for VBLANK to allow controller to initialise (otherwise it starts off with pad->buttons being FFFF for the first frame)
    VSync(0);

    while (1) {
        // Translate pad data buffer into a readable format
        UpdatePad(&pad0);

        if (pad0.status == 0) {
            // Only reason I'm keeping this commented out block is to have a visible shorthand for input names
            /*
            if (AutoRotate == 0) {
                if (pad0.buttons & PADL1)      cube.obj.position.vz -= 4;
                if (pad0.buttons & PADR1)      cube.obj.position.vz += 4;
                if (pad0.buttons & PADL2)      cube.obj.rotation.vz -= 8;
                if (pad0.buttons & PADR2)      cube.obj.rotation.vz += 8;
                if (pad0.buttons & PADLup)     cube.obj.rotation.vx -= 8;
                if (pad0.buttons & PADLdown)   cube.obj.rotation.vx += 8;
                if (pad0.buttons & PADLleft)   cube.obj.rotation.vy -= 8;
                if (pad0.buttons & PADLright)  cube.obj.rotation.vy += 8;
                if (pad0.buttons & PADRup)     cube.obj.position.vy -= 2;
                if (pad0.buttons & PADRdown)   cube.obj.position.vy += 2;
                if (pad0.buttons & PADRleft)   cube.obj.position.vx -= 2;
                if (pad0.buttons & PADRright)  cube.obj.position.vx += 2;
            }
            */

            if (pad0.buttons & PADselect) {
                resetCube(&cube->obj.rotation, &cube->obj.position);
            }

            if (pad0.buttons & PADstart) {
                if (TPressed == 0) {
                    AutoRotate = (AutoRotate + 1) & 1;
                }

                TPressed = 1;
            } 
            else {
                TPressed = 0;
            }

            if (pad0.buttons & PADRleft) {
                if (!isSquareHeld) {
                    isSquareHeld = true;
                    SpawnCoin(0);
                }
            }
            else if (isSquareHeld) {
                isSquareHeld = false;
            }

            if (pad0.buttons & PADRup) {
                if (!isTriangleHeld) {
                    isTriangleHeld = true;
                    SpawnCoin(1);
                }
            }
            else if (isTriangleHeld) {
                isTriangleHeld = false;
            }

            if (pad0.buttons & PADRright) {
                if (!isCircleHeld) {
                    isCircleHeld = true;
                    SpawnCoin(2);
                }
            }
            else if (isCircleHeld) {
                isCircleHeld = false;
            }

            if (pad0.buttons & PADL1) {
                //tiledWall->polyObj.obj.rotation.vy -= 32;
                player->poly.obj.rotation.vy -= 32;
                //cameraRotation.vy += 16;
            }

            if (pad0.buttons & PADR1) {
                //tiledWall->polyObj.obj.rotation.vy += 32;
                player->poly.obj.rotation.vy += 32;
                //cameraRotation.vy -= 16;
            }

            // Clean this up later, preferably by writing a separate file for input handling
            VECTOR inputVelocity = { 0 };

            // LS Up (Move forward)
            if (pad0.leftstick.y < ANALOGUE_MINNEG) {
                inputVelocity.vx -= csin(cameraFixedRot.vy) << 2;
				inputVelocity.vz += ccos(cameraFixedRot.vy) << 2;
            }
            // LS Down (Move backward)
            else if (pad0.leftstick.y > ANALOGUE_MINPOS) {
                inputVelocity.vx += csin(cameraFixedRot.vy) << 2;
				inputVelocity.vz -= ccos(cameraFixedRot.vy) << 2;
            }

            // LS Left (Strafe left)
            if (pad0.leftstick.x < ANALOGUE_MINNEG) {
                inputVelocity.vx -= ccos(cameraFixedRot.vy) << 2;
				inputVelocity.vz -= csin(cameraFixedRot.vy) << 2;
            }
            // LS Right (Strafe right)
            else if (pad0.leftstick.x > ANALOGUE_MINPOS) {
                inputVelocity.vx += ccos(cameraFixedRot.vy) << 2;
				inputVelocity.vz += csin(cameraFixedRot.vy) << 2;
            }

            player->poly.obj.velocity.vx = inputVelocity.vx;
            player->poly.obj.velocity.vz = inputVelocity.vz;

            /*
            // RS Up
            if (pad0.rightstick.y < ANALOGUE_MINNEG) {
                player->cameraPtr->rotation.vx -= ONE * 8;
            }
            // RS Down
            else if (pad0.rightstick.y > ANALOGUE_MINPOS) {
                player->cameraPtr->rotation.vx += ONE * 8;
            }
            */

            // RS Left
            if (pad0.rightstick.x < ANALOGUE_MINNEG) {
                player->cameraPtr->rotation.vy -= ONE * 10;
            }
            // RS Right
            else if (pad0.rightstick.x > ANALOGUE_MINPOS) {
                player->cameraPtr->rotation.vy += ONE * 10;
            }
        }

        //cameraFixedRot.vx = player->cameraPtr->rotation.vx >> 12;
        //cameraFixedRot.vx = 256;
        cameraFixedRot.vy = player->cameraPtr->rotation.vy >> 12;
        cameraFixedRot.vz = player->cameraPtr->rotation.vz >> 12;
        //FntPrint("Cam: %04d, %04d, %04d\n\n", cameraFixedRot.vx, cameraFixedRot.vy, cameraFixedRot.vz);

        //FntPrint("PV: %06d, %06d, %06d\n", player->poly.obj.velocity.vx, player->poly.obj.velocity.vy, player->poly.obj.velocity.vz);
        

        // Simulates player movement and resolves collision, then moves the player accordingly
        SimulatePlayerMovementCollision();
        CheckCollectiblePickup(collectibleList);

        if (isPlayerOnCollision) {
            isPlayerOnFloor = true;
        }
        else if (player->poly.obj.position.vy == 0) {
            isPlayerOnFloor = true;
        }
        else if ((player->poly.obj.position.vy + player->poly.obj.velocity.vy) > 0) {
            player->poly.obj.position.vy = 0;
            isPlayerOnFloor = true;
        }
        else {
            isPlayerOnFloor = false;
        }

        if (isPlayerOnFloor) {
            player->poly.obj.velocity.vy = 0;

            if (pad0.buttons & PADRdown) {
                player->poly.obj.velocity.vy -= 8 * ONE;
            }
        }
        else {
            player->poly.obj.velocity.vy += ONE / 2;
        }
        
        if (AutoRotate) {
            cube->obj.rotation.vy += 16;
            cube->obj.rotation.vz += 16;
        }

        for (size_t i = 0; i < ACTIVEPOLYGONCOUNT; i++) {
            UpdatePolyObject(activePolygons[i]);
        }
        for (size_t i = 0; i < ACTIVETEXPOLYGONCOUNT; i++) {
            UpdatePolyObject(&activeTexPolygons[i]->polyObj);
        }
        for (size_t i = 0; i < ACTIVETILEDTEXPOLYGONCOUNT; i++) {
            UpdatePolyObject(&activeTiledTexPolygons[i]->polyObj);
        }

        UpdatePlayerCamera(&cameraFixedRot);

        // Add polys to OT
        for (size_t i = 0; i < ACTIVEPOLYGONCOUNT; i++) {
            CameraTransformMatrix(player->cameraPtr, &activePolygons[i]->obj.transform);
            AddPolyF(activePolygons[i]);
        }
        
        for (size_t i = 0; i < ACTIVETEXPOLYGONCOUNT; i++) {
            CameraTransformMatrix(player->cameraPtr, &activeTexPolygons[i]->polyObj.obj.transform);
            AddPolyFT(activeTexPolygons[i]);
        }

        for (size_t i = 0; i < ACTIVETILEDTEXPOLYGONCOUNT; i++) {
            CameraTransformMatrix(player->cameraPtr, &activeTiledTexPolygons[i]->polyObj.obj.transform);
            AddTiledPolyFT(activeTiledTexPolygons[i]);
        }

        //CameraTransformMatrix(player->cameraPtr, &testPoly->obj.transform);
        //AddMultiPoly(testPoly, cdb->ot);

        //CameraTransformMatrix(player->cameraPtr, &testPolyFloor->obj.transform);
        //AddMultiPoly(testPolyFloor, cdb->ot);

        for (size_t i = 0; i < activeCollisionPolyBoxes->count; i++) {
            StaticCollisionPolyBox* scpolybox = activeCollisionPolyBoxes->array[i];
            CameraTransformMatrix(player->cameraPtr, &scpolybox->transform);
            AddStaticPolyBox(scpolybox);
        }

        IterateAddCollectibles(collectibleList);

        //FntPrint("PT: %04d, %04d, %04d\n", player->poly.obj.transform.t[0], player->poly.obj.transform.t[1], player->poly.obj.transform.t[2]);
        //FntPrint("PV: %06d, %06d, %06d\n", player->poly.obj.velocity.vx, player->poly.obj.velocity.vy, player->poly.obj.velocity.vz);

        //FntPrint("%x\n", heapStart);

        //FntPrint("Coins\nCollected: %d\nRemaining: %d\n", collectedCoins, GetLinkedListLength(collectibleList));
        
        DrawFrame();
    }

    return 0;
}
