#ifndef __PHYSICS_H
#define __PHYSICS_H

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

void ResolveOverlaps(const PhysicsResolutionEntry* table, const size_t tableSize);

#endif