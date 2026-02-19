#ifndef __GEOMETRY_H
#define __GEOMETRY_H

#include <stdlib.h>
#include <stdbool.h>
#include "objects.h"
#include "physics.h"
#include "graphics.h"


#define PLAYERHEIGHT 36
#define PLAYERWIDTHHALF 16

#define CUBESIZE 80
#define CUBEHALF CUBESIZE / 2
#define FLOORSIZE 128
#define FLOORHALF FLOORSIZE / 2
#define LONGFLOORLENGTH 320
#define LONGFLOORHALF LONGFLOORLENGTH / 2
#define COLBOXHEIGHT 12
#define COLBOXHALFWIDTH 32

#define WALLHEIGHT 128
#define WALLHALF 64
#define DOORHALF 32


extern PolyData coinPolyData;

extern SVECTOR colBoxVertices[];
extern SVECTOR playerBoxVertices[];
extern SVECTOR cubeVertices[];
extern SVECTOR tinyHouseVertices[];
extern SVECTOR tinyBoxVertices[];
extern SVECTOR boxVertices[];
extern SVECTOR platformVertices[];
extern SVECTOR tWallVertices[];
extern SVECTOR tDoorVertices[];

extern long cubeIndices[];
extern long tubeIndices[];

extern SVECTOR floorVertices[];
extern SVECTOR longFloorVertices[];
extern SVECTOR tiledHalfPanelVertices[];
extern SVECTOR tiledPanelVertices[];
extern SVECTOR collectibleVertices[];

extern long reverseWindingIndices[];
extern long windingIndices[];

void OrderThing(long* otz, int dp);

void CreatePlayer(CVECTOR* col);
PolyObject* CreatePolyObjectF4(long posX, long posY, long posZ, short rotX, short rotY, short rotZ, ushort plen, ushort psides, SVECTOR* vertPtr, long* indPtr, enum DrawPriority drprio, bool coll, int collH, int collW, bool fixed, CVECTOR* col);
POLY_FT4* CreateTexturedPolygon4(TIM_IMAGE* tim, u_char u0, u_char v0, u_char u1, u_char v1);
PolyData SetupPolyData(TIM_IMAGE* tim, u_char u0, u_char v0, u_char u1, u_char v1);
PolyData SetupPolyDataRGB(TIM_IMAGE* tim, u_char u0, u_char v0, u_char u1, u_char v1, u_char r, u_char g, u_char b);

TexturedPolyObject* CreateTexturedPolyObjectFT4(
    long posX, long posY, long posZ, 
    short rotX, short rotY, short rotZ, 
    ushort plen, ushort psides, SVECTOR* vertPtr, long* indPtr, 
    enum DrawPriority drprio, 
    bool coll, int collH, int collW, bool fixed, 
    TIM_IMAGE* tim, 
    u_char u0, u_char v0, u_char uvwidth, u_char uvheight, 
    bool repeating,
    u_char twx, u_char twy, u_char tww, u_char twh);

TiledTexturedPolyObject* CreateTiledTexturedPolyObjectFT4(
    long posX, long posY, long posZ, 
    short rotX, short rotY, short rotZ, 
    long* indPtr, 
    u_char segX, u_char segY, u_char segZ,
    u_char tileX, u_char tileY, u_char tileZ,
    enum DrawPriority drprio, 
    TIM_IMAGE* tim, 
    u_char u0, u_char v0, u_char uvwidth, u_char uvheight);

TiledTexturedPolyObject* CreateTiledTexturedPolyObjectFT4UVRect(
    long posX, long posY, long posZ, 
    short rotX, short rotY, short rotZ, 
    long* indPtr, 
    u_char segX, u_char segY, u_char segZ,
    u_char tileX, u_char tileY, u_char tileZ,
    enum DrawPriority drprio, 
    TIM_IMAGE* tim, 
    UVRect* uvRect);

StaticCollisionPolyBox* CreateCollisionPolyBox(
    long posX, long posY, long posZ,
    short rotX, short rotY, short rotZ,
    SVECTOR* vertPtr,
    PolyData pd0, PolyData pd1, PolyData pd2,
    PolyData pd3, PolyData pd4, PolyData pd5);

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
void AddTiledPolyFT(TiledTexturedPolyObject* ttpobj);
void AddStaticPolyBox(StaticCollisionPolyBox* scpolybox);
void AddCollectible(CollectibleObject* cobj);

// Partially unused
void AddMultiPoly(TestTileMultiPoly* tmp, u_long* ot);

#endif
