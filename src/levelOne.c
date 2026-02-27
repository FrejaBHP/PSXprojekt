#include "levelOne.h"

LevelData overworldData;

const short startX = 64;
const short startY = 0;
const short startZ = -160;
const short bridgeStartZ = 96;


SVECTOR bridgeStepEndPos;
SVECTOR waterLandEndPos;
SVECTOR ascentEndPos;

SVECTOR waterfallStartCorner;


void CreateStartSection();
void CreateGrassyHill();
void CreateBridgeArea();
void CreateWaterArea();
void CreateAscent();
void CreatePlateau();
void CreateWaterfall();

void InitialiseLevelOne() {
    overworldData.TiledPolys = CreateGenericPtrListWithSize(128);
    overworldData.PolyBoxes = CreateGenericPtrListWithSize(32);
    overworldData.CollisionBoxes = CreateGenericPtrListWithSize(64);
    overworldData.Collectibles = CreateGenericLinkedList();

    CreateStartSection();
    CreateGrassyHill();
    CreateBridgeArea();
    CreateWaterArea();
    CreateAscent();
    CreatePlateau();
    CreateWaterfall();
}

void SetActiveLevelOneOverworld() {
    CurrentLevelData = &overworldData;
}

void CreateStartSection() {
    // floor
    StaticWorldGeometry* startFloor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX, startY, startZ, 
        windingIndices,
        64, 0, 64,
        6, 1, 4,
        DRP_Low,
        &grass_tim, 0, 192, 63, 63
    );

    CollisionBox* startCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        startX, startY + 64, startZ,
        6 * 64, 64, 4 * 64
    );
}

void CreateGrassyHill() {
    // Grassy hill
    const long grassyHillX = 4;
    const long grassyHillZ = 3;

    StaticWorldGeometry* grassyHillSide = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 384, startY, startZ + 64, 
        reverseWindingIndices,
        0, 64, 64,
        1, 1, grassyHillZ, 
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    StaticWorldGeometry* grassyHillFloorLeft = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 384, startY - 64, startZ + 64, 
        windingIndices,
        64, 0, 64,
        grassyHillX / 2, 1, grassyHillZ,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    StaticWorldGeometry* grassyHillFloorRight = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 512, startY - 64, startZ, 
        windingIndices,
        64, 0, 64,
        grassyHillX / 2, 1, grassyHillZ,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    CollisionBox* grassyHillLeftCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        startX + 384, startY, startZ + 64,
        grassyHillX * 32, 64, grassyHillZ * 64
    );

    CollisionBox* grassyHillRightCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        startX + 512, startY, startZ,
        grassyHillX * 32, 64, 32 + grassyHillZ * 64
    );


    // Steps
    StaticWorldPolyBox* grassyHillStep1 = CreateStaticWorldPolyBox(
        overworldData.PolyBoxes, overworldData.CollisionBoxes,
        startX + 384, startY, startZ, 
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
        startX + 448, startY - 16, startZ, 
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
        startX + 384 + 128, startY - 32, startZ, 
        reverseWindingIndices,
        0, 32, 64,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    StaticWorldGeometry* grassyHillStepWall1 = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 384, startY - 16, startZ + 64, 
        windingIndices,
        64, 48, 0,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    StaticWorldGeometry* grassyHillStepWall2 = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 384 + 64, startY - 32, startZ + 64, 
        windingIndices,
        64, 32, 0,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );


    // On top of the starting grassy hill
    StaticWorldGeometry* grassyHillStepToBridgeTop = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        startX + 512, startY - 96, startZ + 192, 
        windingIndices,
        64, 0, 64,
        2, 1, 1,
        DRP_Neutral,
        &grass_tim, &grass_UVR
    );

    StaticWorldGeometry* grassyHillStepToBridgeFront = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 512, startY - 64, startZ + 192, 
        windingIndices,
        64, 32, 0,
        2, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    StaticWorldGeometry* grassyHillStepToBridgeSide = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 512, startY - 64, startZ + 192, 
        reverseWindingIndices,
        0, 32, 64,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    CollisionBox* grassyHillStepToBridgeCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        startX + 512, startY - 64, startZ + 192,
        128, 32, 64
    );

    StaticWorldGeometry* grassyHillBridgeHalfwall1 = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 384, startY - 64, startZ + 256, 
        windingIndices,
        64, 64, 0,
        grassyHillX / 2, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );

    StaticWorldGeometry* grassyHillBridgeHalfwall2 = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        startX + 512, startY - 96, startZ + 256, 
        windingIndices,
        64, 32, 0,
        grassyHillX / 2, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, 64, 192, 63, 63
    );
}

