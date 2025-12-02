#include <libetc.h>
#include <libgpu.h>
#include <libgte.h>
#include <libapi.h>
#include <stdlib.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))

#define OTSIZE 4096
#define SCREENXRES 640
#define SCREENYRES 480
#define SCREEN_Z 512

#define CUBESIZE 196
#define CUBEHALF CUBESIZE / 2
#define FLOORSIZE 512
#define FLOORHALF FLOORSIZE / 2

#define ANALOGUE_MID 127
#define ANALOGUE_DEADZONE 24
#define ANALOGUE_MINPOS ANALOGUE_MID + ANALOGUE_DEADZONE
#define ANALOGUE_MINNEG ANALOGUE_MID - ANALOGUE_DEADZONE

// Holds Position, Rotation and Transform for an "object". Probably could use a better name.
typedef struct GameObject {
    VECTOR position;
    SVECTOR rotation;
    MATRIX transform;
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
} PolyObject;

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

static void DrawPolyFQuad(u_long* ot, PolyObject* pobj) {
    long p, otz, flg;
    int nclip;

    PushMatrix();

    SetRotMatrix(&pobj->renderTransform);
    SetTransMatrix(&pobj->renderTransform);

    POLY_F4* poly = (POLY_F4*)pobj->polyPtr;

    for (size_t i = 0; i < (pobj->polyLength * pobj->polySides); i += 4, ++poly) {
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
            AddPrim(&ot[otz], poly);
        }
    }

    PopMatrix();
}

void resetCube(SVECTOR* rot, VECTOR* trans) {
    rot->vx = 0;
    rot->vy = 0;
    rot->vz = 0;

    trans->vx = 0;
    trans->vy = 0;
    trans->vz = (SCREEN_Z * 3) / 2;
}

