#include "levelOne.h"

LevelData overworldData;

const short startX = 64;
const short startY = 0;
const short bridgeStartZ = 96;

SVECTOR bridgeStepEndPos;

void CreateStartSection();
void CreateGrassyHill();
void CreateBridgeArea();
void CreateWaterArea();

void InitialiseLevelOne() {
    overworldData.TiledPolys = CreateGenericPtrListWithSize(128);
    overworldData.PolyBoxes = CreateGenericPtrListWithSize(32);
    overworldData.CollisionBoxes = CreateGenericPtrListWithSize(64);
    overworldData.Collectibles = CreateGenericLinkedList();

    CreateStartSection();
    CreateGrassyHill();
    CreateBridgeArea();
    CreateWaterArea();
}

void SetActiveLevelOneOverworld() {
    CurrentLevelData = &overworldData;
}

void CreateStartSection() {
    // floor
    StaticWorldGeometry* startFloor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX, startY, -160, 
        windingIndices,
        64, 0, 64,
        6, 1, 4,
        DRP_Low,
        &grass_tim, 0, 192, 63, 63
    );

    CollisionBox* startCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        startX, startY + 64, -160,
        6 * 64, 64, 4 * 64
    );
}

void CreateGrassyHill() {
    // Grassy hill
    const long grassyHillX = 4;
    const long grassyHillZ = 3;

    StaticWorldGeometry* grassyHillSide = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 384, startY, -96, 
        reverseWindingIndices,
        0, 64, 64,
        1, 1, grassyHillZ, 
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    StaticWorldGeometry* grassyHillFloorLeft = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 384, startY - 64, -96, 
        windingIndices,
        64, 0, 64,
        grassyHillX / 2, 1, grassyHillZ,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    StaticWorldGeometry* grassyHillFloorRight = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 512, startY - 64, -160, 
        windingIndices,
        64, 0, 64,
        grassyHillX / 2, 1, grassyHillZ,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    CollisionBox* grassyHillLeftCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        startX + 384, startY, -96,
        grassyHillX * 32, 64, grassyHillZ * 64
    );

    CollisionBox* grassyHillRightCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        startX + 512, startY, -160,
        grassyHillX * 32, 64, 32 + grassyHillZ * 64
    );


    // Steps
    StaticWorldPolyBox* grassyHillStep1 = CreateStaticWorldPolyBox(
        overworldData.PolyBoxes, overworldData.CollisionBoxes,
        startX + 384, startY, -160, 
        64, 16, 64,
        0, 16, 0,
        DRP_Neutral,
        0b011000,
        (PolyData[]) {
            SetupPolyDataUVRect(&grassydirt_tim, &grassydirt_UVR),
            SetupPolyDataUVRect(&grass_tim, &grass_UVR)
        }
    );

    StaticWorldPolyBox* grassyHillStep2 = CreateStaticWorldPolyBox(
        overworldData.PolyBoxes, overworldData.CollisionBoxes,
        startX + 448, startY - 16, -160, 
        64, 16, 64,
        0, 16, 0,
        DRP_Neutral,
        0b011000,
        (PolyData[]) {
            SetupPolyDataUVRect(&grassydirt_tim, &grassydirt_UVR),
            SetupPolyDataUVRect(&grass_tim, &grass_UVR)
        }
    );

    StaticWorldGeometry* grassyHillStepFront3 = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 384 + 128, startY - 32, -160, 
        reverseWindingIndices,
        0, 32, 64,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    StaticWorldGeometry* grassyHillStepWall1 = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 384, startY - 16, -96, 
        windingIndices,
        64, 48, 0,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    StaticWorldGeometry* grassyHillStepWall2 = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 384 + 64, startY - 32, -96, 
        windingIndices,
        64, 32, 0,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );


    // On top of the starting grassy hill
    StaticWorldGeometry* grassyHillStepToBridgeTop = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        startX + 512, startY - 96, 32, 
        windingIndices,
        64, 0, 64,
        2, 1, 1,
        DRP_Neutral,
        &grass_tim, &grass_UVR
    );

    StaticWorldGeometry* grassyHillStepToBridgeFront = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 512, startY - 64, 32, 
        windingIndices,
        64, 32, 0,
        2, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    StaticWorldGeometry* grassyHillStepToBridgeSide = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 512, startY - 64, 32, 
        reverseWindingIndices,
        0, 32, 64,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    CollisionBox* grassyHillStepToBridgeCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        startX + 512, startY - 64, 32,
        128, 32, 64
    );

    StaticWorldGeometry* grassyHillBridgeHalfwall1 = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 384, startY - 64, 96, 
        windingIndices,
        64, 64, 0,
        grassyHillX / 2, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    StaticWorldGeometry* grassyHillBridgeHalfwall2 = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 512, startY - 96, 96, 
        windingIndices,
        64, 32, 0,
        grassyHillX / 2, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );
}

