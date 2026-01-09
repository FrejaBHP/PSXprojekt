#include <stdlib.h>
#include <stdbool.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
#include <libapi.h>
#include <inline_n.h>
#include <gtemac.h>

#include "graphics.h"
#include "objects.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))
#define setPosVToGrid(v, _x, _y, _z) \
	(v)->vx = _x >> 12, (v)->vy = _y >> 12, (v)->vz = _z >> 12

#define SCREENXRES 640
#define SCREENYRES 480
#define FOV SCREENXRES / 2
#define DISTTHING 512

#define PLAYERHEIGHT 48
#define PLAYERWIDTHHALF 20
#define CAMERADISTANCE 160 // 160

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
#define ACTIVETILEDTEXPOLYGONCOUNT 1
#define ACTIVECOLBOXCOUNT 6


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

/*
static SVECTOR tWallVertices[] = {
    { -WALLHALF, -WALLHEIGHT, -DOORHALF, 0 }, {  WALLHALF, -WALLHEIGHT, -DOORHALF, 0 },
    {  WALLHALF,  0,          -DOORHALF, 0 }, { -WALLHALF,  0,          -DOORHALF, 0 },
    { -WALLHALF, -WALLHEIGHT,  DOORHALF, 0 }, {  WALLHALF, -WALLHEIGHT,  DOORHALF, 0 },
    {  WALLHALF,  0,           DOORHALF, 0 }, { -WALLHALF,  0,           DOORHALF, 0 },
};

static SVECTOR tDoorVertices[] = {
    { -DOORHALF, -WALLHEIGHT, -DOORHALF, 0 }, {  DOORHALF, -WALLHEIGHT, -DOORHALF, 0 },
    {  DOORHALF,  0,          -DOORHALF, 0 }, { -DOORHALF,  0,          -DOORHALF, 0 },
    { -DOORHALF, -WALLHEIGHT,  DOORHALF, 0 }, {  DOORHALF, -WALLHEIGHT,  DOORHALF, 0 },
    {  DOORHALF,  0,           DOORHALF, 0 }, { -DOORHALF,  0,           DOORHALF, 0 },
};
*/

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

static SVECTOR tiledPanelVertices[] = {
    { 0, -WALLHEIGHT, 0, 0 }, { WALLHALF, -WALLHEIGHT, 0, 0 },
    { WALLHALF, 0, 0, 0 }, { 0, 0, 0, 0 }
};

static long floorIndices[] = {
    0, 3, 2, 1
};

static long tileWallIndices[] = {
    0, 1, 2, 3
};


PlayerObject* player = NULL;
bool isPlayerOnFloor = true;
bool isPlayerOnCollision = false;

PolyObject* activePolygons[ACTIVEPOLYGONCOUNT];
TexturedPolyObject* activeTexPolygons[ACTIVETEXPOLYGONCOUNT];
TexturedPolyObject* activeTiledTexPolygons[ACTIVETILEDTEXPOLYGONCOUNT];

StaticCollisionPolyBox* activeCollisionPolyBoxes[ACTIVECOLBOXCOUNT];

// overlaps is treated as an out parameter
void ScanForOverlaps(const VECTOR* pMins, const VECTOR* pMaxs, const StaticCollisionPolyBox* scpolybox, CollisionOverlaps* overlaps) {
    if (pMins->vx < scpolybox->transform.t[0] + scpolybox->colBox.dimensions.vx
        && (pMaxs->vx > scpolybox->transform.t[0])) {
        
        overlaps->x = true;
    }

    if (pMins->vy > scpolybox->transform.t[1] - scpolybox->colBox.dimensions.vy
        && (pMaxs->vy < scpolybox->transform.t[1])) {
        
        overlaps->y = true;
    }

    if (pMins->vz < scpolybox->transform.t[2] + scpolybox->colBox.dimensions.vz 
        && (pMaxs->vz > scpolybox->transform.t[2])) {
        
        overlaps->z = true;
    }
}

bool CanPlayerStep(const VECTOR* position) {
    bool canStep = true;

    VECTOR gridMins = { 
        (position->vx >> 12) - player->poly.boxWidth / 2,
        (position->vy >> 12),
        (position->vz >> 12) - player->poly.boxWidth / 2
    };

    VECTOR gridMaxs = { 
        (position->vx >> 12) + player->poly.boxWidth / 2,
        (position->vy >> 12) - player->poly.boxHeight,
        (position->vz >> 12) + player->poly.boxWidth / 2
    };

    for (size_t i = 0; i < ACTIVECOLBOXCOUNT; i++) {
        CollisionOverlaps overlaps = { 0 };
        ScanForOverlaps(&gridMins, &gridMaxs, activeCollisionPolyBoxes[i], &overlaps);

        if (overlaps.x == true && overlaps.y == true && overlaps.z == true) {
            canStep = false;
            break;
        }
    }

    return canStep;
}

