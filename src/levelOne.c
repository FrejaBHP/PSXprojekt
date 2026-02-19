#include "levelOne.h"

LevelData overworldData;

void InitialiseLevelOne() {
    overworldData.TiledPolys = CreateGenericPtrListWithSize(64);
    overworldData.PolyBoxes = CreateGenericPtrListWithSize(64);
    overworldData.CollisionBoxes = CreateGenericPtrListWithSize(64);
    overworldData.Collectibles = CreateGenericLinkedList();

    CreateStartSection();
}

void SetActiveLevelOneOverworld() {
    CurrentLevelData = &overworldData;
}

void CreateStartSection() {
    long startX = 64;

    // floor
    TiledTexturedPolyObject*  startFloor= CreateTiledTexturedPolyObjectFT4(
        startX, 0, -160, 
        0, 0, 0,
        windingIndices,
        64, 0, 64,
        6, 1, 4,
        DRP_Low,
        &grass_tim,
        0, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, startFloor);


    // Grassy hill
    const long grassyHillX = 4;
    const long grassyHillZ = 3;

    TiledTexturedPolyObject* grassyHillSide = CreateTiledTexturedPolyObjectFT4(
        startX + 384, 0, -96, 
        0, 0, 0,
        reverseWindingIndices,
        0, 64, 64,
        1, 1, grassyHillZ, 
        DRP_Neutral,
        &grassydirt_tim,
        64, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, grassyHillSide);

    TiledTexturedPolyObject* grassyHillFloorLeft = CreateTiledTexturedPolyObjectFT4(
        startX + 384, -64, -96, 
        0, 0, 0,
        windingIndices,
        64, 0, 64,
        grassyHillX / 2, 1, grassyHillZ,
        DRP_Neutral,
        &grass_tim,
        0, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, grassyHillFloorLeft);

    TiledTexturedPolyObject* grassyHillFloorRight = CreateTiledTexturedPolyObjectFT4(
        startX + 512, -64, -160, 
        0, 0, 0,
        windingIndices,
        64, 0, 64,
        grassyHillX / 2, 1, grassyHillZ,
        DRP_Neutral,
        &grass_tim,
        0, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, grassyHillFloorRight);

    CollisionBox* grassyHillLeftCollision = CreateCollisionBox(
        startX + 384, 0, -96,
        grassyHillX * 32, 64, grassyHillZ * 64
    );
    AddItemToGenericPtrList(&overworldData.CollisionBoxes, grassyHillLeftCollision);

    CollisionBox* grassyHillRightCollision = CreateCollisionBox(
        startX + 512, 0, -160,
        grassyHillX * 32, 64, 32 + grassyHillZ * 64
    );
    AddItemToGenericPtrList(&overworldData.CollisionBoxes, grassyHillRightCollision);


    // Steps
    TiledTexturedPolyObject* grassyHillStep1Floor = CreateTiledTexturedPolyObjectFT4(
        startX + 384, -16, -160, 
        0, 0, 0,
        windingIndices,
        64, 0, 64,
        1, 1, 1,
        DRP_Neutral,
        &grass_tim,
        0, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, grassyHillStep1Floor);

    TiledTexturedPolyObject* grassyHillStepFront1 = CreateTiledTexturedPolyObjectFT4(
        startX + 384, 0, -160, 
        0, 0, 0,
        reverseWindingIndices,
        0, 16, 64,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim,
        64, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, grassyHillStepFront1);

    CollisionBox* grassyHillStep1Collision = CreateCollisionBox(
        startX + 384, 0, -160,
        64, 16, 64
    );
    AddItemToGenericPtrList(&overworldData.CollisionBoxes, grassyHillStep1Collision);

    TiledTexturedPolyObject* grassyHillStep2Floor = CreateTiledTexturedPolyObjectFT4(
        startX + 384 + 64, -32, -160, 
        0, 0, 0,
        windingIndices,
        64, 0, 64,
        1, 1, 1,
        DRP_Neutral,
        &grass_tim,
        0, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, grassyHillStep2Floor);

    TiledTexturedPolyObject* grassyHillStepFront2 = CreateTiledTexturedPolyObjectFT4(
        startX + 384 + 64, -16, -160, 
        0, 0, 0,
        reverseWindingIndices,
        0, 16, 64,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim,
        64, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, grassyHillStepFront2);

    CollisionBox* grassyHillStep2Collision = CreateCollisionBox(
        startX + 384 + 64, 0, -160,
        64, 32, 64
    );
    AddItemToGenericPtrList(&overworldData.CollisionBoxes, grassyHillStep2Collision);

    TiledTexturedPolyObject* grassyHillStepFront3 = CreateTiledTexturedPolyObjectFT4(
        startX + 384 + 128, -32, -160, 
        0, 0, 0,
        reverseWindingIndices,
        0, 32, 64,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim,
        64, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, grassyHillStepFront3);

    TiledTexturedPolyObject* grassyHillStepWall1 = CreateTiledTexturedPolyObjectFT4(
        startX + 384, -16, -96, 
        0, 0, 0,
        windingIndices,
        64, 48, 0,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim,
        64, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, grassyHillStepWall1);

    TiledTexturedPolyObject* grassyHillStepWall2 = CreateTiledTexturedPolyObjectFT4(
        startX + 384 + 64, -32, -96, 
        0, 0, 0,
        windingIndices,
        64, 32, 0,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim,
        64, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, grassyHillStepWall2);

    
    // On top of the starting grassy hill
    TiledTexturedPolyObject* grassyHillStepToBridgeTop = CreateTiledTexturedPolyObjectFT4UVRect(
        startX + 512, -96, 32, 
        0, 0, 0,
        windingIndices,
        64, 0, 64,
        2, 1, 1,
        DRP_Neutral,
        &grass_tim,
        &grass_UVR
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, grassyHillStepToBridgeTop);

    TiledTexturedPolyObject* grassyHillStepToBridgeFront = CreateTiledTexturedPolyObjectFT4(
        startX + 512, -64, 32, 
        0, 0, 0,
        windingIndices,
        64, 32, 0,
        2, 1, 1,
        DRP_Neutral,
        &grassydirt_tim,
        64, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, grassyHillStepToBridgeFront);

    TiledTexturedPolyObject* grassyHillStepToBridgeSide = CreateTiledTexturedPolyObjectFT4(
        startX + 512, -64, 32, 
        0, 0, 0,
        reverseWindingIndices,
        0, 32, 64,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim,
        64, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, grassyHillStepToBridgeSide);

    CollisionBox* grassyHillStepToBridgeCollision = CreateCollisionBox(
        startX + 512, -64, 32,
        128, 32, 64
    );
    AddItemToGenericPtrList(&overworldData.CollisionBoxes, grassyHillStepToBridgeCollision);

    TiledTexturedPolyObject* grassyHillBridgeHalfwall1 = CreateTiledTexturedPolyObjectFT4(
        startX + 384, -64, 96, 
        0, 0, 0,
        windingIndices,
        64, 64, 0,
        grassyHillX / 2, 1, 1,
        DRP_Neutral,
        &grassydirt_tim,
        64, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, grassyHillBridgeHalfwall1);

    TiledTexturedPolyObject* grassyHillBridgeHalfwall2 = CreateTiledTexturedPolyObjectFT4(
        startX + 512, -96, 96, 
        0, 0, 0,
        windingIndices,
        64, 32, 0,
        grassyHillX / 2, 1, 1,
        DRP_Neutral,
        &grassydirt_tim,
        64, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, grassyHillBridgeHalfwall2);


    // Bridge section
    TiledTexturedPolyObject* startBridgeFrontWall = CreateTiledTexturedPolyObjectFT4(
        startX, 0, 96, 
        0, 0, 0,
        windingIndices,
        128, 128, 0,
        3, 1, 1,
        DRP_Neutral,
        &grassydirt_tim,
        64, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, startBridgeFrontWall);

    TiledTexturedPolyObject* startBridgeFrontFloor = CreateTiledTexturedPolyObjectFT4(
        startX, -128, 96, 
        0, 0, 0,
        windingIndices,
        64, 0, 64,
        10, 1, 3,
        DRP_Neutral,
        &grass_tim,
        0, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, startBridgeFrontFloor);

    CollisionBox* startBridgeFrontCollision = CreateCollisionBox(
        startX, 0, 96,
        640, 128, 192
    );
    AddItemToGenericPtrList(&overworldData.CollisionBoxes, startBridgeFrontCollision);

    TiledTexturedPolyObject* startBridgeBackFloor = CreateTiledTexturedPolyObjectFT4(
        startX, -128, 352, 
        0, 0, 0,
        windingIndices,
        64, 0, 64,
        10, 1, 3,
        DRP_Neutral,
        &grass_tim,
        0, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, startBridgeBackFloor);

    CollisionBox* startBridgeBackCollision = CreateCollisionBox(
        startX, -64, 352,
        640, 64, 256
    );
    AddItemToGenericPtrList(&overworldData.CollisionBoxes, startBridgeBackCollision);

    TiledTexturedPolyObject* startBridgeStreamFloor = CreateTiledTexturedPolyObjectFT4UVRect(
        startX, -80, 288, 
        0, 0, 0,
        windingIndices,
        64, 0, 64,
        10, 1, 1,
        DRP_Neutral,
        &dirt_tim,
        &dirt_UVR
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, startBridgeStreamFloor);

    CollisionBox* startBridgeStreamFloorCollision = CreateCollisionBox(
        startX, -16, 288,
        640, 64, 64
    );
    AddItemToGenericPtrList(&overworldData.CollisionBoxes, startBridgeStreamFloorCollision);

    TiledTexturedPolyObject* startBridgeStreamFrontWall = CreateTiledTexturedPolyObjectFT4(
        startX, -80, 288, 
        0, 0, 0,
        reverseWindingIndices,
        64, 48, 0,
        10, 1, 1,
        DRP_Neutral,
        &grassydirt_tim,
        64, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, startBridgeStreamFrontWall);

    TiledTexturedPolyObject* startBridgeStreamBackWall = CreateTiledTexturedPolyObjectFT4(
        startX, -80, 352, 
        0, 0, 0,
        windingIndices,
        64, 48, 0,
        10, 1, 1,
        DRP_Neutral,
        &grassydirt_tim,
        64, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, startBridgeStreamBackWall);
}
