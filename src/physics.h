#ifndef __PHYSICS_H
#define __PHYSICS_H

#define ACTIVECOLBOXCOUNT 6

#include <stdlib.h>
#include <libgte.h>
#include <libgpu.h>

#include "objects.h"

typedef struct PhysicsResolutionEntry {
    u_short objIndex;
    u_char axis; // 0 = X, 1 = Y, 2 = Z
    u_char padding; // Do not use
    long value;
} PhysicsResolutionEntry;

extern PhysicsResolutionEntry PhysicsResolutionTable[16];
extern StaticCollisionPolyBox* activeCollisionPolyBoxes[ACTIVECOLBOXCOUNT];

void ScanForOverlaps(const VECTOR* pMins, const VECTOR* pMaxs, const StaticCollisionPolyBox* scpolybox, CollisionOverlaps* overlaps);
bool CanPlayerStep(const VECTOR* position);
void SimulatePlayerMovementCollision();
void ResolveOverlaps(const PhysicsResolutionEntry* table, const size_t tableSize);

#endif