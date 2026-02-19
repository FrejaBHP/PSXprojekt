#ifndef __GEOMETRY_H
#define __GEOMETRY_H

#include <stdlib.h>
#include <stdbool.h>
#include "objects.h"
#include "physics.h"
#include "graphics.h"

#define PLAYERHEIGHT 36
#define PLAYERWIDTHHALF 16


extern PolyData coinPolyData;

extern SVECTOR playerBoxVertices[];
extern SVECTOR collectibleVertices[];

extern long cubeIndices[];
extern long windingIndices[];
extern long reverseWindingIndices[];

void OrderThing(long* otz, int dp);

PolyData SetupPolyData(TIM_IMAGE* tim, u_char u0, u_char v0, u_char u1, u_char v1);
PolyData SetupPolyDataRGB(TIM_IMAGE* tim, u_char u0, u_char v0, u_char u1, u_char v1, u_char r, u_char g, u_char b);
PolyData SetupPolyDataUVRect(TIM_IMAGE* tim, UVRect* uvRect);

void CreatePlayer(CVECTOR* col);
PolyObject* CreatePolyObjectF4(long posX, long posY, long posZ, short rotX, short rotY, short rotZ, ushort plen, ushort psides, SVECTOR* vertPtr, long* indPtr, enum DrawPriority drprio, bool coll, int collH, int collW, bool fixed, CVECTOR* col);
POLY_FT4* CreateTexturedPolygon4(TIM_IMAGE* tim, u_char u0, u_char v0, u_char u1, u_char v1);

StaticWorldGeometry* CreateStaticWorldGeometry(
    GenericPtrList* list,
    short posX, short posY, short posZ, 
    long* indPtr, 
    u_char segX, u_char segY, u_char segZ,
    u_char tileX, u_char tileY, u_char tileZ,
    enum DrawPriority drprio, 
    TIM_IMAGE* tim, u_char u0, u_char v0, u_char uvwidth, u_char uvheight);

StaticWorldGeometry* CreateStaticWorldGeometryUVRect(
    GenericPtrList* list,
    short posX, short posY, short posZ, 
    long* indPtr, 
    u_char segX, u_char segY, u_char segZ,
    u_char tileX, u_char tileY, u_char tileZ,
    enum DrawPriority drprio, 
    TIM_IMAGE* tim, UVRect* uvRect);

StaticWorldPolyBox* CreateStaticWorldPolyBox(
    GenericPtrList* pbList, GenericPtrList* colList,
    short posX, short posY, short posZ,
    short lenX, short lenY, short lenZ,
    enum DrawPriority drprio,
    size_t polyMask,
    PolyData* pd);

CollectibleObject* CreateCollectibleObject(long posX, long posY, long posZ, enum CollectibleType cType);

TestTileMultiPoly* CreateTestMultiPoly(
    long posX, long posY, long posZ, 
    short rotX, short rotY, short rotZ, 
    u_char repeats, u_char subdivs, bool reverseOrder,
    u_char rx, u_char ry, u_char rz,
    u_char width, u_char height, u_char depth,
    TIM_IMAGE* tim, 
    u_char u0, u_char v0, u_char uvwidth, u_char uvheight);


void AddPolyF(PolyObject* pobj);
void AddPolyFT(TexturedPolyObject* tpobj);

void AddStaticWorldGeometry(StaticWorldGeometry* swg);
void AddStaticWorldPolyBox(StaticWorldPolyBox* swpb);

void AddCollectible(CollectibleObject* cobj);

// Partially unused
void AddMultiPoly(TestTileMultiPoly* tmp, u_long* ot);

#endif
