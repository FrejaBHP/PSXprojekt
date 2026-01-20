#include "physics.h"
#include "player.h"


PhysicsResolutionEntry PhysicsResolutionTable[16] = { 0 };
StaticCollisionPolyBox* activeCollisionPolyBoxes[ACTIVECOLBOXCOUNT] = { 0 };

VECTOR playerSimulatedPosition;
VECTOR playerSimulatedPositionFinal;

static int CompareAxisSortX(const void* a, const void* b) {
    PhysicsResolutionEntry dataA = *(const PhysicsResolutionEntry*)a;
    PhysicsResolutionEntry dataB = *(const PhysicsResolutionEntry*)b;

    if (dataA.axis == 1 || dataB.axis == 1) {
        return 0;
    }

    if (dataA.axis < dataB.axis) {
        return -1;
    }

    if (dataA.axis > dataB.axis) {
        return 1;
    }

    return 0;
}

static int CompareAxisSortZ(const void* a, const void* b) {
    PhysicsResolutionEntry dataA = *(const PhysicsResolutionEntry*)a;
    PhysicsResolutionEntry dataB = *(const PhysicsResolutionEntry*)b;

    if (dataA.axis == 1 || dataB.axis == 1) {
        return 0;
    }

    if (dataA.axis > dataB.axis) {
        return -1;
    }

    if (dataA.axis < dataB.axis) {
        return 1;
    }

    return 0;
}

// overlaps is treated as an out parameter
void ScanForOverlaps(const VECTOR* pMins, const VECTOR* pMaxs, const StaticCollisionPolyBox* scpolybox, CollisionOverlaps* overlaps) {
    if (pMins->vx < scpolybox->transform.t[0] + scpolybox->colBox.dimensions.vx
        && (pMaxs->vx > scpolybox->transform.t[0])) {
        
        overlaps->x = true;
    }

    if (pMins->vy > scpolybox->transform.t[1] - scpolybox->colBox.dimensions.vy
        && (pMaxs->vy < scpolybox->transform.t[1])) {
        
        overlaps->y = true;
    }

    if (pMins->vz < scpolybox->transform.t[2] + scpolybox->colBox.dimensions.vz 
        && (pMaxs->vz > scpolybox->transform.t[2])) {
        
        overlaps->z = true;
    }
}

bool CanPlayerStep(const VECTOR* position) {
    bool canStep = true;

    VECTOR gridMins = { 
        (position->vx >> 12) - player->poly.boxWidth / 2,
        (position->vy >> 12),
        (position->vz >> 12) - player->poly.boxWidth / 2
    };

    VECTOR gridMaxs = { 
        (position->vx >> 12) + player->poly.boxWidth / 2,
        (position->vy >> 12) - player->poly.boxHeight,
        (position->vz >> 12) + player->poly.boxWidth / 2
    };

    for (size_t i = 0; i < ACTIVECOLBOXCOUNT; i++) {
        CollisionOverlaps overlaps = { 0 };
        ScanForOverlaps(&gridMins, &gridMaxs, activeCollisionPolyBoxes[i], &overlaps);

        if (overlaps.x == true && overlaps.y == true && overlaps.z == true) {
            canStep = false;
            break;
        }
    }

    return canStep;
}