void SimulatePlayerMovementCollision() {
    isPlayerOnCollision = false;
    bool playerHasStepped = false;

    // In fixed-point units, aka 4096 = 1
    VECTOR playerSimulatedPosition = player->poly.obj.position;
    addVector(&playerSimulatedPosition, &player->poly.obj.velocity);
    VECTOR playerSimulatedPositionFinal = playerSimulatedPosition; // This variable likely is not needed, but kept for now

    // Unsure if these should be recalculated per object, but probably not?
    
    VECTOR playerSimulatedPositionGridMins = { 
        (playerSimulatedPosition.vx >> 12) - player->poly.boxWidth / 2,
        (playerSimulatedPosition.vy >> 12),
        (playerSimulatedPosition.vz >> 12) - player->poly.boxWidth / 2
    };

    VECTOR playerSimulatedPositionGridMaxs = { 
        (playerSimulatedPosition.vx >> 12) + player->poly.boxWidth / 2,
        (playerSimulatedPosition.vy >> 12) - player->poly.boxHeight,
        (playerSimulatedPosition.vz >> 12) + player->poly.boxWidth / 2
    };
    

    for (size_t i = 0; i < ACTIVECOLBOXCOUNT; i++) {
        CollisionOverlaps overlaps = { 0 };
        bool stepping = false;

        /*
        VECTOR playerSimulatedPositionGridMins = { 
            (playerSimulatedPositionFinal.vx >> 12) - player->poly.boxWidth / 2,
            (playerSimulatedPositionFinal.vy >> 12),
            (playerSimulatedPositionFinal.vz >> 12) - player->poly.boxWidth / 2
        };

        VECTOR playerSimulatedPositionGridMaxs = { 
            (playerSimulatedPositionFinal.vx >> 12) + player->poly.boxWidth / 2,
            (playerSimulatedPositionFinal.vy >> 12) - player->poly.boxHeight,
            (playerSimulatedPositionFinal.vz >> 12) + player->poly.boxWidth / 2
        };
        */

        ScanForOverlaps(&playerSimulatedPositionGridMins, &playerSimulatedPositionGridMaxs, activeCollisionPolyBoxes[i], &overlaps);

        //FntPrint("%d %d %d\n", intersectsX, intersectsY, intersectsZ);

        // If player is at least above or underneath a collision box
        if (overlaps.x && overlaps.z) {
            // If player is actually trying to enter collision box
            if (overlaps.y) {
                if (player->poly.obj.velocity.vy == 0 && isPlayerOnFloor && !playerHasStepped) {
                    long stepheight = playerSimulatedPositionGridMins.vy - activeCollisionPolyBoxes[i]->transform.t[1] + activeCollisionPolyBoxes[i]->colBox.dimensions.vy;
                    //FntPrint("StepHeight: %03d\n", stepheight);

                    if (stepheight <= 32 && stepheight > 0) {
                        // Second simulated position to check if player is trying to step up into geometry
                        VECTOR playerStepPosition = playerSimulatedPositionFinal;
                        playerStepPosition.vy = activeCollisionPolyBoxes[i]->position.vy - (activeCollisionPolyBoxes[i]->colBox.dimensions.vy * ONE);

                        if (CanPlayerStep(&playerStepPosition)) {
                            stepping = true;
                            playerHasStepped = true;
                            playerSimulatedPositionFinal.vy = activeCollisionPolyBoxes[i]->position.vy - (activeCollisionPolyBoxes[i]->colBox.dimensions.vy * ONE);
                            isPlayerOnCollision = true;
                        }
                    }
                }

                if (!stepping) {
                    // Bleeding, as in clipping/overlapping - not losing blood
                    long bleed[3]; // X Y Z

                    long bleedXPos = abs((playerSimulatedPositionFinal.vx + ((player->poly.boxWidth / 2) * ONE)) - activeCollisionPolyBoxes[i]->position.vx);
                    long bleedXNeg = abs((playerSimulatedPositionFinal.vx - ((player->poly.boxWidth / 2) * ONE)) - (activeCollisionPolyBoxes[i]->position.vx + (activeCollisionPolyBoxes[i]->colBox.dimensions.vx * ONE)));
                    
                    long bleedYPos = abs((activeCollisionPolyBoxes[i]->position.vy - (activeCollisionPolyBoxes[i]->colBox.dimensions.vy * ONE)) - playerSimulatedPositionFinal.vy);
                    long bleedYNeg = abs(activeCollisionPolyBoxes[i]->position.vy - (playerSimulatedPositionFinal.vy - ((player->poly.boxHeight) * ONE)));

                    long bleedZPos = abs((playerSimulatedPositionFinal.vz + ((player->poly.boxWidth / 2) * ONE)) - activeCollisionPolyBoxes[i]->position.vz);
                    long bleedZNeg = abs((playerSimulatedPositionFinal.vz - ((player->poly.boxWidth / 2) * ONE)) - (activeCollisionPolyBoxes[i]->position.vz + (activeCollisionPolyBoxes[i]->colBox.dimensions.vz * ONE)));

                    if (bleedXPos < bleedXNeg) {
                        bleed[0] = -bleedXPos;
                    }
                    else if (bleedXPos > bleedXNeg) {
                        bleed[0] = bleedXNeg;
                    }

                    if (bleedYPos < bleedYNeg) {
                        bleed[1] = -bleedYPos;
                    }
                    else if (bleedYPos > bleedYNeg) {
                        bleed[1] = bleedYNeg;
                    }

                    if (bleedZPos < bleedZNeg) {
                        bleed[2] = -bleedZPos;
                    }
                    else if (bleedZPos > bleedZNeg) {
                        bleed[2] = bleedZNeg;
                    }

                    size_t index = 0;
                    long bleedValue = abs(bleed[0]);

                    for (size_t i = 0; i < 3; i++) {
                        if (abs(bleed[i]) < bleedValue) {
                            bleedValue = abs(bleed[i]);
                            index = i;
                        }
                    }

                    if (index == 0) {
                        playerSimulatedPositionFinal.vx += bleed[0];
                        player->poly.obj.velocity.vx = 0;
                    }
                    else if (index == 1) {
                        // Need a check somewhere to help with velocity being reset mid-jump
                        // Bug above is very very likely tied to the physics pushing the player up from below
                        // Need to formulate a way to make this not actually happen if the player is ascending
                        playerSimulatedPositionFinal.vy += bleed[1];

                        // Causes issues with gaps, causes jitter on platforms
                        // If moved into if statement below, you still lose momentum on climing some geometry, but you don't jitter in-between platforms
                        player->poly.obj.velocity.vy = 0;

                        // If player is pushed up
                        if (bleed[1] < 0) {
                            isPlayerOnCollision = true;
                        }
                    }
                    else {
                        playerSimulatedPositionFinal.vz += bleed[2];
                        player->poly.obj.velocity.vz = 0;
                    }

                    //FntPrint("Colbox Index: %d\n", i);
                    //FntPrint("BleedX: %06d / %06d\n", bleedXPos, bleedXNeg);
                    //FntPrint("BleedY: %06d / %06d\n", bleedYPos, bleedYNeg);
                    //FntPrint("BleedZ: %06d / %06d\n", bleedZPos, bleedZNeg);

                    //FntPrint("Y: %d, YDim: %d", activeCollisionPolyBoxes[i]->transform.t[1], activeCollisionPolyBoxes[i]->colBox.dimensions.vy);
                }
            }
            // If on the box
            else if (playerSimulatedPositionGridMins.vy == activeCollisionPolyBoxes[i]->transform.t[1] - activeCollisionPolyBoxes[i]->colBox.dimensions.vy
                && player->poly.obj.velocity.vy == 0) {

                isPlayerOnCollision = true;
            }
            
            //player->poly.obj.position = playerSimulatedPosition;
        }
    }

    player->poly.obj.position = playerSimulatedPositionFinal;
}

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
    if (dp == DRP_Low) {
        if (*otz + 256 < OTSIZE) {
            *otz += 256;
        }
    }
    else if (dp == DRP_High) {
        if (*otz - 256 < OTSIZE) {
            *otz -= 256;
        }
    }
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

