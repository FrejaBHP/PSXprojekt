#include <stdlib.h>

#include "player.h"


PlayerObject* player = NULL;
bool isPlayerOnFloor = true;
bool isPlayerOnCollision = false;
int collectedCoins = 0;

bool isCrossHeld = false;
bool isSquareHeld = false;
bool isTriangleHeld = false;
bool isCircleHeld = false;