void CreateBridgeArea() {
    const short bridgeHalfX = 10;
    const short bridgeHalfZ = 3;
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


    StaticWorldGeometry* startBridgeWaterfallWall = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        startX + (64 * bridgeHalfX), startY - 224, bridgeStartZ, 
        reverseWindingIndices,
        0, 96, 128,
        1, 2, 4,
        DRP_Neutral,
        &dirt_tim, &dirt_UVR
    );

    waterfallStartCorner.vx = startX + (64 * bridgeHalfX);
    waterfallStartCorner.vy = startY - (224 + 96);
    waterfallStartCorner.vz = bridgeStartZ + (128 * 4);


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

    StaticWorldGeometry* startBridgeStreamRightWall = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        startX + (64 * bridgeHalfX), startY - 80, bridgeStartZ + (bridgeHalfZ * 64), 
        reverseWindingIndices,
        0, 48, 64,
        1, 1, bridgeStreamZ,
        DRP_Neutral,
        &dirt_tim, &dirt_UVR
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
    short stairOffset = 96;
    short stairWidth = 96;

    StaticWorldPolyBox* bridgeStairStepTop = CreateStaticWorldPolyBox(
        overworldData.PolyBoxes, overworldData.CollisionBoxes,
        startX - stairWidth, startY - (stairOffset - (16 * i)), bridgeStartZ + ((bridgeHalfStreamZ + i++) * 64), 
        stairWidth, 16, 64,
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
        startX - stairWidth, startY - (stairOffset - (16 * i)), bridgeStartZ + ((bridgeHalfStreamZ + i++) * 64), 
        stairWidth, 16, 64,
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
        startX - stairWidth, startY - (stairOffset - (16 * i)), bridgeStartZ + ((bridgeHalfStreamZ + i++) * 64), 
        stairWidth, 16, 64,
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
        startX - stairWidth + 8, startY - (stairOffset - (16 * i)), bridgeStartZ + ((bridgeHalfStreamZ + i++) * 64), 
        stairWidth, 16, 64,
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
        startX - stairWidth + 24, startY - (stairOffset - (16 * i)), bridgeStartZ + ((bridgeHalfStreamZ + i++) * 64), 
        stairWidth, 16, 64,
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
    const short wLandX = 4;
    const short wLandZ = 4;

    const short wLandSeg1OffsetX = wLandX * 64;
    const short wLandSeg1X = 3;
    const short wLandSeg1Z = 3;

    const short wLandSeg2OffsetX = wLandSeg1OffsetX + (wLandSeg1X * 64);
    const short wLandSeg2X = 2;
    const short wLandSeg2Z = 4;

    const short wLandSeg3OffsetX = wLandSeg2OffsetX + (wLandSeg2X * 64);
    const short wLandSeg3X = 2;
    const short wLandSeg3Z = 5;

    const short wLandSeg4OffsetX = wLandSeg3OffsetX + (wLandSeg3X * 64);
    const short wLandSeg4X = 3;
    const short wLandSeg4Z = 4;

    const short wLandSeg5OffsetX = wLandSeg4OffsetX + (wLandSeg4X * 64);
    const short wLandSeg5X = 1;
    const short wLandSeg5Z = 3;

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
        bridgeStepEndPos.vx, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz + (wLandZ * 64), 
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
        bridgeStepEndPos.vx + (wLandX * 64), bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz + ((wLandZ - 1) * 64), 
        windingIndices,
        0, 64, 64,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    CollisionBox* waterLandStartCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        bridgeStepEndPos.vx, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz, 
        wLandX * 64, 64, wLandZ * 64
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
        bridgeStepEndPos.vx + wLandSeg1OffsetX, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz + (wLandSeg1Z * 64), 
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
        bridgeStepEndPos.vx + wLandSeg2OffsetX, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz + ((wLandSeg2Z - 1) * 64), 
        reverseWindingIndices,
        0, 64, 64,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    StaticWorldGeometry* waterLandSeg2Back = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        bridgeStepEndPos.vx + wLandSeg2OffsetX, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz + (wLandSeg2Z * 64), 
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
        bridgeStepEndPos.vx + wLandSeg3OffsetX, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz + ((wLandSeg3Z - 1) * 64), 
        reverseWindingIndices,
        0, 64, 64,
        1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    StaticWorldGeometry* waterLandSegCombinedBack = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        bridgeStepEndPos.vx + wLandSeg3OffsetX, bridgeStepEndPos.vy + 64, bridgeStepEndPos.vz + (wLandSeg3Z * 64), 
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

    setVector(&waterLandEndPos, bridgeStepEndPos.vx + wLandSeg5OffsetX, bridgeStepEndPos.vy, bridgeStepEndPos.vz + (wLandSeg3Z * 64));

    // Temp wall
    StaticWorldGeometry* waterLandCliffWall = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        waterLandEndPos.vx + 64, bridgeStepEndPos.vy - 128, bridgeStepEndPos.vz - 128, 
        reverseWindingIndices,
        0, 128, 128,
        1, 2, 3,
        DRP_Neutral,
        &dlv_stonebrick_tim, &dlv_stonebrick_UVR
    );
}

