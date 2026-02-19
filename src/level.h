#ifndef __LEVEL_H
#define __LEVEL_H

#include "graphics.h"
#include "objects.h"
#include "geometry.h"
#include "player.h"
#include "clist.h"


typedef struct LevelData {
    GenericPtrList* TiledPolys;
    GenericPtrList* PolyBoxes;
    GenericPtrList* CollisionBoxes;
    LinkedList* Collectibles;
} LevelData;

extern LevelData* CurrentLevelData;

#endif
