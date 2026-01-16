#include "physics.h"
#include "player.h"


PhysicsResolutionEntry PhysicsResolutionTable[16] = { 0 };
StaticCollisionPolyBox* activeCollisionPolyBoxes[ACTIVECOLBOXCOUNT] = { 0 };

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
    VECTOR playerSimulatedPosition = player->poly.obj.position;
    addVector(&playerSimulatedPosition, &player->poly.obj.velocity);
    VECTOR playerSimulatedPositionFinal = playerSimulatedPosition; // This variable likely is not needed, but kept for now

    // Unsure if these should be recalculated per object, but probably not?
    
    /*
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
    */

    for (size_t i = 0; i < ACTIVECOLBOXCOUNT; i++) {
        CollisionOverlaps overlaps = { 0 };
        bool stepping = false;

        
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

                    /*
                        *** Bonking head against top platform bug: ***
                        Because the platforms are indexed from top to bottom, hitting the top platform while hugging the middle one
                        results in bonking it, because the push on X from the middle platform is performed *after* the top platform detected a Y overlap
                    */

                    // Landing on something
                    if (bleedYPos < bleedYNeg) {
                        FntPrint("YV: %06d\n", player->poly.obj.velocity.vy);
                        bleed[1] = -bleedYPos;

                        if (player->poly.obj.velocity.vy < 0) {
                            ignoreY = true;
                        }
                    }
                    // Hitting head against something
                    else if (bleedYPos >= bleedYNeg) {
                        FntPrint("YV: %06d\n", player->poly.obj.velocity.vy);
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

                    // Can probably save a few cycles here by not comparing X to itself
                    for (size_t i = 0; i < 3; i++) {
                        if (i == 1 && ignoreY) {
                            continue;
                        }

                        if (abs(bleed[i]) < bleedValue) {
                            bleedValue = abs(bleed[i]);
                            index = i;

                            FntPrint("%d < %d: true\n", abs(bleed[i]), bleedValue);
                        }
                        else {
                            FntPrint("%d < %d: false\n", abs(bleed[i]), bleedValue);
                        }
                    }

                    if (index == 0) {
                        playerSimulatedPositionFinal.vx += bleed[0];
                        player->poly.obj.velocity.vx = 0;
                    }
                    else if (index == 1) {
                        playerSimulatedPositionFinal.vy += bleed[1];

                        // If player is pushed up
                        if (bleed[1] < 0) {
                            isPlayerOnCollision = true;
                        }
                        else {
                            player->poly.obj.velocity.vy = 0;
                        }
                    }
                    else {
                        playerSimulatedPositionFinal.vz += bleed[2];
                        player->poly.obj.velocity.vz = 0;
                    }

                    FntPrint("Colbox Index: %d\n", i);
                    FntPrint("BleedX: %06d / %06d\n", bleedXPos, bleedXNeg);
                    FntPrint("BleedY: %06d / %06d\n", bleedYPos, bleedYNeg);
                    FntPrint("BleedZ: %06d / %06d\n", bleedZPos, bleedZNeg);
                    FntPrint("Bleed Index: %d\n", index);

                    //FntPrint("Y: %d, YDim: %d", activeCollisionPolyBoxes[i]->transform.t[1], activeCollisionPolyBoxes[i]->colBox.dimensions.vy);
                }
            }
            // If on the box
            else if (playerSimulatedPositionGridMins.vy == activeCollisionPolyBoxes[i]->transform.t[1] - activeCollisionPolyBoxes[i]->colBox.dimensions.vy
                && player->poly.obj.velocity.vy == 0) {

                isPlayerOnCollision = true;
            }
            
            //player->poly.obj.position = playerSimulatedPosition;
        }
    }

    player->poly.obj.position = playerSimulatedPositionFinal;
}

void ResolveOverlaps(const PhysicsResolutionEntry* table, const size_t tableSize) {
    // Sort if X or Z resolution should be done first based on (possibly) player velocity
    // Move resolution logic to this function
    // Do a second ScanForOverlaps on the i > 0 object in this function's expected for loop, to check if the previous plane resolution avoids a Y overlap
    // Otherwise resolve as normal
}