PolyObject* CreatePolyObjectF4(long posX, long posY, long posZ, short rotX, short rotY, short rotZ, ushort plen, ushort psides, SVECTOR* vertPtr, long* indPtr, enum DrawPriority drprio, bool coll, int collH, int collW, bool fixed, CVECTOR* col) {
    PolyObject* pobj = calloc(1, sizeof(PolyObject));
    POLY_F4* poly = calloc(plen, sizeof(POLY_F4));
    VECTOR pos = { posX, posY, posZ };

    if (pobj != NULL) {
        setVector(&pobj->obj.position, pos.vx * ONE, pos.vy * ONE, pos.vz * ONE);
        setVector(&pobj->obj.rotation, rotX, rotY, rotZ);
        pobj->polyLength = plen;
        pobj->polySides = psides;
        pobj->verticesPtr = vertPtr;
        pobj->indicesPtr = indPtr;
        pobj->polyPtr = poly;
        pobj->drPrio = drprio;
        pobj->collides = coll;
        pobj->boxHeight = collH;
        pobj->boxWidth = collW;
        pobj->obj.isStatic = fixed;
        //platform->add = &AddPolyF;

        for (size_t i = 0; i < plen; ++i) {
            SetPolyF4(&poly[i]);
            setRGB0(&poly[i], col[i].r, col[i].g, col[i].b);
        }

        RotMatrix_gte(&pobj->obj.rotation, &pobj->obj.transform);
        TransMatrix(&pobj->obj.transform, &pos);
    }

    return pobj;
}

POLY_FT4* CreateTexturedPolygon4(TIM_IMAGE* tim, u_char u0, u_char v0, u_char u1, u_char v1) {
    POLY_FT4* poly = calloc(1, sizeof(POLY_FT4));
    SetPolyFT4(poly);

    poly->tpage = getTPage(tim->mode & 0x3, 0, tim->prect->x, tim->prect->y);
    poly->clut = getClut(tim->crect->x, tim->crect->y);
    setRGB0(poly, 128, 128, 128);
    setUVWH(poly, u0, v0, u1, v1);

    return poly;
}