void SimulatePlayerMovementCollision() {
    isPlayerOnCollision = false;
    bool playerHasStepped = false;

    // In fixed-point units, aka 4096 = 1
    playerSimulatedPosition = player->poly.obj.position;
    addVector(&playerSimulatedPosition, &player->poly.obj.velocity);
    playerSimulatedPositionFinal = playerSimulatedPosition; // This variable likely is not needed, but kept for now

    // Might want to move these somewhere else, likely as a global in this file
    VECTOR playerSimulatedPositionGridMins = { 
        (playerSimulatedPosition.vx >> 12) - player->poly.boxWidth / 2,
        (playerSimulatedPosition.vy >> 12),
        (playerSimulatedPosition.vz >> 12) - player->poly.boxWidth / 2
    };

    VECTOR playerSimulatedPositionGridMaxs = { 
        (playerSimulatedPosition.vx >> 12) + player->poly.boxWidth / 2,
        (playerSimulatedPosition.vy >> 12) - player->poly.boxHeight,
        (playerSimulatedPosition.vz >> 12) + player->poly.boxWidth / 2
    };

    size_t overlapsCount = 0;

    for (size_t i = 0; i < ACTIVECOLBOXCOUNT; i++) {
        CollisionOverlaps overlaps = { 0 };
        bool stepping = false;

        ScanForOverlaps(&playerSimulatedPositionGridMins, &playerSimulatedPositionGridMaxs, activeCollisionPolyBoxes[i], &overlaps);

        //FntPrint("%d %d %d\n", intersectsX, intersectsY, intersectsZ);

        // If player is at least above or underneath a collision box
        if (overlaps.x && overlaps.z) {
            // If player is actually trying to enter collision box
            if (overlaps.y) {
                if (player->poly.obj.velocity.vy == 0 && isPlayerOnFloor && !playerHasStepped) {
                    long stepheight = playerSimulatedPositionGridMins.vy - activeCollisionPolyBoxes[i]->transform.t[1] + activeCollisionPolyBoxes[i]->colBox.dimensions.vy;
                    //FntPrint("StepHeight: %03d\n", stepheight);

                    if (stepheight <= 32 && stepheight > 0) {
                        // Second simulated position to check if player is trying to step up into geometry
                        VECTOR playerStepPosition = playerSimulatedPositionFinal;
                        playerStepPosition.vy = activeCollisionPolyBoxes[i]->position.vy - (activeCollisionPolyBoxes[i]->colBox.dimensions.vy * ONE);

                        if (CanPlayerStep(&playerStepPosition)) {
                            stepping = true;
                            playerHasStepped = true;
                            playerSimulatedPositionFinal.vy = activeCollisionPolyBoxes[i]->position.vy - (activeCollisionPolyBoxes[i]->colBox.dimensions.vy * ONE);
                            isPlayerOnCollision = true;
                        }
                    }
                }

                if (!stepping) {
                    // Bleeding, as in clipping/overlapping - not losing blood
                    long bleed[3]; // X Y Z
                    bool ignoreY = false;

                    long bleedXPos = abs((playerSimulatedPositionFinal.vx + ((player->poly.boxWidth / 2) * ONE)) - activeCollisionPolyBoxes[i]->position.vx);
                    long bleedXNeg = abs((playerSimulatedPositionFinal.vx - ((player->poly.boxWidth / 2) * ONE)) - (activeCollisionPolyBoxes[i]->position.vx + (activeCollisionPolyBoxes[i]->colBox.dimensions.vx * ONE)));
                    
                    long bleedYPos = abs((activeCollisionPolyBoxes[i]->position.vy - (activeCollisionPolyBoxes[i]->colBox.dimensions.vy * ONE)) - playerSimulatedPositionFinal.vy);
                    long bleedYNeg = abs(activeCollisionPolyBoxes[i]->position.vy - (playerSimulatedPositionFinal.vy - ((player->poly.boxHeight) * ONE)));

                    long bleedZPos = abs((playerSimulatedPositionFinal.vz + ((player->poly.boxWidth / 2) * ONE)) - activeCollisionPolyBoxes[i]->position.vz);
                    long bleedZNeg = abs((playerSimulatedPositionFinal.vz - ((player->poly.boxWidth / 2) * ONE)) - (activeCollisionPolyBoxes[i]->position.vz + (activeCollisionPolyBoxes[i]->colBox.dimensions.vz * ONE)));

                    if (bleedXPos <= bleedXNeg) {
                        bleed[0] = -bleedXPos;
                    }
                    else if (bleedXPos > bleedXNeg) {
                        bleed[0] = bleedXNeg;
                    }

                    // Landing on something
                    if (bleedYPos < bleedYNeg) {
                        bleed[1] = -bleedYPos;

                        if (player->poly.obj.velocity.vy < 0) {
                            ignoreY = true;
                        }
                    }
                    // Hitting head against something
                    else if (bleedYPos >= bleedYNeg) {
                        bleed[1] = bleedYNeg;

                        if (player->poly.obj.velocity.vy >= 0) {
                            ignoreY = true;
                        }
                    }

                    if (bleedZPos <= bleedZNeg) {
                        bleed[2] = -bleedZPos;
                    }
                    else if (bleedZPos > bleedZNeg) {
                        bleed[2] = bleedZNeg;
                    }

                    size_t index = 0;
                    long bleedValue = abs(bleed[0]);

                    //FntPrint("%d < %d\n", abs(bleed[0]), bleedValue);

                    for (size_t a = 1; a < 3; a++) {
                        if (a == 1 && ignoreY) {
                            continue;
                        }

                        //FntPrint("%d < %d\n", abs(bleed[a]), bleedValue);

                        if (abs(bleed[a]) < bleedValue) {
                            bleedValue = abs(bleed[a]);
                            index = a;
                        }
                    }

                    if (overlapsCount < 16) {
                        PhysicsResolutionTable[overlapsCount].objIndex = i;
                        PhysicsResolutionTable[overlapsCount].axis = index;
                        PhysicsResolutionTable[overlapsCount].overlapType = 0;
                        PhysicsResolutionTable[overlapsCount].value = bleed[index];
                        overlapsCount++;
                    }
                }
            }
            // If on the box
            else if (playerSimulatedPositionGridMins.vy == activeCollisionPolyBoxes[i]->transform.t[1] - activeCollisionPolyBoxes[i]->colBox.dimensions.vy
                && player->poly.obj.velocity.vy == 0) {

                if (overlapsCount < 16) {
                    PhysicsResolutionTable[overlapsCount].objIndex = i;
                    PhysicsResolutionTable[overlapsCount].axis = 255;
                    PhysicsResolutionTable[overlapsCount].overlapType = 1;
                    PhysicsResolutionTable[overlapsCount].value = 0;
                    overlapsCount++;
                }
            }
        }
    }

    SortAndResolveOverlaps(overlapsCount);
}

