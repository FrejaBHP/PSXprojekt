#include <stdlib.h>
#include <stdbool.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
#include <libapi.h>
#include <libspu.h>
#include <inline_n.h>
#include <gtemac.h>

#include "clist.h"
#include "graphics.h"
#include "objects.h"
#include "physics.h"
#include "geometry.h"
#include "player.h"
#include "level.h"

#include "levelOne.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))
#define setPosVToGrid(v, _x, _y, _z) \
	(v)->vx = _x >> 12, (v)->vy = _y >> 12, (v)->vz = _z >> 12

#define CAMERADISTANCE 144 // 160

#define ANALOGUE_MID 127
#define ANALOGUE_DEADZONE 24
#define ANALOGUE_MINPOS ANALOGUE_MID + ANALOGUE_DEADZONE
#define ANALOGUE_MINNEG ANALOGUE_MID - ANALOGUE_DEADZONE

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

//LinkedList* collectibleList;

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

    /*
    MATRIX invPlayerRotationMatrix;
    InvertMatrix(&player->poly.obj.transform, &invPlayerRotationMatrix);
    MulMatrix(&cameraRotationMatrix, &invPlayerRotationMatrix);
    */

    TransMatrix(&pivotMatrix, &cameraOrbitPos);

    CompMatrix(&cameraRotationMatrix, &pivotMatrix, &player->cameraPtr->transform);

    gte_SetRotMatrix(&player->cameraPtr->transform);
    gte_SetTransMatrix(&player->cameraPtr->transform);
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