StaticCollisionPolyBox* CreateCollisionPolyBox(
    long posX, long posY, long posZ,
    short rotX, short rotY, short rotZ,
    SVECTOR* vertPtr) {

    StaticCollisionPolyBox* scpolybox = malloc(sizeof(StaticCollisionPolyBox));
    VECTOR pos = { posX, posY, posZ };

    setVector(&scpolybox->position, pos.vx * ONE, pos.vy * ONE, pos.vz * ONE);
    setVector(&scpolybox->rotation, rotX, rotY, rotZ);
    scpolybox->vertices = vertPtr;
    scpolybox->colBox.dimensions = scpolybox->vertices[5];
    scpolybox->colBox.dimensions.vy = -scpolybox->colBox.dimensions.vy;
    scpolybox->indices = cubeIndices;

    for (size_t i = 0; i < 6; i++) {
        scpolybox->polys[i] = NULL;
    }

    RotMatrix_gte(&scpolybox->rotation, &scpolybox->transform);
    TransMatrix(&scpolybox->transform, &pos);

    return scpolybox;
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
    POLY_FT4* poly = calloc(plen, sizeof(POLY_FT4));
    VECTOR pos = { posX, posY, posZ };
    CVECTOR colour = { 128, 128, 128, 0 };

    if (tpobj != NULL) {
        setVector(&tpobj->polyObj.obj.position, pos.vx * ONE, pos.vy * ONE, pos.vz * ONE);
        setVector(&tpobj->polyObj.obj.rotation, rotX, rotY, rotZ);
        tpobj->polyObj.polyLength = plen;
        tpobj->polyObj.polySides = psides;
        tpobj->polyObj.verticesPtr = vertPtr;
        tpobj->polyObj.indicesPtr = indPtr;
        tpobj->polyObj.polyPtr = poly;
        tpobj->polyObj.drPrio = drprio;
        tpobj->polyObj.collides = coll;
        tpobj->polyObj.boxHeight = collH;
        tpobj->polyObj.boxWidth = collW;
        tpobj->polyObj.obj.isStatic = fixed;
        tpobj->tim = tim;
        tpobj->repeating = repeating;
        setRECT(&tpobj->trect, twx, twy, tww, twh);
        //platform->add = &AddPolyF;

        for (size_t i = 0; i < plen; ++i) {
            SetPolyFT4(&poly[i]);
            poly[i].tpage = getTPage(tim->mode & 0x3, 0, tim->prect->x, tim->prect->y);
            poly[i].clut = getClut(tim->crect->x, tim->crect->y);
            setRGB0(&poly[i], colour.r, colour.g, colour.b);
            setUVWH(&poly[i], u0, v0, uvwidth, uvheight);
        }

        RotMatrix_gte(&tpobj->polyObj.obj.rotation, &tpobj->polyObj.obj.transform);
        TransMatrix(&tpobj->polyObj.obj.transform, &pos);
    }

    return tpobj;
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
            tmp->indicesPtr = floorIndices;
        }
        else {
            tmp->indicesPtr = tileWallIndices;
        }

        POLY_FT4* poly = calloc(tmp->totalPolys, sizeof(POLY_FT4));
        for (size_t i = 0; i < tmp->totalPolys; ++i) {
            SetPolyFT4(&poly[i]);
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

void CreatePlayer(CVECTOR* col) {
    player = calloc(1, sizeof(PlayerObject));

    CameraObject* camera = calloc(1, sizeof(CameraObject));
    POLY_F4* pplayer = calloc(6, sizeof(POLY_F4));
    VECTOR pos = { 0, 0, 0 };

    if (player != NULL) {
        setVector(&player->poly.obj.position, pos.vx * ONE, pos.vy * ONE, pos.vz * ONE);
        player->poly.polyLength = 6;
        player->poly.polySides = 4;
        player->poly.verticesPtr = playerBoxVertices;
        player->poly.indicesPtr = cubeIndices;
        player->poly.polyPtr = pplayer;
        player->poly.drPrio = DRP_Neutral;
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
            SetPolyF4(&pplayer[i]);
            setRGB0(&pplayer[i], col[i].r, col[i].g, col[i].b);
        }

        RotMatrix_gte(&player->poly.obj.rotation, &player->poly.obj.transform);
        TransMatrix(&player->poly.obj.transform, &pos);
    }
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
    // Could get away with replacing this with a global instead of storing the render transform in every object
    gte_CompMatrix(&camera->transform, matrix, &globalRenderTransform);
        
    gte_SetRotMatrix(&globalRenderTransform);
    gte_SetTransMatrix(&globalRenderTransform);
}