void CreateAscent() {
    SVECTOR ascentPivot = waterLandEndPos;
    short stepHeight = 32;
    // ascentStartPos.vy - (64 * wholeSteps++)

    // Step 1
    const short step1X = 2;
    const short step1Z = 1;

    ascentPivot.vx += (64 - (step1X * 80));

    StaticWorldGeometry* ascStep1Floor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        ascentPivot.vx, ascentPivot.vy - stepHeight, ascentPivot.vz, 
        windingIndices,
        80, 0, 96,
        step1X, 1, step1Z,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    StaticWorldGeometry* ascStep1Front = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        ascentPivot.vx, ascentPivot.vy, ascentPivot.vz, 
        windingIndices,
        80, stepHeight, 0,
        step1X, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    CollisionBox* ascStep1Collision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        ascentPivot.vx, ascentPivot.vy, ascentPivot.vz, 
        step1X * 80, stepHeight, step1Z * 96
    );


    // Step 2
    const short step2X = 4;
    const short step2Z = 1;

    ascentPivot.vy -= stepHeight;
    ascentPivot.vz += step1Z * 96;

    StaticWorldGeometry* ascStep2Floor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        ascentPivot.vx, ascentPivot.vy - stepHeight, ascentPivot.vz, 
        windingIndices,
        64, 0, 96,
        step2X, 1, step2Z,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    StaticWorldGeometry* ascStep2Front = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        ascentPivot.vx, ascentPivot.vy, ascentPivot.vz, 
        windingIndices,
        80, stepHeight, 0,
        step1X, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    CollisionBox* ascStep2Collision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        ascentPivot.vx, ascentPivot.vy, ascentPivot.vz, 
        step2X * 64, stepHeight, step2Z * 96
    );


    // Step 3
    const short step3X = 4;
    const short step3Z = 2;

    ascentPivot.vx += 64;
    ascentPivot.vy -= stepHeight;
    ascentPivot.vz += step2Z * 96;

    StaticWorldGeometry* ascStep3Floor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        ascentPivot.vx, ascentPivot.vy - stepHeight, ascentPivot.vz, 
        windingIndices,
        64, 0, 64,
        step3X, 1, step3Z,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    StaticWorldGeometry* ascStep3Front = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        ascentPivot.vx, ascentPivot.vy, ascentPivot.vz, 
        windingIndices,
        64, stepHeight, 0,
        step3X - 1, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    CollisionBox* ascStep3Collision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        ascentPivot.vx, ascentPivot.vy, ascentPivot.vz, 
        step3X * 64, stepHeight, step3Z * 64
    );


    // Step 4
    const short step4X = 4;
    const short step4Z = 4;

    ascentPivot.vx += step3X * 64;
    ascentPivot.vy -= stepHeight;

    StaticWorldGeometry* ascStep4Floor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        ascentPivot.vx, ascentPivot.vy - stepHeight, ascentPivot.vz, 
        windingIndices,
        64, 0, 64,
        step4X, 1, step4Z,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    StaticWorldGeometry* ascStep4Left = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        ascentPivot.vx, ascentPivot.vy, ascentPivot.vz, 
        reverseWindingIndices,
        0, stepHeight, 64,
        1, 1, step4Z,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    CollisionBox* ascStep4Collision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        ascentPivot.vx, ascentPivot.vy, ascentPivot.vz, 
        step4X * 64, stepHeight, step4Z * 64
    );


    // Step 5
    const short step5X = 4;
    const short step5Z = 5;

    ascentPivot.vx += step4X * 64;
    ascentPivot.vy -= stepHeight;
    ascentPivot.vz -= 2 * 64;

    StaticWorldGeometry* ascStep5Floor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        ascentPivot.vx, ascentPivot.vy - stepHeight, ascentPivot.vz, 
        windingIndices,
        64, 0, 64,
        step5X, 1, step5Z,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    StaticWorldGeometry* ascStep5Left = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        ascentPivot.vx, ascentPivot.vy, ascentPivot.vz + (2 * 64), 
        reverseWindingIndices,
        0, stepHeight, 64,
        1, 1, step4Z - 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    CollisionBox* ascStep5Collision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        ascentPivot.vx, ascentPivot.vy, ascentPivot.vz, 
        step5X * 64, stepHeight, step5Z * 64
    );


    // Step 6
    const short step6X = 3;
    const short step6Z = 4;

    ascentPivot.vx += 96;
    ascentPivot.vy -= stepHeight;
    ascentPivot.vz -= step6Z * 64;

    StaticWorldGeometry* ascStep6Floor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        ascentPivot.vx, ascentPivot.vy - stepHeight, ascentPivot.vz, 
        windingIndices,
        72, 0, 64,
        step6X, 1, step6Z,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    StaticWorldGeometry* ascStep6Back = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        ascentPivot.vx, ascentPivot.vy, ascentPivot.vz + (step6Z * 64), 
        reverseWindingIndices,
        80, stepHeight, 0,
        2, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    CollisionBox* ascStep6Collision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        ascentPivot.vx, ascentPivot.vy, ascentPivot.vz, 
        step6X * 72, stepHeight, step6Z * 64
    );


    // Step 7
    const short step7X = 3;
    const short step7Z = 3;

    const short step7_1X = 2;
    const short step7_1Z = 1;

    ascentPivot.vy -= stepHeight;
    ascentPivot.vz -= step7Z * 64;

    StaticWorldGeometry* ascStep7Floor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        ascentPivot.vx, ascentPivot.vy - stepHeight, ascentPivot.vz, 
        windingIndices,
        64, 0, 64,
        step7X, 1, step7Z,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    StaticWorldGeometry* ascStep7_1Floor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        ascentPivot.vx, ascentPivot.vy - stepHeight, ascentPivot.vz - (step7_1Z * 64), 
        windingIndices,
        72, 0, 64,
        2, 1, 1,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    StaticWorldGeometry* ascStep7Back = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        ascentPivot.vx, ascentPivot.vy, ascentPivot.vz + (step7Z * 64), 
        reverseWindingIndices,
        64, stepHeight, 0,
        step7Z, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    CollisionBox* ascStep7Collision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        ascentPivot.vx, ascentPivot.vy, ascentPivot.vz - (step7_1Z * 64), 
        (step7X * 64), stepHeight, (step7Z * 64) + (step7_1Z * 64)
    );


    // Step 8
    const short step8X = 1;
    const short step8Z = 3;

    ascentPivot.vx -= step8X * 64;
    ascentPivot.vy -= stepHeight;
    ascentPivot.vz -= step7_1Z * 64;

    stepHeight = 16;

    StaticWorldGeometry* ascStep8Floor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        ascentPivot.vx, ascentPivot.vy - stepHeight, ascentPivot.vz, 
        windingIndices,
        64, 0, 64,
        step8X, 1, step8Z,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    StaticWorldGeometry* ascStep8Right = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        ascentPivot.vx + (step8X * 64), ascentPivot.vy, ascentPivot.vz, 
        windingIndices,
        0, stepHeight, 64,
        1, 1, step8Z,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    CollisionBox* ascStep8Collision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        ascentPivot.vx, ascentPivot.vy, ascentPivot.vz, 
        step8X * 64, stepHeight, step8Z * 64
    );

    ascentEndPos = ascentPivot;
}


