#include "physics.h"

PhysicsResolutionEntry PhysicsResolutionTable[16] = { 0 };

void ResolveOverlaps(const PhysicsResolutionEntry* table, const size_t tableSize) {
    // Sort if X or Z resolution should be done first based on (possibly) player velocity
    // Move resolution logic to this function
    // Do a second ScanForOverlaps on the i > 0 object in this function's expected for loop, to check if the previous plane resolution avoids a Y overlap
    // Otherwise resolve as normal
}