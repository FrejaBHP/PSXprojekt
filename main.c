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

typedef struct GameObject {
    SVECTOR rotation;
    VECTOR position;
    MATRIX transform;
    void* polyPtr;
    ushort polyLength;
} GameObject;

// Draw buffer struct
typedef struct DB {
    DRAWENV draw;
    DISPENV disp;
    u_long ot[OTSIZE];
    POLY_F4 s[6];
    POLY_F4 floor;
} DB;

typedef struct Vector2UB {
    u_char x;
    u_char y;
} Vector2UB;

typedef struct GamePad {
    u_char dataBuffer[34];
    u_char status;
    u_char type;
    ushort buttons;
    Vector2UB leftstick;
    Vector2UB rightstick;
} GamePad;

void UpdatePad(GamePad* pad) {
    pad->status = pad->dataBuffer[0];
    pad->type = pad->dataBuffer[1];
    pad->buttons = 0xFFFF - ((pad->dataBuffer[2] << 8) | (pad->dataBuffer[3])); // Stores buffer[2] in the upper 8 bits and [3] in the lower 8 bits. Eases parsing later
    pad->leftstick.x = pad->dataBuffer[6];
    pad->leftstick.y = pad->dataBuffer[7];
    pad->rightstick.x = pad->dataBuffer[4];
    pad->rightstick.y = pad->dataBuffer[5];
}

static SVECTOR cube_vertices[] = {
    { -CUBEHALF, -CUBEHALF, -CUBEHALF, 0 }, { CUBEHALF , -CUBEHALF, -CUBEHALF, 0 },
    { CUBEHALF , CUBEHALF , -CUBEHALF, 0 }, { -CUBEHALF, CUBEHALF , -CUBEHALF, 0 },
    { -CUBEHALF, -CUBEHALF, CUBEHALF , 0 }, { CUBEHALF , -CUBEHALF, CUBEHALF , 0 },
    { CUBEHALF , CUBEHALF , CUBEHALF , 0 }, { -CUBEHALF, CUBEHALF , CUBEHALF , 0 },
};

static int cube_indices[] = {
    0, 1, 2, 3, 
    1, 5, 6, 2, 
    5, 4, 7, 6, 
    4, 0, 3, 7, 
    4, 5, 1, 0, 
    6, 7, 3, 2,
};

static SVECTOR floorVertices[] = {
    { -CUBESIZE * 2, 0, -CUBESIZE * 2, 0 }, { CUBESIZE * 2 , 0, -CUBESIZE * 2, 0 },
    { CUBESIZE * 2 , 0, CUBESIZE * 2 , 0 }, { -CUBESIZE * 2, 0, CUBESIZE * 2 , 0 },
};

static void init_cube(DB *db, CVECTOR *col) {
    size_t i;

    for (i = 0; i < ARRAY_SIZE(db->s); ++i) {
        SetPolyF4(&db->s[i]);
        setRGB0(&db->s[i], col[i].r, col[i].g, col[i].b);
    }
}

static void add_cube(u_long *ot, POLY_F4 *s, MATRIX *transform) {
    long p, otz, flg;
    int nclip;

    PushMatrix();

    SetRotMatrix(transform);
    SetTransMatrix(transform);

    for (size_t i = 0; i < ARRAY_SIZE(cube_indices); i += 4, ++s) {
        nclip = RotAverageNclip4(
            &cube_vertices[cube_indices[i + 0]], &cube_vertices[cube_indices[i + 1]],
            &cube_vertices[cube_indices[i + 2]], &cube_vertices[cube_indices[i + 3]],
            (long*)&s->x0, (long*)&s->x1, (long*)&s->x3, (long*)&s->x2, &p, &otz, &flg
        );

        if (nclip <= 0) {
            continue;
        }

        if ((otz > 0) && (otz < OTSIZE)) {
            AddPrim(&ot[otz], s);
        }
    }

    PopMatrix();
}

static void initFloor(DB* db, CVECTOR* col) {
    SetPolyF4(&db->floor);
    setRGB0(&db->floor, col->r, col->g, col->b);
}