void resetCube(SVECTOR* rot, VECTOR* trans) {
    //setVector(rot, 0, 0, 0);
    //setVector(trans, 0, (-CUBEHALF - 32) * ONE, DISTTHING * ONE);
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

    AppendItemToLinkedList(CurrentLevelData->Collectibles, coin);
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

    //collectibleList = CreateGenericLinkedList();
    coinPolyData = SetupPolyData(&goldCoin_tim, 0, 128, 32, 32);

    InitialiseLevelOne();
    SetActiveLevelOneOverworld();

    CreatePlayer(col);
    cameraRotationMatrix = identity;
    //cameraRotationMatrix.t[1] = (PLAYERHEIGHT * 2);
    cameraRotationMatrix.t[2] = CAMERADISTANCE;

    /*
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
        &grassydirt_tim,
        64, 192, 63, 63
    );

    TiledTexturedPolyObject* tiledWallSide2 = CreateTiledTexturedPolyObjectFT4(
        startX + 384, 0, -160, 
        0, 0, 0,
        windingIndices,
        64, 64, 0,
        2, 1, 1,
        DRP_Neutral,
        &grassydirt_tim,
        64, 192, 63, 63
    );

    TiledTexturedPolyObject* tiledWallSide = CreateTiledTexturedPolyObjectFT4(
        startX + 384, 0, 96, 
        0, 2048, 0,
        windingIndices,
        0, 64, 64,
        1, 1, 4, 
        DRP_Neutral,
        &grassydirt_tim,
        64, 192, 63, 63
    );

    TiledTexturedPolyObject* slateFloor = CreateTiledTexturedPolyObjectFT4(
        startX, 0, -160, 
        0, 0, 0,
        windingIndices,
        //128, 0, 128,
        64, 0, 64,
        //3, 1, 2, 
        6, 1, 4,
        DRP_Low,
        //&dlv_slate_tim,
        //0, 127, 128, 128
        &grass_tim,
        0, 192, 63, 63
    );

    TiledTexturedPolyObject* slateFloor1 = CreateTiledTexturedPolyObjectFT4(
        startX, -128, 96, 
        0, 0, 0,
        windingIndices,
        //128, 0, 128,
        64, 0, 64,
        //3, 1, 1, 
        6, 1, 2,
        DRP_Low,
        //&dlv_slate_tim,
        //0, 128, 127, 127
        &grass_tim,
        0, 192, 63, 63
    );

    TiledTexturedPolyObject* grassyHillFloor = CreateTiledTexturedPolyObjectFT4(
        startX + 384, -64, -160, 
        0, 0, 0,
        windingIndices,
        64, 0, 64,
        2, 1, 4,
        DRP_Low,
        &grass_tim,
        0, 192, 63, 63
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
    activeTiledTexPolygons[1] = tiledWallSide2;
    activeTiledTexPolygons[2] = tiledWallSide;
    activeTiledTexPolygons[3] = slateFloor;
    activeTiledTexPolygons[4] = slateFloor1;
    activeTiledTexPolygons[5] = grassyHillFloor;


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

    StaticCollisionPolyBox* topPlatform = CreateCollisionPolyBox(
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

    StaticCollisionPolyBox* middlePlatform = CreateCollisionPolyBox(
        startX + 240, -72, -128,
        0, 0, 0,
        platformVertices,
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64),
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64),
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64),
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64),
        SetupPolyData(&dlv_metalpanel_tim, 0, 0, 128, 128),
        SetupPolyData(&dlv_slate_tim, 0, 128, 64, 64)
    );

    StaticCollisionPolyBox* bottomPlatform = CreateCollisionPolyBox(
        startX + 240, -32, -192,
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
        128, 64, 256
    );

    CollisionBox* nostalgia = CreateCollisionBox(
        -32, -12, 224,
        64, 24, 64
    );
    */


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

            /*
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
            */

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
                player->poly.obj.rotation.vy -= 24;
                //cameraRotation.vy += 16;
            }

            if (pad0.buttons & PADR1) {
                //tiledWall->polyObj.obj.rotation.vy += 32;
                player->poly.obj.rotation.vy += 24;
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
        CheckCollectiblePickup(CurrentLevelData->Collectibles);

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
        
        /*
        if (AutoRotate) {
            cube->obj.rotation.vy += 16;
            cube->obj.rotation.vz += 16;
        }
        */


        UpdatePolyObject(&player->poly);

        /*
        for (size_t i = 0; i < ACTIVEPOLYGONCOUNT; i++) {
            UpdatePolyObject(activePolygons[i]);
        }
        for (size_t i = 0; i < ACTIVETEXPOLYGONCOUNT; i++) {
            UpdatePolyObject(&activeTexPolygons[i]->polyObj);
        }
        */

        for (size_t i = 0; i < CurrentLevelData->TiledPolys->count; i++) {
            TiledTexturedPolyObject* ttpobj = CurrentLevelData->TiledPolys->array[i];
            UpdatePolyObject(&ttpobj->polyObj);
        }

        UpdatePlayerCamera(&cameraFixedRot);

        // Add polys to OT

        CameraTransformMatrix(player->cameraPtr, &player->poly.obj.transform);
        AddPolyF(&player->poly);

        /*
        for (size_t i = 0; i < ACTIVEPOLYGONCOUNT; i++) {
            CameraTransformMatrix(player->cameraPtr, &activePolygons[i]->obj.transform);
            AddPolyF(activePolygons[i]);
        }
        
        for (size_t i = 0; i < ACTIVETEXPOLYGONCOUNT; i++) {
            CameraTransformMatrix(player->cameraPtr, &activeTexPolygons[i]->polyObj.obj.transform);
            AddPolyFT(activeTexPolygons[i]);
        }
        */

        for (size_t i = 0; i < CurrentLevelData->TiledPolys->count; i++) {
            //CameraTransformMatrix(player->cameraPtr, &activeTiledTexPolygons[i]->polyObj.obj.transform);
            //AddTiledPolyFT(activeTiledTexPolygons[i]);
            TiledTexturedPolyObject* ttpobj = CurrentLevelData->TiledPolys->array[i];
            CameraTransformMatrix(player->cameraPtr, &ttpobj->polyObj.obj.transform);
            AddTiledPolyFT(ttpobj);
        }

        //CameraTransformMatrix(player->cameraPtr, &testPoly->obj.transform);
        //AddMultiPoly(testPoly, cdb->ot);

        //CameraTransformMatrix(player->cameraPtr, &testPolyFloor->obj.transform);
        //AddMultiPoly(testPolyFloor, cdb->ot);

        for (size_t i = 0; i < CurrentLevelData->PolyBoxes->count; i++) {
            StaticCollisionPolyBox* scpolybox = CurrentLevelData->PolyBoxes->array[i];
            CameraTransformMatrix(player->cameraPtr, &scpolybox->transform);
            AddStaticPolyBox(scpolybox);
        }

        IterateAddCollectibles(CurrentLevelData->Collectibles);

        //FntPrint("PT: %04d, %04d, %04d\n", player->poly.obj.transform.t[0], player->poly.obj.transform.t[1], player->poly.obj.transform.t[2]);
        //FntPrint("PV: %06d, %06d, %06d\n", player->poly.obj.velocity.vx, player->poly.obj.velocity.vy, player->poly.obj.velocity.vz);

        //FntPrint("%x\n", heapStart);

        //FntPrint("Coins\nCollected: %d\nRemaining: %d\n", collectedCoins, GetLinkedListLength(collectibleList));
        
        DrawFrame();
    }

    return 0;
}