static void AddPolyF(PolyObject* pobj, u_long* ot) {
    long p, otz, flg;
    int nclip;

    if (pobj->polySides == 4) {
        POLY_F4* poly = (POLY_F4*)pobj->polyPtr;

        for (size_t i = 0; i < (pobj->polyLength * pobj->polySides); i += pobj->polySides, ++poly) {
            // Non-Average version (RotNclip4) presents layering issues, at least tested on floor against Average cube
            nclip = RotAverageNclip4(
                &pobj->verticesPtr[pobj->indicesPtr[i + 0]], &pobj->verticesPtr[pobj->indicesPtr[i + 1]],
                &pobj->verticesPtr[pobj->indicesPtr[i + 2]], &pobj->verticesPtr[pobj->indicesPtr[i + 3]],
                (long*)&poly->x0, (long*)&poly->x1, (long*)&poly->x3, (long*)&poly->x2, &p, &otz, &flg
            );

            if (nclip <= 0) {
                continue;
            }
            
            if ((otz > 0) && (otz < OTSIZE)) {
                OrderThing(&otz, pobj->drPrio);
                AddPrim(&ot[otz], poly);
            }
        }
    }
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
                AddPrim(&ot[otz], poly);
            }
        }
    }
}

//static void AddPolyFT(PolyObject* pobj, DR_TPAGE* tpage, u_long* ot) {
static void AddPolyFT(TexturedPolyObject* tpobj, u_long* ot) {
    long p, otz, flg;
    int nclip;

    if (tpobj->polyObj.polySides == 4) {
        POLY_FT4* poly = (POLY_FT4*)tpobj->polyObj.polyPtr;

        for (size_t i = 0; i < (tpobj->polyObj.polyLength * tpobj->polyObj.polySides); i += tpobj->polyObj.polySides, ++poly) {
            // Non-Average version (RotNclip4) presents layering issues, at least tested on floor against Average cube
            nclip = RotAverageNclip4(
                &tpobj->polyObj.verticesPtr[tpobj->polyObj.indicesPtr[i + 0]], &tpobj->polyObj.verticesPtr[tpobj->polyObj.indicesPtr[i + 1]],
                &tpobj->polyObj.verticesPtr[tpobj->polyObj.indicesPtr[i + 2]], &tpobj->polyObj.verticesPtr[tpobj->polyObj.indicesPtr[i + 3]],
                (long*)&poly->x0, (long*)&poly->x1, (long*)&poly->x3, (long*)&poly->x2, &p, &otz, &flg
            );

            if (nclip <= 0) {
                continue;
            }
            
            if ((otz > 0) && (otz < OTSIZE)) {
                OrderThing(&otz, tpobj->polyObj.drPrio);
                AddPrim(&ot[otz], poly);

                /*
                if (tpobj->repeating) {
                    curTPage = poly->tpage;
                    DR_MODE* drMode = &drModeList[curdrModeIndex];
                    setDrawMode(drMode, 0, 1, curTPage, &tpobj->trect);
                    curdrModeIndex++;
                    AddPrim(&ot[otz], drMode);

                    //DR_MODE* drModeReset = &drModeList[curdrModeIndex];
                    //setDrawMode(drModeReset, 0, 1, curTPage, &resetRect);
                    //curdrModeIndex++;
                    //AddPrim(&ot[otz], &drModeReset);
                }
                */
            }
        }
    }
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
                AddPrim(&ot[otz], poly);

                curTPage = poly->tpage;
                DR_MODE* drMode = &drModeList[curdrModeIndex];
                setDrawMode(drMode, 0, 1, curTPage, &tpobj->trect);

                AddPrim(&ot[otz], drMode);
                curdrModeIndex++;
            }
        }
    }
}

// Tiles polys side by side
static void AddTiledPolyFT(TexturedPolyObject* tpobj, u_long* ot) {
    long p, otz, flg;
    int nclip;

    if (tpobj->polyObj.polySides == 4) {
        POLY_FT4* poly = (POLY_FT4*)tpobj->polyObj.polyPtr;

        for (size_t i = 0; i < (tpobj->polyObj.polyLength * tpobj->polyObj.polySides); i += tpobj->polyObj.polySides, ++poly) {
            SVECTOR modVertices[4];
            
            for (size_t v = 0; v < 4; v++) {
                modVertices[v] = tpobj->polyObj.verticesPtr[tpobj->polyObj.indicesPtr[v]];
                modVertices[v].vx += 64 * (i / tpobj->polyObj.polySides); // 64 is length of wall segment. Magic value will be removed later
            }

            nclip = RotAverageNclip4(
                &modVertices[0], &modVertices[1],
                &modVertices[2], &modVertices[3],
                (long*)&poly->x0, (long*)&poly->x1, (long*)&poly->x3, (long*)&poly->x2, &p, &otz, &flg
            );
            
            if (nclip <= 0) {
                continue;
            }
            
            if ((otz > 0) && (otz < OTSIZE)) {
                OrderThing(&otz, tpobj->polyObj.drPrio);
                AddPrim(&ot[otz], poly);
            }
        }
    }
}

// Tiles polys side by side
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
            AddPrim(&ot[otz], poly);
        }
    }
}

