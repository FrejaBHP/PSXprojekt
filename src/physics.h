#ifndef __PHYSICS_H
#define __PHYSICS_H

#include <stdlib.h>
#include <libgte.h>
#include <libgpu.h>

#include "objects.h"
#include "clist.h"

typedef struct PhysicsResolutionEntry {
    u_short objIndex;
    u_char axis; // 0 = X, 1 = Y, 2 = Z
    u_char overlapType; // 0 = XYZ, 1 = XZ
    long value;
} PhysicsResolutionEntry;

extern PhysicsResolutionEntry PhysicsResolutionTable[16];
extern GenericPtrList* activeCollisionPolyBoxes;
extern GenericPtrList* activeCollisionBoxes;

void ScanForOverlaps(const VECTOR* pMins, const VECTOR* pMaxs, const CollisionBox* colBox, CollisionOverlaps* overlaps);
bool CanPlayerStep(const VECTOR* position);
void SimulatePlayerMovementCollision();
void SortAndResolveOverlaps(const size_t numEntries);
void ResolveOverlaps(const PhysicsResolutionEntry* table, size_t numEntries);

void InitPhysicsLists();
void RegisterColPolyBox(StaticCollisionPolyBox* scpolybox);
void RegisterCollisionBox(CollisionBox* colBox);

#endif
