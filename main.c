#include <libetc.h>
#include <libgpu.h>
#include <libgte.h>
#include <libapi.h>
//#include <inline_c.h>
#include <stdlib.h>
#include <stdbool.h>

#include "objects.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))

#define OTSIZE 4096
#define SCREENXRES 640
#define SCREENYRES 480
#define SCREEN_Z 512

#define PLAYERHEIGHT 64
#define PLAYERWIDTHHALF 24
#define CAMERADISTANCE 192 // 160

#define CUBESIZE 96
#define CUBEHALF CUBESIZE / 2
#define FLOORSIZE 256
#define FLOORHALF FLOORSIZE / 2

#define ANALOGUE_MID 127
#define ANALOGUE_DEADZONE 24
#define ANALOGUE_MINPOS ANALOGUE_MID + ANALOGUE_DEADZONE
#define ANALOGUE_MINNEG ANALOGUE_MID - ANALOGUE_DEADZONE

// (Double) Buffer struct
typedef struct DB {
    DRAWENV draw;
    DISPENV disp;
    u_long ot[OTSIZE];
} DB;

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

PlayerObject* player = NULL;

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

static void AddPolyF(PolyObject* pobj, u_long* ot) {
    long p, otz, flg;
    int nclip;

    PushMatrix();

    SetRotMatrix(&pobj->renderTransform);
    SetTransMatrix(&pobj->renderTransform);

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

    PopMatrix();
}