int main(void) {
    DB db[2];
    DB* cdb;

    GamePad pad0 = { 0 };
    GamePad pad1 = { 0 };

    CameraObject camera = {
        .position = { 0, ONE * -100, ONE * -100, 0 },
        .rotation = { 0 }
    };

    POLY_F4 pcube[6];
    PolyObject cube = { 
        .obj = {
            .position = { 0, 0, (SCREEN_Z * 3) / 2, 0 },
            .rotation = { 0 }
        },
        .polyLength = 6,
        .polySides = 4,
        .verticesPtr = cubeVertices,
        .indicesPtr = cubeIndices,
        .polyPtr = &pcube
    };
    CVECTOR col[6];

    POLY_F4 pfloor;
    PolyObject floor = { 
        .obj = {
            .position = { 0, 160, (SCREEN_Z * 3) / 2, 0 },
            .rotation = { ONE / 2, 0, 0, 0 }
        },
        .polyLength = 1,
        .polySides = 4,
        .verticesPtr = floorVertices,
        .indicesPtr = floorIndices,
        .polyPtr = &pfloor
    };
    CVECTOR floorColour = { .r = 0, .g = 128, .b = 0 };

    // Non-fixed-point values for calculating camera position and rotation.
    // Rotation still works with 4096 (ONE) = 360 degrees
    VECTOR rPos = { 0 };
    SVECTOR rRot = { 0 };

    //ushort lastInput = 0;
    
    int PadStatus;
    int TPressed = 0;
    int AutoRotate = 1;

    // Initialises the controllers with the Kernel library function
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
    SetDumpFnt(FntOpen(32, 32, 320, 96, 0, 512));

    SetGeomOffset(SCREENXRES / 2, SCREENYRES / 2);
    SetGeomScreen(SCREEN_Z);

    // Set drawenv defs
    SetDefDrawEnv(&db[0].draw, 0, 0, SCREENXRES, SCREENYRES);
    SetDefDrawEnv(&db[1].draw, 0, 0, SCREENXRES, SCREENYRES);
    SetDefDispEnv(&db[0].disp, 0, 0, SCREENXRES, SCREENYRES);
    SetDefDispEnv(&db[1].disp, 0, 0, SCREENXRES, SCREENYRES);

    // Seed rand for same result every time
    srand(0);
    for (size_t i = 0; i < ARRAY_SIZE(col); ++i) {
        col[i].r = rand();
        col[i].g = rand();
        col[i].b = rand();
    }

    // Init cube and floor
    initCube((POLY_F4*)cube.polyPtr, col);
    initFloor((POLY_F4*)floor.polyPtr, &floorColour);

    RotMatrix(&floor.obj.rotation, &floor.obj.transform);
    TransMatrix(&floor.obj.transform, &floor.obj.position);

    // Actually display the things on screen
    SetDispMask(1);

    PutDrawEnv(&db[0].draw);
    PutDispEnv(&db[0].disp);

    // Wait for VBLANK to allow controller to initialise (otherwise it starts off with pad->buttons being FFFF for the first frame)
    VSync(0);

    while (1) {
        // Swap used buffer
        cdb = (cdb == &db[0]) ? &db[1] : &db[0];

        rRot.vx = camera.rotation.vx >> 12;
        rRot.vy = camera.rotation.vy >> 12;
        rRot.vz = camera.rotation.vz >> 12;

        // Translate pad data buffer into a readable format
        UpdatePad(&pad0);

        if (pad0.status == 0) {
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

            // LS Up
            if (pad0.leftstick.y < ANALOGUE_MINNEG) {
                camera.position.vx -= ((csin(rRot.vy) * ccos(rRot.vx)) >> 12) << 2;
				camera.position.vy += csin(rRot.vx) << 2;
				camera.position.vz += ((ccos(rRot.vy) * ccos(rRot.vx)) >> 12) << 2;
            }
            // LS Down
            else if (pad0.leftstick.y > ANALOGUE_MINPOS) {
                camera.position.vx += ((csin(rRot.vy) * ccos(rRot.vx)) >> 12) << 2;
				camera.position.vy -= csin(rRot.vx) << 2;
				camera.position.vz -= ((ccos(rRot.vy) * ccos(rRot.vx)) >> 12) << 2;
            }

            // LS Left
            if (pad0.leftstick.x < ANALOGUE_MINNEG) {
                camera.position.vx -= ccos(rRot.vy) << 2;
				camera.position.vz -= csin(rRot.vy) << 2;
            }
            // LS Right
            else if (pad0.leftstick.x > ANALOGUE_MINPOS) {
                camera.position.vx += ccos(rRot.vy) << 2;
				camera.position.vz += csin(rRot.vy) << 2;
            }

            // RS Up
            if (pad0.rightstick.y < ANALOGUE_MINNEG) {
                camera.rotation.vx -= ONE * 8;
            }
            // RS Down
            else if (pad0.rightstick.y > ANALOGUE_MINPOS) {
                camera.rotation.vx += ONE * 8;
            }

            // RS Left
            if (pad0.rightstick.x < ANALOGUE_MINNEG) {
                camera.rotation.vy += ONE * 8;
            }
            // RS Right
            else if (pad0.rightstick.x > ANALOGUE_MINPOS) {
                camera.rotation.vy -= ONE * 8;
            }

            RotMatrix(&rRot, &camera.transform);

            rPos.vx = -camera.position.vx >> 12;
            rPos.vy = -camera.position.vy >> 12;
            rPos.vz = -camera.position.vz >> 12;

            ApplyMatrixLV(&camera.transform, &rPos, &rPos);
            TransMatrix(&camera.transform, &rPos);

            SetRotMatrix(&camera.transform);
            SetTransMatrix(&camera.transform);
        }
        
        if (AutoRotate) {
            cube.obj.rotation.vy += 16;
            cube.obj.rotation.vz += 16;
        }

        /*
        if (pad0.buttons > 0) {
            lastInput = pad0.buttons;
        }
        */

        // Updates cube's local transform
        RotMatrix(&cube.obj.rotation, &cube.obj.transform);
        TransMatrix(&cube.obj.transform, &cube.obj.position);

        // Calculates render transforms for cube and floor
        CompMatrixLV(&camera.transform, &cube.obj.transform, &cube.renderTransform);
        CompMatrixLV(&camera.transform, &floor.obj.transform, &floor.renderTransform);

        // Initialises a linked list for OT / clears (zeroes?) OT for current frame in reverse order (faster)
        // "When an OT is initialized, the polygons are unlinked, and only then is a re-sort possible. 
        // Therefore, it is always necessary to initialize an OT prior to executing a sort." - Library Overview, 10-8
        ClearOTagR(cdb->ot, OTSIZE);

        //FntPrint("Status: %x\n", pad0.status);
        //FntPrint("Type: %x\n", pad0.type);
        FntPrint("Buttons: %04x\n", pad0.buttons);
        //FntPrint("LastInput: %04x\n", lastInput);
        FntPrint("Stick L XY: (%02x, %02x)\n", pad0.leftstick.x, pad0.leftstick.y);
        FntPrint("Stick R XY: (%02x, %02x)\n\n", pad0.rightstick.x, pad0.rightstick.y);

        FntPrint("R_Rot: %04d, %04d\n", rRot.vx, rRot.vy);
        FntPrint("R_Pos: %04d, %04d, %04d\n\n", rPos.vx, rPos.vy, rPos.vz);

        FntPrint("C_Rot: %08d, %08d\n", camera.rotation.vx, camera.rotation.vy);
        FntPrint("C_Pos: %08d, %08d, %08d\n", camera.position.vx, camera.position.vy, camera.position.vz);

        // Draw cube and floor
        DrawPolyFQuad(cdb->ot, &floor);
        DrawPolyFQuad(cdb->ot, &cube);

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
