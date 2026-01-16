#ifndef __PLAYER_H
#define __PLAYER_H

#include <libgte.h>
#include <libgpu.h>

#include "objects.h"

typedef struct PlayerObject {
    PolyObject poly;
    CameraObject* cameraPtr;
    VECTOR velocity;
} PlayerObject;

extern PlayerObject* player;
extern bool isPlayerOnFloor;
extern bool isPlayerOnCollision;

#endif