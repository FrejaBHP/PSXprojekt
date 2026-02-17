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
    AddItemToGenericPtrList(&overworldData.TiledPolys, tiledWall);

    TiledTexturedPolyObject* tiledWallSide2 = CreateTiledTexturedPolyObjectFT4(
        startX + 384, 0, -160, 
        0, 0, 0,
        windingIndices,
        64, 64, 0,
        3, 1, 1,
        DRP_Neutral,
        &grassydirt_tim,
        64, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, tiledWallSide2);

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
    AddItemToGenericPtrList(&overworldData.TiledPolys, tiledWallSide);

    TiledTexturedPolyObject* slateFloor = CreateTiledTexturedPolyObjectFT4(
        startX, 0, -160, 
        0, 0, 0,
        windingIndices,
        64, 0, 64,
        6, 1, 4,
        DRP_Low,
        &grass_tim,
        0, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, slateFloor);

    TiledTexturedPolyObject* slateFloor1 = CreateTiledTexturedPolyObjectFT4(
        startX, -128, 96, 
        0, 0, 0,
        windingIndices,
        64, 0, 64,
        9, 1, 4,
        DRP_Low,
        &grass_tim,
        0, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, slateFloor1);

    TiledTexturedPolyObject* grassyHillFloor = CreateTiledTexturedPolyObjectFT4(
        startX + 384, -64, -160, 
        0, 0, 0,
        windingIndices,
        64, 0, 64,
        3, 1, 4,
        DRP_Low,
        &grass_tim,
        0, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, grassyHillFloor);

    TiledTexturedPolyObject* grassyHillHalfwall = CreateTiledTexturedPolyObjectFT4(
        startX + 384, -64, 96, 
        0, 0, 0,
        windingIndices,
        64, 64, 0,
        3, 1, 1,
        DRP_Neutral,
        &grassydirt_tim,
        64, 192, 63, 63
    );
    AddItemToGenericPtrList(&overworldData.TiledPolys, grassyHillHalfwall);

    
    CollisionBox* wallTestCollision = CreateCollisionBox(
        startX, 0, 96,
        576, 128, 256
    );
    AddItemToGenericPtrList(&overworldData.CollisionBoxes, wallTestCollision);

    CollisionBox* wallTestCollision1 = CreateCollisionBox(
        startX + 384, 0, -160,
        192, 64, 256
    );
    AddItemToGenericPtrList(&overworldData.CollisionBoxes, wallTestCollision1);
    
}