void CreatePlateau() {
    SVECTOR plateauPivot = ascentEndPos;

    // start section
    const short plStart1X = 2;
    const short plStart1Z = 4;

    const short plStart2X = 2;
    const short plStart2Z = 6;

    const short plStart3X = 3;
    const short plStart3Z = 6;

    const short plStartCombX = plStart1X + plStart2X + plStart3X;

    plateauPivot.vx -= plStart1X * 64;
    plateauPivot.vy -= 32;

    StaticWorldGeometry* plStart1Floor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        plateauPivot.vx, plateauPivot.vy, plateauPivot.vz, 
        windingIndices,
        64, 0, 64,
        plStart1X, 1, plStart1Z,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    StaticWorldGeometry* plStartRight = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        plateauPivot.vx + (plStart1X * 64), plateauPivot.vy + 16, plateauPivot.vz, 
        windingIndices,
        0, 16, 64,
        1, 1, plStart1Z - 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    plateauPivot.vx -= plStart2X * 64;

    StaticWorldGeometry* plStart2Floor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        plateauPivot.vx, plateauPivot.vy, plateauPivot.vz, 
        windingIndices,
        64, 0, 64,
        plStart2X, 1, plStart2Z,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    plateauPivot.vx -= plStart3X * 64;

    StaticWorldGeometry* plStart3Floor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        plateauPivot.vx, plateauPivot.vy, plateauPivot.vz, 
        windingIndices,
        64, 0, 64,
        plStart3X, 1, plStart3Z,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    CollisionBox* plStartFloorCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        plateauPivot.vx, plateauPivot.vy + 32, plateauPivot.vz, 
        plStartCombX * 64, 32, plStart3Z * 64
    );

    plateauPivot.vx += 128;

    const u_char plWaterfall1X = 2;
    const u_char plWaterfall1Z = 1;
    const short plWaterfall1XLen = 96;
    const short plWaterfall1ZLen = 96;

    StaticWorldGeometry* plWaterfallStep1Back = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        plateauPivot.vx, plateauPivot.vy, plateauPivot.vz, 
        reverseWindingIndices,
        plWaterfall1XLen, 32, 0,
        plWaterfall1X, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    plateauPivot.vy -= 32;
    plateauPivot.vz -= plWaterfall1Z * plWaterfall1ZLen;

    StaticWorldGeometry* plWaterfallStep1Floor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        plateauPivot.vx, plateauPivot.vy, plateauPivot.vz, 
        windingIndices,
        plWaterfall1XLen, 0, plWaterfall1ZLen,
        plWaterfall1X, 1, plWaterfall1Z,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    CollisionBox* plWaterfallStep1Collision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        plateauPivot.vx, plateauPivot.vy + 64, plateauPivot.vz, 
        plWaterfall1X * plWaterfall1XLen, 64, plWaterfall1Z * plWaterfall1ZLen
    );


    StaticWorldGeometry* plWaterfallStep2Back = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        plateauPivot.vx, plateauPivot.vy, plateauPivot.vz, 
        reverseWindingIndices,
        plWaterfall1XLen, 32, 0,
        plWaterfall1X, 1, 1,
        DRP_Neutral,
        &grassydirt_tim, &grassydirt_UVR
    );

    plateauPivot.vy -= 32;
    plateauPivot.vz -= plWaterfall1Z * plWaterfall1ZLen;

    StaticWorldGeometry* plWaterfallStep2Floor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        plateauPivot.vx, plateauPivot.vy, plateauPivot.vz, 
        windingIndices,
        plWaterfall1XLen, 0, plWaterfall1ZLen,
        plWaterfall1X, 1, plWaterfall1Z,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    CollisionBox* plWaterfallStep2Collision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        plateauPivot.vx, plateauPivot.vy + 64, plateauPivot.vz, 
        plWaterfall1X * plWaterfall1XLen, 64, plWaterfall1Z * plWaterfall1ZLen
    );

    const u_char bigGrassX = 4;
    const u_char bigGrassZ = 3;
    const short bigGrassXLen = 128;
    const short bigGrassZLen = 128;

    plateauPivot.vz -= bigGrassZ * bigGrassZLen;

    StaticWorldGeometry* plWaterfallBigGrassFloor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        plateauPivot.vx, plateauPivot.vy, plateauPivot.vz, 
        windingIndices,
        bigGrassXLen, 0, bigGrassZLen,
        bigGrassX, 1, bigGrassZ,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    CollisionBox* plWaterfallBigGrassCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        plateauPivot.vx, plateauPivot.vy + 64, plateauPivot.vz, 
        bigGrassX * bigGrassXLen, 64, bigGrassZ * bigGrassZLen
    );

    const u_char bigGrass2X = 4;
    const u_char bigGrass2Z = 4;
    const short bigGrass2XLen = 116;
    const short bigGrass2ZLen = 128;

    plateauPivot.vx += 48;
    plateauPivot.vz -= bigGrass2Z * bigGrass2ZLen;

    StaticWorldGeometry* plWaterfallBigGrass2Floor = CreateStaticWorldGeometry(
        overworldData.TiledPolys,
        plateauPivot.vx, plateauPivot.vy, plateauPivot.vz, 
        windingIndices,
        bigGrass2XLen, 0, bigGrass2ZLen,
        bigGrass2X, 1, bigGrass2Z,
        DRP_Neutral,
        &grass_tim, 0, 192, 63, 63
    );

    CollisionBox* plWaterfallBigGrass2Collision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        plateauPivot.vx, plateauPivot.vy + 64, plateauPivot.vz, 
        bigGrass2X * bigGrass2XLen, 64, bigGrass2Z * bigGrass2ZLen
    );
}