static void AddStaticPolyBox(StaticCollisionPolyBox* scpolybox, u_long* ot) {
    long p, otz, flg;
    int nclip;

    for (size_t i = 0; i < 6; ++i) {
        if (scpolybox->polys[i] == NULL) {
            continue;
        }

        // Non-Average version (RotNclip4) presents layering issues, at least tested on floor against Average cube
        nclip = RotAverageNclip4(
            &scpolybox->vertices[scpolybox->indices[(4 * i) + 0]], &scpolybox->vertices[scpolybox->indices[(4 * i) + 1]],
            &scpolybox->vertices[scpolybox->indices[(4 * i) + 2]], &scpolybox->vertices[scpolybox->indices[(4 * i) + 3]],
            (long*)&scpolybox->polys[i]->x0, (long*)&scpolybox->polys[i]->x1, (long*)&scpolybox->polys[i]->x3, (long*)&scpolybox->polys[i]->x2, &p, &otz, &flg
        );

        if (nclip <= 0) {
            continue;
        }
        
        if ((otz > 0) && (otz < OTSIZE)) {
            AddPrim(&ot[otz], scpolybox->polys[i]);
        }
    }
}

static void UpdatePlayerCamera(VECTOR* tPos, VECTOR* cPos, SVECTOR* cRot) {
    RotMatrix(cRot, &player->cameraPtr->transform);

    tPos->vx = player->poly.obj.position.vx >> 12;
    tPos->vy = player->poly.obj.position.vy >> 12;
    tPos->vz = player->poly.obj.position.vz >> 12;

    player->cameraPtr->position = player->poly.obj.position;
    player->cameraPtr->position.vy -= PLAYERHEIGHT * ONE * 2;
    player->cameraPtr->position.vz -= CAMERADISTANCE * ONE;

    cPos->vx = -player->cameraPtr->position.vx >> 12;
    cPos->vy = -player->cameraPtr->position.vy >> 12;
    cPos->vz = -player->cameraPtr->position.vz >> 12;

    ApplyMatrixLV(&player->cameraPtr->transform, cPos, cPos);
    TransMatrix(&player->cameraPtr->transform, cPos);
    
    gte_SetRotMatrix(&player->cameraPtr->transform);
    gte_SetTransMatrix(&player->cameraPtr->transform);
}

void resetCube(SVECTOR* rot, VECTOR* trans) {
    setVector(rot, 0, 0, 0);
    setVector(trans, 0, (-CUBEHALF - 32) * ONE, DISTTHING * ONE);
}