void CreateBridgeArea() {
    const short bridgeHalfX = 10;
    const short bridgeHalfZ = 4;
    const short bridgeStreamZ = 2;

    const short bridgeHalfStreamZ = bridgeHalfZ + bridgeStreamZ;
    const short bridgeHalfStreamHalfZ = bridgeHalfZ + bridgeStreamZ + bridgeHalfZ;

    StaticWorldGeometry* startBridgeFrontWall = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX, startY, bridgeStartZ, 
        windingIndices,
        128, 128, 0,
        3, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    StaticWorldGeometry* startBridgeFrontFloor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX, startY - 128, bridgeStartZ, 
        windingIndices,
        64, 0, 64,
        bridgeHalfX, 1, bridgeHalfZ,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    CollisionBox* startBridgeFrontCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        startX, startY, bridgeStartZ,
        bridgeHalfX * 64, 128, bridgeHalfZ * 64
    );

    // Stream
    StaticWorldGeometry* startBridgeStreamFloor = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        startX, startY - 80, bridgeStartZ + (bridgeHalfZ * 64), 
        windingIndices,
        64, 0, 64,
        bridgeHalfX, 1, bridgeStreamZ,
        DRP_Neutral,
        &dirt_tim, &dirt_UVR
    );

    CollisionBox* startBridgeStreamFloorCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        startX, startY - 16, bridgeStartZ + (bridgeHalfZ * 64),
        640, 64, bridgeStreamZ * 64
    );

    StaticWorldGeometry* startBridgeStreamFrontWall = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX, startY - 80, bridgeStartZ + (bridgeHalfZ * 64), 
        reverseWindingIndices,
        64, 48, 0,
        bridgeHalfX, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    StaticWorldGeometry* startBridgeStreamBackWall = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX, startY - 80, bridgeStartZ + (bridgeHalfStreamZ * 64), 
        windingIndices,
        64, 48, 0,
        bridgeHalfX, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    // Bridge
    StaticWorldPolyBox* bridge = CreateStaticWorldPolyBox(
        overworldData.PolyBoxes, overworldData.CollisionBoxes,
        startX + 160, startY - 128, bridgeStartZ + (bridgeHalfZ * 64), 
        96, 24, 128,
        0, 0, 0,
        DRP_High,
        0b111111,
        (PolyData[]) {
            SetupPolyDataUVRect(&woodPanel_tim, &woodPanel_UVR),
            SetupPolyDataUVRect(&woodPanel_tim, &woodPanel_UVR),
            SetupPolyDataUVRect(&woodPanel_tim, &woodPanel_UVR),
            SetupPolyDataUVRect(&woodPanel_tim, &woodPanel_UVR),
            SetupPolyDataUVRect(&woodPanel_tim, &woodPanel_UVR),
            SetupPolyDataUVRect(&woodPanel_tim, &woodPanel_UVR)
        }
    );

    // Back half
    StaticWorldGeometry* startBridgeBackFloor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX, startY - 128, bridgeStartZ + (bridgeHalfStreamZ * 64), 
        windingIndices,
        64, 0, 64,
        bridgeHalfX, 1, bridgeHalfZ,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    StaticWorldGeometry* startBridgeBackback = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX, startY, bridgeStartZ + (bridgeHalfStreamHalfZ * 64), 
        reverseWindingIndices,
        128, 128, 0,
        bridgeHalfX / 2, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    CollisionBox* startBridgeBackCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        startX, startY, bridgeStartZ + (bridgeHalfStreamZ * 64),
        bridgeHalfX * 64, 128, bridgeHalfZ * 64
    );

    
    // Bridge "staircase", descending order
    short i = 0;

    StaticWorldPolyBox* bridgeStairStepTop = CreateStaticWorldPolyBox(
        overworldData.PolyBoxes, overworldData.CollisionBoxes,
        startX - 64, startY - 112, bridgeStartZ + ((bridgeHalfStreamZ + i++) * 64), 
        64, 16, 64,
        0, 16, 0,
        DRP_Neutral,
        0b010100,
        (PolyData[]) {
            SetupPolyDataUVRect(&grassydirt_tim, &grassydirt_UVR),
            SetupPolyDataUVRect(&grass_tim, &grass_UVR)
        }
    );

    StaticWorldPolyBox* bridgeStairStep1 = CreateStaticWorldPolyBox(
        overworldData.PolyBoxes, overworldData.CollisionBoxes,
        startX - 64, startY - 96, bridgeStartZ + ((bridgeHalfStreamZ + i++) * 64), 
        64, 16, 64,
        0, 16, 0,
        DRP_Neutral,
        0b010100,
        (PolyData[]) {
            SetupPolyDataUVRect(&grassydirt_tim, &grassydirt_UVR),
            SetupPolyDataUVRect(&grass_tim, &grass_UVR)
        }
    );

    StaticWorldPolyBox* bridgeStairStep2 = CreateStaticWorldPolyBox(
        overworldData.PolyBoxes, overworldData.CollisionBoxes,
        startX - 64, startY - 80, bridgeStartZ + ((bridgeHalfStreamZ + i++) * 64), 
        64, 16, 64,
        0, 16, 0,
        DRP_Neutral,
        0b010100,
        (PolyData[]) {
            SetupPolyDataUVRect(&grassydirt_tim, &grassydirt_UVR),
            SetupPolyDataUVRect(&grass_tim, &grass_UVR)
        }
    );

    StaticWorldPolyBox* bridgeStairStep3 = CreateStaticWorldPolyBox(
        overworldData.PolyBoxes, overworldData.CollisionBoxes,
        startX - 64, startY - 64, bridgeStartZ + ((bridgeHalfStreamZ + i++) * 64), 
        64, 16, 64,
        0, 16, 0,
        DRP_Neutral,
        0b010100,
        (PolyData[]) {
            SetupPolyDataUVRect(&grassydirt_tim, &grassydirt_UVR),
            SetupPolyDataUVRect(&grass_tim, &grass_UVR)
        }
    );

    StaticWorldPolyBox* bridgeStairStep4 = CreateStaticWorldPolyBox(
        overworldData.PolyBoxes, overworldData.CollisionBoxes,
        startX - 64, startY - 48, bridgeStartZ + ((bridgeHalfStreamZ + i++) * 64), 
        64, 16, 64,
        0, 16, 0,
        DRP_Neutral,
        0b010100,
        (PolyData[]) {
            SetupPolyDataUVRect(&grassydirt_tim, &grassydirt_UVR),
            SetupPolyDataUVRect(&grass_tim, &grass_UVR)
        }
    );

    StaticWorldPolyBox* bridgeStairStep5 = CreateStaticWorldPolyBox(
        overworldData.PolyBoxes, overworldData.CollisionBoxes,
        startX - 64, startY - 32, bridgeStartZ + ((bridgeHalfStreamZ + i++) * 64), 
        64, 16, 64,
        0, 16, 0,
        DRP_Neutral,
        0b010100,
        (PolyData[]) {
            SetupPolyDataUVRect(&grassydirt_tim, &grassydirt_UVR),
            SetupPolyDataUVRect(&grass_tim, &grass_UVR)
        }
    );

    setVector(&bridgeStepEndPos, startX - 64, startY - 32, bridgeStartZ + ((bridgeHalfStreamZ + i) * 64));
}

