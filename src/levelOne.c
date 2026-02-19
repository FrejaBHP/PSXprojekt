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
    StaticWorldGeometry* startFloor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX, 0, -160, 
        windingIndices,
        64, 0, 64,
        6, 1, 4,
        DRP_Low,
        &grass_tim, 0, 192, 63, 63
    );


    // Grassy hill
    const long grassyHillX = 4;
    const long grassyHillZ = 3;

    StaticWorldGeometry* grassyHillSide = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 384, 0, -96, 
        reverseWindingIndices,
        0, 64, 64,
        1, 1, grassyHillZ, 
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    StaticWorldGeometry* grassyHillFloorLeft = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 384, -64, -96, 
        windingIndices,
        64, 0, 64,
        grassyHillX / 2, 1, grassyHillZ,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    StaticWorldGeometry* grassyHillFloorRight = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 512, -64, -160, 
        windingIndices,
        64, 0, 64,
        grassyHillX / 2, 1, grassyHillZ,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    CollisionBox* grassyHillLeftCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        startX + 384, 0, -96,
        grassyHillX * 32, 64, grassyHillZ * 64
    );

    CollisionBox* grassyHillRightCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        startX + 512, 0, -160,
        grassyHillX * 32, 64, 32 + grassyHillZ * 64
    );


    // Steps
    StaticWorldGeometry* grassyHillStep1Floor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 384, -16, -160, 
        windingIndices,
        64, 0, 64,
        1, 1, 1,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    StaticWorldGeometry* grassyHillStepFront1 = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 384, 0, -160, 
        reverseWindingIndices,
        0, 16, 64,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    CollisionBox* grassyHillStep1Collision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        startX + 384, 0, -160,
        64, 16, 64
    );

    StaticWorldGeometry* grassyHillStep2Floor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 384 + 64, -32, -160, 
        windingIndices,
        64, 0, 64,
        1, 1, 1,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    StaticWorldGeometry* grassyHillStepFront2 = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 384 + 64, -16, -160, 
        reverseWindingIndices,
        0, 16, 64,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    CollisionBox* grassyHillStep2Collision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        startX + 384 + 64, 0, -160,
        64, 32, 64
    );

    StaticWorldGeometry* grassyHillStepFront3 = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 384 + 128, -32, -160, 
        reverseWindingIndices,
        0, 32, 64,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    StaticWorldGeometry* grassyHillStepWall1 = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 384, -16, -96, 
        windingIndices,
        64, 48, 0,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    StaticWorldGeometry* grassyHillStepWall2 = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 384 + 64, -32, -96, 
        windingIndices,
        64, 32, 0,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );


    // On top of the starting grassy hill
    StaticWorldGeometry* grassyHillStepToBridgeTop = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        startX + 512, -96, 32, 
        windingIndices,
        64, 0, 64,
        2, 1, 1,
        DRP_Neutral,
        &grass_tim, &grass_UVR
    );

    StaticWorldGeometry* grassyHillStepToBridgeFront = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 512, -64, 32, 
        windingIndices,
        64, 32, 0,
        2, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    StaticWorldGeometry* grassyHillStepToBridgeSide = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 512, -64, 32, 
        reverseWindingIndices,
        0, 32, 64,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    CollisionBox* grassyHillStepToBridgeCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        startX + 512, -64, 32,
        128, 32, 64
    );

    StaticWorldGeometry* grassyHillBridgeHalfwall1 = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 384, -64, 96, 
        windingIndices,
        64, 64, 0,
        grassyHillX / 2, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    StaticWorldGeometry* grassyHillBridgeHalfwall2 = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 512, -96, 96, 
        windingIndices,
        64, 32, 0,
        grassyHillX / 2, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );


    // Bridge section
    StaticWorldGeometry* startBridgeFrontWall = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX, 0, 96, 
        windingIndices,
        128, 128, 0,
        3, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    StaticWorldGeometry* startBridgeFrontFloor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX, -128, 96, 
        windingIndices,
        64, 0, 64,
        10, 1, 3,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    CollisionBox* startBridgeFrontCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        startX, 0, 96,
        640, 128, 192
    );

    StaticWorldGeometry* startBridgeBackFloor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX, -128, 352, 
        windingIndices,
        64, 0, 64,
        10, 1, 3,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    CollisionBox* startBridgeBackCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        startX, -64, 352,
        640, 64, 256
    );

    StaticWorldGeometry* startBridgeStreamFloor = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        startX, -80, 288, 
        windingIndices,
        64, 0, 64,
        10, 1, 1,
        DRP_Neutral,
        &dirt_tim, &dirt_UVR
    );

    CollisionBox* startBridgeStreamFloorCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        startX, -16, 288,
        640, 64, 64
    );

    StaticWorldGeometry* startBridgeStreamFrontWall = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX, -80, 288, 
        reverseWindingIndices,
        64, 48, 0,
        10, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    StaticWorldGeometry* startBridgeStreamBackWall = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX, -80, 352, 
        windingIndices,
        64, 48, 0,
        10, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    StaticWorldPolyBox* testPolyBox = CreateStaticWorldPolyBox(
        overworldData.PolyBoxes, overworldData.CollisionBoxes,
        0, 0, 0, 
        64, 48, 64,
        DRP_Neutral,
        0b010011,
        (PolyData[]) {
            SetupPolyDataUVRect(&grassydirt_tim, &grassydirt_UVR),
            SetupPolyDataUVRect(&grassydirt_tim, &grassydirt_UVR),
            SetupPolyDataUVRect(&grass_tim, &grass_UVR)
        }
    );
}