int main(void) {
    // The PlayStation does not provide a usable heap to the program. Instead, it has to be assigned/claimed by the program
    // The system's main RAM is found at 0x80000000 through 0x80200000 (or 0x80800000 with the 8MB RAM in debug mode over the 2MB standard)
    // First 0x10000 bytes are taken up by the kernel, followed by libraries and data, until finally the rest is available
    // Where the program heap can reasonably be placed should be determined from the Linker Address Map (.map)
    // It is set to start at 0x80040000 here to give hopefully more than enough space to the previous data
    // Function signature for InitHeap() takes a starting address and a size for the heap in bytes (size needs to be a multiple of 4)
    // InitHeap() only allows standard malloc(), calloc(), free(), etc. The numbered versions, eg malloc<2 or 3>(), require use of InitHeap<2 or 3>() instead

    // Maybe __heap_start could be funny
    InitHeap((u_long*)0x80040000, (u_long)0x40000);

    InitGraphics();
    drModeList = malloc(sizeof(DR_MODE) * SPECPRIMSSIZE);

    GamePad pad0 = { 0 };
    GamePad pad1 = { 0 };

    CVECTOR col[6];

    // Rotation still works with 4096 (ONE) = 360 degrees
    VECTOR rPos = { 0 };
    SVECTOR rRot = { 0 };

    VECTOR cPos = { 0 };
    
    int PadStatus;
    int TPressed = 0;
    int AutoRotate = 1;

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
        //ONE / 2, 0, 0,
        0, 0, 0,
        1, 4, floorVertices, floorIndices,
        DRP_Low, 
        false, 0, 0, true, 
        &cobble_tim, 
        0, 127, 128, 128, 
        false,
        0, 127, 128, 128
    );

    TexturedPolyObject* tWallLeft = CreateTexturedPolyObjectFT4(
        192, 0, 96, 
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
        //192 + WALLHALF * 2 - DOORHALF, 0, 96, 
        192 + WALLHALF * 2, 0, 96, 
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
        //192 + WALLHALF * 2 + DOORHALF * 2, 0, 96, 
        192 + WALLHALF * 2 + DOORHALF * 2, 0, 96, 
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
        //288, 0, -32, 
        192, 0, -32, 
        //ONE / 2, 0, 0,
        0, 0, 0,
        1, 4, longFloorVertices, floorIndices,
        DRP_Low, 
        false, 0, 0, true, 
        &cobble_tim, 
        //0, 127, 255, 128,
        0, 127, 128, 128,
        true,
        0, 127, 128, 128
    );

    TexturedPolyObject* tiledWall = CreateTexturedPolyObjectFT4(
        192 + WALLHALF * 4 + DOORHALF * 3, 0, 96, 
        0, 0, 0,
        //6, 4, tWallVertices, cubeIndices,
        5, 4, tiledPanelVertices, tileWallIndices,
        DRP_Neutral, 
        false, 0, 0, true, 
        &woodPanel_tim, 
        //0, 0, 128, 128, 
        0, 0, 64, 128, 
        false,
        0, 0, 64, 128
    );


    TestTileMultiPoly* testPoly = CreateTestMultiPoly(
        -320, 0, 96,
        0, 0, 0,
        6, 2, false,
        1, 0, 0,
        64, 128, 0,
        &woodPanel_tim, 
        0, 0, 64, 128
    );

    TestTileMultiPoly* testPolyFloor = CreateTestMultiPoly(
        -320, 0, -32,
        0, 0, 0,
        3, 2, false,
        1, 0, 0,
        128, 0, 128,
        &cobble_tim, 
        0, 127, 128, 128
    );
    

    int heightDif;
    bool occupiesSameSpace = false;

    activePolygons[0] = &player->poly;
    activePolygons[1] = cube;
    activePolygons[2] = colPlatform;

    activeTexPolygons[0] = floor;
    activeTexPolygons[1] = tWallLeft;
    activeTexPolygons[2] = tWallRight;
    activeTexPolygons[3] = tDoor;
    activeTexPolygons[4] = longFloor;

    activeTiledTexPolygons[0] = tiledWall;


    // Should really do something about these "constructors". They're really long

    StaticCollisionPolyBox* testPolyBox = CreateCollisionPolyBox(
        640, 0, -64,
        0, 0, 0,
        tinyHouseVertices
    );

    testPolyBox->polys[0] = CreateTexturedPolygon4(&woodDoor_tim, 63, 0, 64, 128);
    testPolyBox->polys[1] = CreateTexturedPolygon4(&woodPanel_tim, 0, 0, 64, 128);
    testPolyBox->polys[2] = CreateTexturedPolygon4(&woodPanel_tim, 0, 0, 64, 128);
    testPolyBox->polys[3] = CreateTexturedPolygon4(&woodPanel_tim, 0, 0, 64, 128);
    testPolyBox->polys[4] = CreateTexturedPolygon4(&woodPanel_tim, 0, 0, 64, 128);
    testPolyBox->polys[5] = CreateTexturedPolygon4(&woodPanel_tim, 0, 0, 64, 128);

    activeCollisionPolyBoxes[0] = testPolyBox;


    StaticCollisionPolyBox* testPolyBox2 = CreateCollisionPolyBox(
        544, 0, -64,
        0, 0, 0,
        tinyBoxVertices
    );

    testPolyBox2->polys[0] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox2->polys[1] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox2->polys[2] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox2->polys[3] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox2->polys[4] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox2->polys[5] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);

    activeCollisionPolyBoxes[1] = testPolyBox2;


    StaticCollisionPolyBox* testPolyBox3 = CreateCollisionPolyBox(
        576, 0, -64,
        0, 0, 0,
        boxVertices
    );

    testPolyBox3->polys[0] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox3->polys[1] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox3->polys[2] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox3->polys[3] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox3->polys[4] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox3->polys[5] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);

    activeCollisionPolyBoxes[2] = testPolyBox3;


    StaticCollisionPolyBox* testPolyBox4 = CreateCollisionPolyBox(
        784, -112, -64,
        0, 0, 0,
        platformVertices
    );

    testPolyBox4->polys[0] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox4->polys[1] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox4->polys[2] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox4->polys[3] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox4->polys[4] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox4->polys[5] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);

    activeCollisionPolyBoxes[3] = testPolyBox4;


    StaticCollisionPolyBox* testPolyBox5 = CreateCollisionPolyBox(
        784, -72, -96,
        0, 0, 0,
        platformVertices
    );

    testPolyBox5->polys[0] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox5->polys[1] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox5->polys[2] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox5->polys[3] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox5->polys[4] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox5->polys[5] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);

    activeCollisionPolyBoxes[4] = testPolyBox5;


    StaticCollisionPolyBox* testPolyBox6 = CreateCollisionPolyBox(
        784, -32, -128,
        0, 0, 0,
        platformVertices
    );

    testPolyBox6->polys[0] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox6->polys[1] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox6->polys[2] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox6->polys[3] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox6->polys[4] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);
    testPolyBox6->polys[5] = CreateTexturedPolygon4(&cobble_tim, 0, 127, 128, 128);

    activeCollisionPolyBoxes[5] = testPolyBox6;

    // Wait for VBLANK to allow controller to initialise (otherwise it starts off with pad->buttons being FFFF for the first frame)
    VSync(0);

    while (1) {
        rRot.vx = player->cameraPtr->rotation.vx >> 12;
        rRot.vy = player->cameraPtr->rotation.vy >> 12;
        rRot.vz = player->cameraPtr->rotation.vz >> 12;

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

            // Clean this up later, preferably by writing a separate file for input handling
            VECTOR inputVelocity = { 0 };

            // LS Up (Move forward)
            if (pad0.leftstick.y < ANALOGUE_MINNEG) {
                inputVelocity.vx -= ((csin(rRot.vy) * ccos(rRot.vx)) >> 12) << 2;
				inputVelocity.vz += ((ccos(rRot.vy) * ccos(rRot.vx)) >> 12) << 2;
            }
            // LS Down (Move backward)
            else if (pad0.leftstick.y > ANALOGUE_MINPOS) {
                inputVelocity.vx += ((csin(rRot.vy) * ccos(rRot.vx)) >> 12) << 2;
				inputVelocity.vz -= ((ccos(rRot.vy) * ccos(rRot.vx)) >> 12) << 2;
            }

            // LS Left (Strafe left)
            if (pad0.leftstick.x < ANALOGUE_MINNEG) {
                inputVelocity.vx -= ccos(rRot.vy) << 2;
				inputVelocity.vz -= csin(rRot.vy) << 2;
            }
            // LS Right (Strafe right)
            else if (pad0.leftstick.x > ANALOGUE_MINPOS) {
                inputVelocity.vx += ccos(rRot.vy) << 2;
				inputVelocity.vz += csin(rRot.vy) << 2;
            }

            player->poly.obj.velocity.vx = inputVelocity.vx;
            player->poly.obj.velocity.vz = inputVelocity.vz;

            // RS Up
            if (pad0.rightstick.y < ANALOGUE_MINNEG) {
                player->cameraPtr->rotation.vx -= ONE * 8;
            }
            // RS Down
            else if (pad0.rightstick.y > ANALOGUE_MINPOS) {
                player->cameraPtr->rotation.vx += ONE * 8;
            }

            // RS Left
            if (pad0.rightstick.x < ANALOGUE_MINNEG) {
                player->cameraPtr->rotation.vy += ONE * 8;
            }
            // RS Right
            else if (pad0.rightstick.x > ANALOGUE_MINPOS) {
                player->cameraPtr->rotation.vy -= ONE * 8;
            }
        }

        // Simulates player movement and resolves collision, then moves the player accordingly
        SimulatePlayerMovementCollision();

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

        UpdatePlayerCamera(&rPos, &cPos, &rRot);

        // Swap used buffer. This and the OT reset/population should be moved to DrawFrame eventually when more things, like the player and camera, are moved to global space
        cdb = (cdb == &db[0]) ? &db[1] : &db[0];

        // Initialises a linked list for OT / clears (zeroes?) OT for current frame in reverse order (faster)
        // "When an OT is initialized, the polygons are unlinked, and only then is a re-sort possible. 
        // Therefore, it is always necessary to initialize an OT prior to executing a sort." - Library Overview, 10-8
        ClearOTagR(cdb->ot, OTSIZE);

        // Add polys to OT
        for (size_t i = 0; i < ACTIVEPOLYGONCOUNT; i++) {
            CameraTransformMatrix(player->cameraPtr, &activePolygons[i]->obj.transform);
            AddPolyF(activePolygons[i], cdb->ot);
        }
        
        for (size_t i = 0; i < ACTIVETEXPOLYGONCOUNT; i++) {
            CameraTransformMatrix(player->cameraPtr, &activeTexPolygons[i]->polyObj.obj.transform);
            AddPolyFT(activeTexPolygons[i], cdb->ot);
        }

        for (size_t i = 0; i < ACTIVETILEDTEXPOLYGONCOUNT; i++) {
            CameraTransformMatrix(player->cameraPtr, &activeTiledTexPolygons[i]->polyObj.obj.transform);
            AddTiledPolyFT(activeTiledTexPolygons[i], cdb->ot);
        }

        CameraTransformMatrix(player->cameraPtr, &testPoly->obj.transform);
        AddMultiPoly(testPoly, cdb->ot);

        CameraTransformMatrix(player->cameraPtr, &testPolyFloor->obj.transform);
        AddMultiPoly(testPolyFloor, cdb->ot);

        for (size_t i = 0; i < ACTIVECOLBOXCOUNT; i++) {
            CameraTransformMatrix(player->cameraPtr, &activeCollisionPolyBoxes[i]->transform);
            AddStaticPolyBox(activeCollisionPolyBoxes[i], cdb->ot);
        }

        //FntPrint("PT: %04d, %04d, %04d\n", player->poly.obj.transform.t[0], player->poly.obj.transform.t[1], player->poly.obj.transform.t[2]);
        //FntPrint("PV : %06d, %06d, %06d\n", player->poly.obj.velocity.vx, player->poly.obj.velocity.vy, player->poly.obj.velocity.vz);

        DrawFrame();
    }

    return 0;
}