static void drawFloor(u_long* ot, POLY_F4* s, MATRIX* transform) {
    long p, otz, flg;
    int nclip;

    PushMatrix();

    SetRotMatrix(transform);
    SetTransMatrix(transform);

    nclip = RotAverageNclip4(
        &floorVertices[0], &floorVertices[1],
        &floorVertices[2], &floorVertices[3],
        (long*)&s->x0, (long*)&s->x1, (long*)&s->x3, (long*)&s->x2, &p, &otz, &flg
    );

    // nclip <= 0 && 
    if ((otz > 0) && (otz < OTSIZE)) {
        AddPrim(&ot[otz], s);
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
    
    SVECTOR rotation = { 0 };
    VECTOR translation = { 0, 0, (SCREEN_Z * 3) / 2, 0 };
    MATRIX transform;
    CVECTOR col[6];

    VECTOR camRot = { 0 };
    VECTOR camPos = { 0, ONE*-200, 0, 0 };
    MATRIX camTransform;

    SVECTOR tempRot = { 0 };
    VECTOR tempPos = { 0 };

    MATRIX cubeRenderTransform;

    CVECTOR floorColour;
    floorColour.r = 0;
    floorColour.g = 128;
    floorColour.b = 0;

    SVECTOR floorRot = { ONE / 2, 0, 0, 0 };
    VECTOR floorPos = { 0, 160, (SCREEN_Z * 3) / 2, 0 };
    MATRIX floorTransform;

    MATRIX floorRenderTransform;

    ushort lastInput = 0;
    
    int PadStatus;
    int TPressed = 0;
    int AutoRotate = 1;

    // Initialises the controller. Should always take 0. Used for standard analogue controllers only, otherwise look up PadInitDirect()
    // Note: Use InitPAD from the Kernel library instead, as below
    //PadInit(0);

    InitPAD(pad0.dataBuffer, 34, pad1.dataBuffer, 34);
    StartPAD();

    // Initialises drawing engine (param = 0 -> complete reset)
    ResetGraph(0);

    // Initialises the Geometry Transformation Engine (GTE)
    InitGeom();

    // Set graphics debugging level
    // 0 = No checking (fastest)
    // 1 = Checks verteces and drawn primitives
    // 2 = Same as above but dumps them instead
    SetGraphDebug(0);

    // Init debug text
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

    // Init cube for both draw buffers
    init_cube(&db[0], col);
    init_cube(&db[1], col);

    initFloor(&db[0], &floorColour);
    initFloor(&db[1], &floorColour);

    RotMatrix(&floorRot, &floorTransform);
    TransMatrix(&floorTransform, &floorPos);

    // Actually display the things on screen
    SetDispMask(1);

    PutDrawEnv(&db[0].draw);
    PutDispEnv(&db[0].disp);

    VSync(0);

    while (1) {
        // Flip used draw buffer
        cdb = (cdb == &db[0]) ? &db[1] : &db[0];

        tempRot.vx = camRot.vx >> 12;
        tempRot.vy = camRot.vy >> 12;
        tempRot.vz = camRot.vz >> 12;

        UpdatePad(&pad0);

        //if (pad0.status == 0 && pad0.buttons != 0xFFFF) {
        if (pad0.status == 0) {
            if (AutoRotate == 0) {
                if (pad0.buttons & PADL1)      translation.vz -= 4;
                if (pad0.buttons & PADR1)      translation.vz += 4;
                if (pad0.buttons & PADL2)      rotation.vz -= 8;
                if (pad0.buttons & PADR2)      rotation.vz += 8;
                if (pad0.buttons & PADLup)     rotation.vx -= 8;
                if (pad0.buttons & PADLdown)   rotation.vx += 8;
                if (pad0.buttons & PADLleft)   rotation.vy -= 8;
                if (pad0.buttons & PADLright)  rotation.vy += 8;
                if (pad0.buttons & PADRup)     translation.vy -= 2;
                if (pad0.buttons & PADRdown)   translation.vy += 2;
                if (pad0.buttons & PADRleft)   translation.vx -= 2;
                if (pad0.buttons & PADRright)  translation.vx += 2;
            }

            if (pad0.buttons & PADselect) {
                resetCube(&rotation, &translation);
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
            if (pad0.leftstick.y < 103) {
                camPos.vx -= ((csin(tempRot.vy) * ccos(tempRot.vx)) >> 12) << 2;
				camPos.vy += csin(tempRot.vx) << 2;
				camPos.vz += ((ccos(tempRot.vy) * ccos(tempRot.vx)) >> 12) << 2;
            }
            // LS Down
            else if (pad0.leftstick.y > 151) {
                camPos.vx += ((csin(tempRot.vy) * ccos(tempRot.vx)) >> 12) << 2;
				camPos.vy -= csin(tempRot.vx) << 2;
				camPos.vz -= ((ccos(tempRot.vy) * ccos(tempRot.vx)) >> 12) << 2;
            }

            // LS Left
            if (pad0.leftstick.x < 103) {
                camPos.vx -= ccos(tempRot.vy) << 2;
				camPos.vz -= csin(tempRot.vy) << 2;
            }
            // LS Right
            else if (pad0.leftstick.x > 151) {
                camPos.vx += ccos(tempRot.vy) << 2;
				camPos.vz += csin(tempRot.vy) << 2;
            }


            // RS Up
            if (pad0.rightstick.y < 103) {
                camRot.vx -= ONE*8;
            }
            // RS Down
            else if (pad0.rightstick.y > 151) {
                camRot.vx += ONE*8;
            }

            // RS Left
            if (pad0.rightstick.x < 103) {
                camRot.vy += ONE*8;
            }
            // RS Right
            else if (pad0.rightstick.x > 151) {
                camRot.vy -= ONE*8;
            }

            RotMatrix(&tempRot, &camTransform);

            tempPos.vx = -camPos.vx >> 12;
            tempPos.vy = -camPos.vy >> 12;
            tempPos.vz = -camPos.vz >> 12;

            ApplyMatrixLV(&camTransform, &tempPos, &tempPos);
            TransMatrix(&camTransform, &tempPos);

            SetRotMatrix(&camTransform);
            SetTransMatrix(&camTransform);
        }
        
        if (AutoRotate) {
            rotation.vy += 16;
            rotation.vz += 16;
        }

        if (pad0.buttons > 0) {
            lastInput = pad0.buttons;
        }

        RotMatrix(&rotation, &transform);
        TransMatrix(&transform, &translation);

        CompMatrixLV(&camTransform, &transform, &cubeRenderTransform);
        CompMatrixLV(&camTransform, &floorTransform, &floorRenderTransform);

        // Initialises a linked list for OT / clears (zeroes?) OT for current frame in reverse order (faster)
        ClearOTagR(cdb->ot, OTSIZE);

        //FntPrint("Status: %x\n", pad0.status);
        //FntPrint("Type: %x\n", pad0.type);
        FntPrint("Buttons: %04x\n", pad0.buttons);
        //FntPrint("LastInput: %04x\n", lastInput);
        FntPrint("Stick L XY: (%02x, %02x)\n", pad0.leftstick.x, pad0.leftstick.y);
        FntPrint("Stick R XY: (%02x, %02x)\n\n", pad0.rightstick.x, pad0.rightstick.y);

        FntPrint("TempRot: %d, %d\n", tempRot.vx, tempRot.vy);
        FntPrint("CamRot: %d, %d\n", camRot.vx, camRot.vy);
        FntPrint("TempPos: %d, %d, %d\n", tempPos.vx, tempPos.vy, tempPos.vz);
        FntPrint("CamPos: %d, %d, %d\n", camPos.vx, camPos.vy, camPos.vz);

        // Update cube
        drawFloor(cdb->ot, &cdb->floor, &floorRenderTransform);
        add_cube(cdb->ot, cdb->s, &cubeRenderTransform);
        

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