void CreateWaterArea() {
    const short wLandX = 3;
    const short wLandZ = 3;

    const short wLandSeg1OffsetX = wLandX * 64;
    const short wLandSeg1X = 2;
    const short wLandSeg1Z = 2;

    const short wLandSeg2OffsetX = wLandSeg1OffsetX + (wLandSeg1X * 64);
    const short wLandSeg2X = 1;
    const short wLandSeg2Z = 3;

    const short wLandSeg3OffsetX = wLandSeg2OffsetX + (wLandSeg2X * 64);
    const short wLandSeg3X = 1;
    const short wLandSeg3Z = 4;

    const short wLandSeg4OffsetX = wLandSeg3OffsetX + (wLandSeg3X * 64);
    const short wLandSeg4X = 3;
    const short wLandSeg4Z = 3;

    const short wLandSeg5OffsetX = wLandSeg4OffsetX + (wLandSeg4X * 64);
    const short wLandSeg5X = 1;
    const short wLandSeg5Z = 2;

    const short combinedSegFront = wLandX + wLandSeg1X + wLandSeg2X + wLandSeg3X;
    const short combinedSegBack = wLandSeg3X + wLandSeg4X + wLandSeg5X;

    // Start
    StaticWorldGeometry* waterLandStartFloor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        bridgeStepEndPos.vx, bridgeStepEndPos.vy, bridgeStepEndPos.vz, 
        windingIndices,
        64, 0, 64,
        wLandX, 1, wLandZ,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    StaticWorldGeometry* waterLandCombinedFront = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        bridgeStepEndPos.vx, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz, 
        windingIndices,
        64, 64, 0,
        combinedSegFront, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    StaticWorldGeometry* waterLandStartBack = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        bridgeStepEndPos.vx, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz + 192, 
        reverseWindingIndices,
        64, 64, 0,
        wLandX, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    StaticWorldGeometry* waterLandStartLeft = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        bridgeStepEndPos.vx, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz, 
        reverseWindingIndices,
        0, 64, 64,
        1, 1, wLandZ,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    StaticWorldGeometry* waterLandStartRight = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        bridgeStepEndPos.vx + 192, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz + 128, 
        windingIndices,
        0, 64, 64,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    CollisionBox* waterLandStartCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        bridgeStepEndPos.vx, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz, 
        192, 64, 192
    );

    // Segment 1
    StaticWorldGeometry* waterLandSeg1Floor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        bridgeStepEndPos.vx + wLandSeg1OffsetX, bridgeStepEndPos.vy, bridgeStepEndPos.vz, 
        windingIndices,
        64, 0, 64,
        wLandSeg1X, 1, wLandSeg1Z,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    StaticWorldGeometry* waterLandSeg1Back = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        bridgeStepEndPos.vx + wLandSeg1OffsetX, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz + 128, 
        reverseWindingIndices,
        64, 64, 0,
        wLandSeg1X, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    CollisionBox* waterLandSeg1Collision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        bridgeStepEndPos.vx + wLandSeg1OffsetX, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz, 
        wLandSeg1X * 64, 64, wLandSeg1Z * 64
    );

    // Segment 2
    StaticWorldGeometry* waterLandSeg2Floor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        bridgeStepEndPos.vx + wLandSeg2OffsetX, bridgeStepEndPos.vy, bridgeStepEndPos.vz, 
        windingIndices,
        64, 0, 64,
        wLandSeg2X, 1, wLandSeg2Z,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    StaticWorldGeometry* waterLandSeg2Left = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        bridgeStepEndPos.vx + wLandSeg2OffsetX, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz + 128, 
        reverseWindingIndices,
        0, 64, 64,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    StaticWorldGeometry* waterLandSeg2Back = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        bridgeStepEndPos.vx + wLandSeg2OffsetX, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz + 192, 
        reverseWindingIndices,
        64, 64, 0,
        wLandSeg2X, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    CollisionBox* waterLandSeg2Collision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        bridgeStepEndPos.vx + wLandSeg2OffsetX, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz, 
        wLandSeg2X * 64, 64, wLandSeg2Z * 64
    );

    // Segment 3
    StaticWorldGeometry* waterLandSeg3Floor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        bridgeStepEndPos.vx + wLandSeg3OffsetX, bridgeStepEndPos.vy, bridgeStepEndPos.vz, 
        windingIndices,
        64, 0, 64,
        wLandSeg3X, 1, wLandSeg3Z,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    StaticWorldGeometry* waterLandSeg3Right = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        bridgeStepEndPos.vx + wLandSeg3OffsetX + (wLandSeg3X * 64), bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz, 
        windingIndices,
        0, 64, 64,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    StaticWorldGeometry* waterLandSeg3Left = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        bridgeStepEndPos.vx + wLandSeg3OffsetX, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz + 192, 
        reverseWindingIndices,
        0, 64, 64,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    StaticWorldGeometry* waterLandSegCombinedBack = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        bridgeStepEndPos.vx + wLandSeg3OffsetX, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz + 256, 
        reverseWindingIndices,
        64, 64, 0,
        combinedSegBack, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    CollisionBox* waterLandSeg3Collision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        bridgeStepEndPos.vx + wLandSeg3OffsetX, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz, 
        wLandSeg3X * 64, 64, wLandSeg3Z * 64
    );

    // Segment 4
    StaticWorldGeometry* waterLandSeg4Floor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        bridgeStepEndPos.vx + wLandSeg4OffsetX, bridgeStepEndPos.vy, bridgeStepEndPos.vz + 64, 
        windingIndices,
        64, 0, 64,
        wLandSeg4X, 1, wLandSeg4Z,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    StaticWorldGeometry* waterLandSeg4Front = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        bridgeStepEndPos.vx + wLandSeg4OffsetX, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz + 64, 
        windingIndices,
        64, 64, 0,
        wLandSeg4X, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    StaticWorldGeometry* waterLandSeg4Right = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        bridgeStepEndPos.vx + wLandSeg4OffsetX + (wLandSeg4X * 64), bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz + 64, 
        windingIndices,
        0, 64, 64,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    CollisionBox* waterLandSeg4Collision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        bridgeStepEndPos.vx + wLandSeg4OffsetX, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz + 64, 
        wLandSeg4X * 64, 64, wLandSeg4Z * 64
    );

    // Segment 5
    StaticWorldGeometry* waterLandSeg5Floor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        bridgeStepEndPos.vx + wLandSeg5OffsetX, bridgeStepEndPos.vy, bridgeStepEndPos.vz + 128, 
        windingIndices,
        64, 0, 64,
        wLandSeg5X, 1, wLandSeg5Z,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    StaticWorldGeometry* waterLandSeg5Front = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        bridgeStepEndPos.vx + wLandSeg5OffsetX, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz + 128, 
        windingIndices,
        64, 64, 0,
        wLandSeg5X, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    CollisionBox* waterLandSeg5Collision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        bridgeStepEndPos.vx + wLandSeg5OffsetX, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz + 128, 
        wLandSeg5X * 64, 64, wLandSeg5Z * 64
    );
}