static SVECTOR colBoxVertices[] = {
    { -32, -12, -32, 0 }, {  32, -12, -32, 0 },
    {  32,   0, -32, 0 }, { -32,   0, -32, 0 },
    { -32, -12,  32, 0 }, {  32, -12,  32, 0 },
    {  32,   0,  32, 0 }, { -32,   0,  32, 0 },
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

static void initCube(POLY_F4* cube, CVECTOR* col) {
    for (size_t i = 0; i < 6; ++i) {
        SetPolyF4(&cube[i]);
        setRGB0(&cube[i], col[i].r, col[i].g, col[i].b);
    }
}

static void initFloor(POLY_F4* floor, CVECTOR* col) {
    SetPolyF4(floor);
    setRGB0(floor, col->r, col->g, col->b);
}

PolyObject* CreateColPlatform(VECTOR* pos, CVECTOR* col) {
    PolyObject* platform = calloc(1, sizeof(PolyObject));
    POLY_F4* pplf = calloc(6, sizeof(POLY_F4));

    if (platform != NULL) {
        setVector(&platform->obj.position, pos->vx, pos->vy, pos->vz);
        platform->polyLength = 6;
        platform->polySides = 4;
        platform->verticesPtr = colBoxVertices;
        platform->indicesPtr = cubeIndices;
        platform->polyPtr = pplf;
        platform->drPrio = DRP_Neutral;
        platform->collides = true;
        platform->boxHeight = 12;
        platform->boxWidth = 64;
        //platform->add = &AddPolyF;

        for (size_t i = 0; i < 6; ++i) {
            SetPolyF4(&pplf[i]);
            setRGB0(&pplf[i], col[i].r, col[i].g, col[i].b);
        }

        RotMatrix(&platform->obj.rotation, &platform->obj.transform);
        TransMatrix(&platform->obj.transform, &platform->obj.position);
    }

    return platform;
}

void CreatePlayer(CVECTOR* col) {
    player = calloc(1, sizeof(PlayerObject));

    CameraObject* camera = calloc(1, sizeof(CameraObject));
    POLY_F4* pplayer = calloc(6, sizeof(POLY_F4));

    if (player != NULL) {
        setVector(&player->poly.obj.position, 0, 0, SCREEN_Z * 2);
        player->poly.polyLength = 6;
        player->poly.polySides = 4;
        player->poly.verticesPtr = playerBoxVertices;
        player->poly.indicesPtr = cubeIndices;
        player->poly.polyPtr = pplayer;
        player->poly.drPrio = DRP_Neutral;
        player->poly.collides = false;
        player->poly.boxHeight = 64;
        player->poly.boxWidth = 48;
        //player->poly.add = &AddPolyF;

        if (camera != NULL) {
            player->cameraPtr = camera;
        }

        for (size_t i = 0; i < 6; ++i) {
            SetPolyF4(&pplayer[i]);
            setRGB0(&pplayer[i], col[i].r, col[i].g, col[i].b);
        }

        RotMatrix(&player->poly.obj.rotation, &player->poly.obj.transform);
        TransMatrix(&player->poly.obj.transform, &player->poly.obj.position);
    }
}

static void UpdatePlayer(VECTOR* tPos, VECTOR* cPos) {
    player->cameraPtr->position = player->poly.obj.position;
    player->cameraPtr->position.vy -= PLAYERHEIGHT * ONE * 2;
    player->cameraPtr->position.vz -= CAMERADISTANCE * ONE;

    cPos->vx = -player->cameraPtr->position.vx >> 12;
    cPos->vy = -player->cameraPtr->position.vy >> 12;
    cPos->vz = -player->cameraPtr->position.vz >> 12;

    ApplyMatrixLV(&player->cameraPtr->transform, cPos, cPos);
    TransMatrix(&player->cameraPtr->transform, cPos);
    
    SetRotMatrix(&player->cameraPtr->transform);
    SetTransMatrix(&player->cameraPtr->transform);

    TransMatrix(&player->poly.obj.transform, tPos);
}

void resetCube(SVECTOR* rot, VECTOR* trans) {
    rot->vx = 0;
    rot->vy = 0;
    rot->vz = 0;

    trans->vx = 0;
    trans->vy = -CUBEHALF - 32;
    trans->vz = SCREEN_Z;
}

int main(void) {
    // The PlayStation does not assign a usable heap to the program. Instead, it has to be asigned by the program
    // First 0x10000 bytes are taken up by the kernel, followed by libraries and data, until finally the rest is available
    // The heap takes up 0x80000000 till 0x80200000 (or 0x80800000 with the 8MB RAM in debug mode over the 2MB standard)
    // Where the heap can be allowed to be placed should be determined from the Linker Address Map (.map)
    // It is set to start at 0x80040000 here to give hopefully more than enough space to the previous overhead
    // Function signature for InitHeap() takes a start address and a size in bytes (needs to be a multiple of 4)
    // InitHeap() only allows standard malloc(), calloc(), free(), etc. The numbered versions, eg malloc<2 or 3>(), require use of InitHeap<2 or 3>() instead
    InitHeap((u_long*)0x80040000, (u_long)0x20000);

    DB db[2];
    DB* cdb;

    GamePad pad0 = { 0 };
    GamePad pad1 = { 0 };

    CVECTOR col[6];

    POLY_F4 pcube[6];
    PolyObject cube = { 
        .obj = {
            .position = { 0, -CUBEHALF - 32, SCREEN_Z, 0 },
            .rotation = { 0 }
        },
        .polyLength = 6,
        .polySides = 4,
        .verticesPtr = cubeVertices,
        .indicesPtr = cubeIndices,
        .polyPtr = &pcube,
        .drPrio = DRP_Neutral,
        .collides = false,
        //.add = &AddPolyF
    };
    
    POLY_F4 pfloor;
    PolyObject floor = { 
        .obj = {
            .position = { 0, 0, (SCREEN_Z * 2) / 2, 0 },
            .rotation = { ONE / 2, 0, 0, 0 }
        },
        .polyLength = 1,
        .polySides = 4,
        .verticesPtr = floorVertices,
        .indicesPtr = floorIndices,
        .polyPtr = &pfloor,
        .drPrio = DRP_Low,
        .collides = false,
        //.add = &AddPolyF
    };
    CVECTOR floorColour = { .r = 0, .g = 128, .b = 0 };

    // Non-fixed-point values for calculating camera position and rotation.
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

    // Initialises drawing engine (param = 0 -> complete reset)
    ResetGraph(0);

    // Initialises the Geometry Transformation Engine (GTE)
    InitGeom();

    // Set graphics debugging level
    // 0 = No checking (fastest)
    // 1 = Checks vertices and drawn primitives
    // 2 = Same as above but dumps them instead
    SetGraphDebug(0);

    // Initialises and allows use of debug text
    FntLoad(960, 256);
    SetDumpFnt(FntOpen(32, 32, 320, 160, 0, 512));

    // Set drawenv defs
    SetDefDrawEnv(&db[0].draw, 0, 0, SCREENXRES, SCREENYRES);
    SetDefDrawEnv(&db[1].draw, 0, 0, SCREENXRES, SCREENYRES);
    SetDefDispEnv(&db[0].disp, 0, 0, SCREENXRES, SCREENYRES);
    SetDefDispEnv(&db[1].disp, 0, 0, SCREENXRES, SCREENYRES);

    SetGeomOffset(SCREENXRES / 2, SCREENYRES / 2);
    SetGeomScreen(SCREENXRES / 2);

    // Seed rand for same result every time
    srand(0);
    for (size_t i = 0; i < ARRAY_SIZE(col); ++i) {
        col[i].r = rand();
        col[i].g = rand();
        col[i].b = rand();
    }

    // Init cube and floor
    //initCube((POLY_F4*)player->poly.polyPtr, col);
    CreatePlayer(col);
    bool isPlayerOnFloor = true;

    PolyObject* colPlatform = CreateColPlatform(&(VECTOR){ 0, -24, SCREEN_Z / 2 }, col);

    initCube((POLY_F4*)cube.polyPtr, col);
    initFloor((POLY_F4*)floor.polyPtr, &floorColour);

    // Update floor transform
    RotMatrix(&floor.obj.rotation, &floor.obj.transform);
    TransMatrix(&floor.obj.transform, &floor.obj.position);

    // Actually display the things on screen
    SetDispMask(1);

    PutDrawEnv(&db[0].draw);
    PutDispEnv(&db[0].disp);

    // Wait for VBLANK to allow controller to initialise (otherwise it starts off with pad->buttons being FFFF for the first frame)
    VSync(0);

    int heightDif;
    bool occupiesSameSpace = false;

    while (1) {
        // Swap used buffer
        cdb = (cdb == &db[0]) ? &db[1] : &db[0];

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
                resetCube(&cube.obj.rotation, &cube.obj.position);
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

            // LS Up (Move forward)
            if (pad0.leftstick.y < ANALOGUE_MINNEG) {
                player->poly.obj.position.vx -= ((csin(rRot.vy) * ccos(rRot.vx)) >> 12) << 2;
				player->poly.obj.position.vz += ((ccos(rRot.vy) * ccos(rRot.vx)) >> 12) << 2;
            }
            // LS Down (Move backward)
            else if (pad0.leftstick.y > ANALOGUE_MINPOS) {
                player->poly.obj.position.vx += ((csin(rRot.vy) * ccos(rRot.vx)) >> 12) << 2;
				player->poly.obj.position.vz -= ((ccos(rRot.vy) * ccos(rRot.vx)) >> 12) << 2;
            }

            // LS Left (Strafe left)
            if (pad0.leftstick.x < ANALOGUE_MINNEG) {
                player->poly.obj.position.vx -= ccos(rRot.vy) << 2;
				player->poly.obj.position.vz -= csin(rRot.vy) << 2;
            }
            // LS Right (Strafe right)
            else if (pad0.leftstick.x > ANALOGUE_MINPOS) {
                player->poly.obj.position.vx += ccos(rRot.vy) << 2;
				player->poly.obj.position.vz += csin(rRot.vy) << 2;
            }

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

            // Is player on platform
            if (heightDif == 0 && occupiesSameSpace) {
                isPlayerOnFloor = true;
            }
            else if (player->poly.obj.position.vy == 0) {
                isPlayerOnFloor = true;
            }
            else if (player->poly.obj.position.vy > 0) {
                player->poly.obj.position.vy = 0;
                isPlayerOnFloor = true;
            }
            else {
                isPlayerOnFloor = false;
            }

            if (isPlayerOnFloor) {
                if (pad0.buttons & PADRdown) {
                    player->poly.obj.position.vy -= 48 * ONE;
                }
            }
            else {
                player->poly.obj.position.vy += ONE;
            }

            rPos.vx = player->poly.obj.position.vx >> 12;
            rPos.vy = player->poly.obj.position.vy >> 12;
            rPos.vz = player->poly.obj.position.vz >> 12;

            RotMatrix(&player->poly.obj.rotation, &player->poly.obj.transform);
            RotMatrix(&rRot, &player->cameraPtr->transform);
            
            UpdatePlayer(&rPos, &cPos);
        }
        
        if (AutoRotate) {
            cube.obj.rotation.vy += 16;
            cube.obj.rotation.vz += 16;
        }

        CompMatrixLV(&player->cameraPtr->transform, &player->poly.obj.transform, &player->poly.renderTransform);

        // Updates cube's local transform
        RotMatrix(&cube.obj.rotation, &cube.obj.transform);
        TransMatrix(&cube.obj.transform, &cube.obj.position);

        // Calculates render transforms for cube and floor
        CompMatrixLV(&player->cameraPtr->transform, &cube.obj.transform, &cube.renderTransform);
        CompMatrixLV(&player->cameraPtr->transform, &floor.obj.transform, &floor.renderTransform);
        CompMatrixLV(&player->cameraPtr->transform, &colPlatform->obj.transform, &colPlatform->renderTransform);

        // Initialises a linked list for OT / clears (zeroes?) OT for current frame in reverse order (faster)
        // "When an OT is initialized, the polygons are unlinked, and only then is a re-sort possible. 
        // Therefore, it is always necessary to initialize an OT prior to executing a sort." - Library Overview, 10-8
        ClearOTagR(cdb->ot, OTSIZE);

        //FntPrint("Status: %x\n", pad0.status);
        //FntPrint("Type: %x\n", pad0.type);
        //FntPrint("Buttons: %04x\n", pad0.buttons);
        //FntPrint("LastInput: %04x\n", lastInput);
        //FntPrint("Stick L XY: (%02x, %02x)\n", pad0.leftstick.x, pad0.leftstick.y);
        //FntPrint("Stick R XY: (%02x, %02x)\n\n", pad0.rightstick.x, pad0.rightstick.y);

        FntPrint("CM0: %04d, %04d, %04d\n", player->cameraPtr->transform.m[0][0], player->cameraPtr->transform.m[0][1], player->cameraPtr->transform.m[0][2]);
        FntPrint("CM1: %04d, %04d, %04d\n", player->cameraPtr->transform.m[1][0], player->cameraPtr->transform.m[1][1], player->cameraPtr->transform.m[1][2]);
        FntPrint("CM2: %04d, %04d, %04d\n", player->cameraPtr->transform.m[2][0], player->cameraPtr->transform.m[2][1], player->cameraPtr->transform.m[2][2]);
        FntPrint("CT0: %04d, %04d, %04d\n\n", player->cameraPtr->transform.t[0], player->cameraPtr->transform.t[1], player->cameraPtr->transform.t[2]);

        FntPrint("PM0: %04d, %04d, %04d\n", player->poly.obj.transform.m[0][0], player->poly.obj.transform.m[0][1], player->poly.obj.transform.m[0][2]);
        FntPrint("PM1: %04d, %04d, %04d\n", player->poly.obj.transform.m[1][0], player->poly.obj.transform.m[1][1], player->poly.obj.transform.m[1][2]);
        FntPrint("PM2: %04d, %04d, %04d\n", player->poly.obj.transform.m[2][0], player->poly.obj.transform.m[2][1], player->poly.obj.transform.m[2][2]);
        FntPrint("PT0: %04d, %04d, %04d\n\n", player->poly.obj.transform.t[0], player->poly.obj.transform.t[1], player->poly.obj.transform.t[2]);

        //FntPrint("C_Rot: %08d, %08d\n", camera.rotation.vx, camera.rotation.vy);
        //FntPrint("C_Pos: %08d, %08d, %08d\n", player->cameraPtr->position.vx, player->cameraPtr->position.vy, player->cameraPtr->position.vz);
        //FntPrint("P_Pos: %08d, %08d, %08d\n\n", player->poly.obj.position.vx, player->poly.obj.position.vy, player->poly.obj.position.vz);

        //FntPrint("HDif: %d\n", heightDif);
        //FntPrint("Space: %d\n", occupiesSameSpace);

        // Add polys to OT
        AddPolyF(&player->poly, cdb->ot);
        AddPolyF(&floor, cdb->ot);
        AddPolyF(&cube, cdb->ot);
        AddPolyF(colPlatform, cdb->ot);

        //player->poly.add(&player->poly, cdb->ot);
        //floor.add(&floor, cdb->ot);
        //cube.add(&cube, cdb->ot);
        //colPlatform->add(colPlatform, cdb->ot);

        // Wait for previous frame to have finished drawing if needed
        DrawSync(0);

        // Waits for VBLANK (param = 0 -> waits for generated vertical sync)
        VSync(0);

        // Clear draw buffer with solid colour
        ClearImage(&cdb->draw.clip, 128, 128, 255);

        // Draw from ordering table
        DrawOTag(&cdb->ot[OTSIZE - 1]);
        
        // Draw debug text set in SetDumpFnt with value -1
        FntFlush(-1);
    }

    return 0;
}