void CreateWaterfall() {
    SVECTOR waterfallPivot = waterfallStartCorner;

    const short floorXOffset = 64;

    waterfallPivot.vz -= 96 * 2;

    StaticWorldGeometry* wfLeftRockLeft = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        waterfallStartCorner.vx, waterfallPivot.vy, waterfallPivot.vz, 
        reverseWindingIndices,
        0, 96, 96,
        1, 1, 1,
        DRP_Neutral,
        &dlv_slate_tim, &dlv_slate_UVR
    );

    StaticWorldGeometry* wfLeftRockFront = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        waterfallStartCorner.vx, waterfallPivot.vy, waterfallPivot.vz, 
        windingIndices,
        floorXOffset, 96, 0,
        1, 1, 1,
        DRP_Neutral,
        &dlv_slate_tim, &dlv_slate_UVR
    );

    waterfallPivot.vx += floorXOffset;
    waterfallPivot.vy -= 32;

    StaticWorldGeometry* wfLeftFloor = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        waterfallPivot.vx, waterfallPivot.vy, waterfallPivot.vz, 
        windingIndices,
        120, 0, 96,
        2, 1, 1,
        DRP_Neutral,
        &dlv_slate_tim, &dlv_slate_UVR
    );

    CollisionBox* wfLeftFloorCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        waterfallPivot.vx, waterfallPivot.vy + 48, waterfallPivot.vz, 
        2 * 120, 48, 96
    );

    
    waterfallPivot.vy += 32;
    waterfallPivot.vz -= 128;

    const short wfStreamWidth = 128;

    StaticWorldGeometry* wfStream1Floor = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        waterfallStartCorner.vx, waterfallPivot.vy, waterfallPivot.vz, 
        windingIndices,
        88, 0, wfStreamWidth,
        2, 1, 1,
        DRP_Neutral,
        &dirt_tim, &dirt_UVR
    );

    CollisionBox* plWaterfallStream1Collision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        waterfallStartCorner.vx, waterfallPivot.vy + 64, waterfallPivot.vz, 
        2 * 88, 64, wfStreamWidth
    );

    waterfallPivot.vz -= 96;

    StaticWorldGeometry* wfRightRockLeft = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        waterfallStartCorner.vx, waterfallPivot.vy, waterfallPivot.vz, 
        reverseWindingIndices,
        0, 96, 96,
        1, 1, 1,
        DRP_Neutral,
        &dlv_slate_tim, &dlv_slate_UVR
    );

    waterfallPivot.vy -= 32;
    waterfallPivot.vz -= 5 * 96;

    StaticWorldGeometry* wfRightFloor = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        waterfallPivot.vx, waterfallPivot.vy, waterfallPivot.vz, 
        windingIndices,
        112, 0, 96,
        1, 1, 6,
        DRP_Neutral,
        &dlv_slate_tim, &dlv_slate_UVR
    );

    CollisionBox* wfRightFloorCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        waterfallPivot.vx, waterfallPivot.vy + 48, waterfallPivot.vz, 
        112, 48, 96 * 6
    );

    waterfallPivot.vx += 112;
    waterfallPivot.vy += 32;
    waterfallPivot.vz += 64;

    StaticWorldGeometry* wfStream2Floor = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        waterfallPivot.vx, waterfallPivot.vy, waterfallPivot.vz, 
        windingIndices,
        wfStreamWidth, 0, wfStreamWidth,
        1, 1, 5,
        DRP_Neutral,
        &dirt_tim, &dirt_UVR
    );

    CollisionBox* wfStream2Collision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        waterfallPivot.vx, waterfallPivot.vy + 64, waterfallPivot.vz, 
        128, 64, 5 * 128
    );

    waterfallPivot.vy -= 32;
    waterfallPivot.vz -= 64;

    StaticWorldGeometry* wfStreamEndFloor = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        waterfallPivot.vx, waterfallPivot.vy, waterfallPivot.vz, 
        windingIndices,
        128, 0, 64,
        1, 1, 1,
        DRP_Neutral,
        &dlv_slate_tim, &dlv_slate_UVR
    );

    CollisionBox* wfStreamEndCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        waterfallPivot.vx, waterfallPivot.vy + 48, waterfallPivot.vz, 
        128, 48, 64
    );

    waterfallPivot.vx += wfStreamWidth;
    //96 * 6

    StaticWorldGeometry* wfStreamPlBorderFloor = CreateStaticWorldGeometryUVRect(
        overworldData.TiledPolys,
        waterfallPivot.vx, waterfallPivot.vy, waterfallPivot.vz, 
        windingIndices,
        80, 0, 100,
        1, 1, 8,
        DRP_Neutral,
        &dlv_slate_tim, &dlv_slate_UVR
    );

    CollisionBox* wfStreamPlBorderCollision = CreateCollisionBox(
        overworldData.CollisionBoxes,
        waterfallPivot.vx, waterfallPivot.vy + 48, waterfallPivot.vz, 
        80, 48, 100 * 8
    );
}