void SortAndResolveOverlaps(const size_t numEntries) {
    if (numEntries == 1) {
        ResolveOverlaps(PhysicsResolutionTable, 1);
    }
    else {
        PhysicsResolutionEntry SortedTable[numEntries];
        size_t curSortedIndex = 0;
        size_t lastSortedIndex = numEntries - 1;

        // First sorting just to have Y and 2D overlaps at the bottom
        for (size_t i = 0; i < numEntries; i++) {
            if (PhysicsResolutionTable[i].axis == 1 || PhysicsResolutionTable[i].overlapType == 1) {
                SortedTable[lastSortedIndex] = PhysicsResolutionTable[i];
                lastSortedIndex--;
            }
            else {
                SortedTable[curSortedIndex] = PhysicsResolutionTable[i];
                curSortedIndex++;
            }
        }

        if (abs(player->poly.obj.velocity.vx) > abs(player->poly.obj.velocity.vz)) {
            qsort(SortedTable, numEntries, sizeof(PhysicsResolutionEntry), CompareAxisSortX);
        }
        else {
            qsort(SortedTable, numEntries, sizeof(PhysicsResolutionEntry), CompareAxisSortZ);
        }

        ResolveOverlaps(SortedTable, numEntries);
    }
    
    
    // Sort if X or Z resolution should be done first based on (possibly) player velocity
    // Move resolution logic to this function
    // Do a second ScanForOverlaps on the i > 0 object in this function's expected for loop, to check if the previous plane resolution avoids a Y overlap
    // Otherwise resolve as normal
}

void ResolveOverlaps(const PhysicsResolutionEntry* table, const size_t numEntries) {
    //bool skipRecalc = false;

    for (size_t i = 0; i < numEntries; i++) {
        VECTOR playerSimulatedPositionGridMins = { 
            (playerSimulatedPositionFinal.vx >> 12) - player->poly.boxWidth / 2,
            (playerSimulatedPositionFinal.vy >> 12),
            (playerSimulatedPositionFinal.vz >> 12) - player->poly.boxWidth / 2
        };

        VECTOR playerSimulatedPositionGridMaxs = { 
            (playerSimulatedPositionFinal.vx >> 12) + player->poly.boxWidth / 2,
            (playerSimulatedPositionFinal.vy >> 12) - player->poly.boxHeight,
            (playerSimulatedPositionFinal.vz >> 12) + player->poly.boxWidth / 2
        };

        CollisionOverlaps overlaps = { 0 };
        ScanForOverlaps(&playerSimulatedPositionGridMins, &playerSimulatedPositionGridMaxs, activeCollisionPolyBoxes[table[i].objIndex], &overlaps);

        if (!overlaps.x && !overlaps.y && !overlaps.z) {
            continue;
        }

        if (overlaps.x && overlaps.y && overlaps.z) {
            if (table[i].axis == 0) {
                playerSimulatedPositionFinal.vx += table[i].value;
                player->poly.obj.velocity.vx = 0;
            }
            else if (table[i].axis == 1) {
                playerSimulatedPositionFinal.vy += table[i].value;

                // If player is pushed up
                if (table[i].value < 0) {
                    isPlayerOnCollision = true;
                }
                else {
                    player->poly.obj.velocity.vy = 0;
                }
            }
            else if (table[i].axis == 2) {
                playerSimulatedPositionFinal.vz += table[i].value;
                player->poly.obj.velocity.vz = 0;
            }

            //FntPrint("Colbox Index: %d\n", table[i].objIndex);
            //FntPrint("Bleed Index: %d\n", table[i].axis);
            //FntPrint("Bleed Value: %d\n", table[i].value);
        }
        else if (overlaps.x && overlaps.z && playerSimulatedPositionGridMins.vy == activeCollisionPolyBoxes[table[i].objIndex]->transform.t[1] - activeCollisionPolyBoxes[table[i].objIndex]->colBox.dimensions.vy
            && player->poly.obj.velocity.vy == 0) {

            //FntPrint("Standing on colbox Index: %d\n", table[i].objIndex);

            isPlayerOnCollision = true;
        }
        
    }

    player->poly.obj.position = playerSimulatedPositionFinal;
}
