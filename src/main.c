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
#define FLOORSIZE 192
#define FLOORHALF FLOORSIZE / 2
#define COLBOXHEIGHT 12
#define COLBOXHALFWIDTH 32

#define ANALOGUE_MID 127
#define ANALOGUE_DEADZONE 24
#define ANALOGUE_MINPOS ANALOGUE_MID + ANALOGUE_DEADZONE
#define ANALOGUE_MINNEG ANALOGUE_MID - ANALOGUE_DEADZONE


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

static int cubeIndices[] = {
    0, 1, 2, 3, 
    1, 5, 6, 2, 
    5, 4, 7, 6, 
    4, 0, 3, 7, 
    4, 5, 1, 0, 
    6, 7, 3, 2
};

static SVECTOR floorVertices[] = {
    { -FLOORHALF, 0, -FLOORHALF, 0 }, {  FLOORHALF, 0, -FLOORHALF, 0 },
    {  FLOORHALF, 0,  FLOORHALF, 0 }, { -FLOORHALF, 0,  FLOORHALF, 0 }
};

static int floorIndices[] = {
    0, 1, 2, 3
};


PlayerObject* player = NULL;

PolyObject* activePolygons[4];


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

// Update poly matrix
static void UpdatePolyObject(PolyObject* pobj) {
    // If object can move in any way, apply velocity
    if (!pobj->obj.isStatic) {
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
        
        VECTOR gridPos = { pobj->obj.position.vx >> 12, pobj->obj.position.vy >> 12, pobj->obj.position.vz >> 12 };
        RotMatrix_gte(&pobj->obj.rotation, &pobj->obj.transform);
        TransMatrix(&pobj->obj.transform, &gridPos);
    }
}

static void CameraTransformPoly(CameraObject* camera, PolyObject* pobj) {
    // Could get away with replacing this with a global instead of storing the render transform in every object
    gte_CompMatrix(&camera->transform, &pobj->obj.transform, &pobj->renderTransform);
        
    gte_SetRotMatrix(&pobj->renderTransform);
    gte_SetTransMatrix(&pobj->renderTransform);
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

PolyObject* CreatePolyObjectF4(long posX, long posY, long posZ, short rotX, short rotY, short rotZ, ushort plen, ushort psides, SVECTOR* vertPtr, int* indPtr, enum DrawPriority drprio, bool coll, int collH, int collW, bool fixed, CVECTOR* col) {
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
        player->poly.boxHeight = 64;
        player->poly.boxWidth = 48;
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
    InitHeap((u_long*)0x80040000, (u_long)0x20000);

    InitGraphics();

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
    bool isPlayerOnFloor = true;

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

    CVECTOR floorColour = { .r = 0, .g = 128, .b = 0 };
    PolyObject* floor = CreatePolyObjectF4(
        0, 0, DISTTHING, 
        ONE / 2, 0, 0,
        1, 4, floorVertices, floorIndices,
        DRP_Low, 
        false, 0, 0, true, &floorColour
    );

    int heightDif;
    bool occupiesSameSpace = false;

    activePolygons[0] = &player->poly;
    activePolygons[1] = floor;
    activePolygons[2] = cube;
    activePolygons[3] = colPlatform;

    // Wait for VBLANK to allow controller to initialise (otherwise it starts off with pad->buttons being FFFF for the first frame)
    VSync(0);

    while (1) {
        rRot.vx = player->cameraPtr->rotation.vx >> 12;
        rRot.vy = player->cameraPtr->rotation.vy >> 12;
        rRot.vz = player->cameraPtr->rotation.vz >> 12;

        // Translate pad data buffer into a readable format
        UpdatePad(&pad0);

        if (pad0.status == 0) {
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


            // Ugly, slow, and temporary manual platform check, don't mind me
            heightDif = player->poly.obj.transform.t[1] - (colPlatform->obj.transform.t[1] - colPlatform->boxHeight);

            // If player is within the same XY space as the platform
            if (((player->poly.obj.transform.t[0] + player->poly.boxWidth / 2) > (colPlatform->obj.transform.t[0] - colPlatform->boxWidth / 2))
                && ((player->poly.obj.transform.t[0] - player->poly.boxWidth / 2) < (colPlatform->obj.transform.t[0] + colPlatform->boxWidth / 2))
                && ((player->poly.obj.transform.t[2] + player->poly.boxWidth / 2) > (colPlatform->obj.transform.t[2] - colPlatform->boxWidth / 2))
                && ((player->poly.obj.transform.t[2] - player->poly.boxWidth / 2) < (colPlatform->obj.transform.t[2] + colPlatform->boxWidth / 2))) {
                
                occupiesSameSpace = true;
            }
            else {
                occupiesSameSpace = false;
            }

            // Is player on/over platform
            if (heightDif < 0 && ((heightDif << 12) + player->poly.obj.velocity.vy) >= 0 && occupiesSameSpace) {
                isPlayerOnFloor = true;
                player->poly.obj.position.vy = (colPlatform->obj.transform.t[1] - colPlatform->boxHeight) * ONE;
            }
            else if (heightDif == 0 && occupiesSameSpace) {
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
        }
        
        if (AutoRotate) {
            cube->obj.rotation.vy += 16;
            cube->obj.rotation.vz += 16;
        }

        for (size_t i = 0; i < 4; i++) {
            UpdatePolyObject(activePolygons[i]);
        }

        UpdatePlayerCamera(&rPos, &cPos, &rRot);
        

        // Swap used buffer. This and the OT reset/population should be moved to DrawFrame eventually when more things, like the player and camera, are moved to global space
        cdb = (cdb == &db[0]) ? &db[1] : &db[0];

        // Initialises a linked list for OT / clears (zeroes?) OT for current frame in reverse order (faster)
        // "When an OT is initialized, the polygons are unlinked, and only then is a re-sort possible. 
        // Therefore, it is always necessary to initialize an OT prior to executing a sort." - Library Overview, 10-8
        ClearOTagR(cdb->ot, OTSIZE);

        // Add polys to OT
        for (size_t i = 0; i < 4; i++) {
            CameraTransformPoly(player->cameraPtr, activePolygons[i]);
            AddPolyF(activePolygons[i], cdb->ot);
        }

        DrawFrame();
    }

    return 0;
}
