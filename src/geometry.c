#include "geometry.h"
#include "player.h"

PolyData coinPolyData = { 0 };

SVECTOR playerBoxVertices[] = {
    { -PLAYERWIDTHHALF, -PLAYERHEIGHT, -PLAYERWIDTHHALF, 0 }, {  PLAYERWIDTHHALF, -PLAYERHEIGHT, -PLAYERWIDTHHALF, 0 },
    {  PLAYERWIDTHHALF, 0, -PLAYERWIDTHHALF, 0 }, { -PLAYERWIDTHHALF, 0, -PLAYERWIDTHHALF, 0 },
    { -PLAYERWIDTHHALF, -PLAYERHEIGHT,  PLAYERWIDTHHALF, 0 }, {  PLAYERWIDTHHALF, -PLAYERHEIGHT,  PLAYERWIDTHHALF, 0 },
    {  PLAYERWIDTHHALF, 0,  PLAYERWIDTHHALF, 0 }, { -PLAYERWIDTHHALF, 0,  PLAYERWIDTHHALF, 0 },
};

SVECTOR collectibleVertices[] = {
    { -16, -16, 0, 0 }, { 16, -16, 0, 0 },
    { 16, 16, 0, 0 },   { -16, 16, 0, 0 }
};

long cubeIndices[] = {
    // From camera starting position:
    0, 1, 2, 3, // Front
    1, 5, 6, 2, // Left
    5, 4, 7, 6, // Back
    4, 0, 3, 7, // Right
    4, 5, 1, 0, // Top
    6, 7, 3, 2  // Bottom
};

long windingIndices[] = {
    0, 1, 2, 3
};

long reverseWindingIndices[] = {
    1, 0, 3, 2
};


// Lazy and likely fragile attempt at influencing sorting order
void OrderThing(long* otz, int dp) {
    switch (dp) {
        case DRP_First:
            *otz = 0;
            break;

        case DRP_Highest:
            if (*otz - 16 > 0) {
                *otz -= 16;
            }
            else {
                *otz = 1;
            }
            break;

        case DRP_Higher:
            if (*otz - 8 > 0) {
                *otz -= 8;
            }
            else if (*otz > 2) {
                *otz = 2;
            }
            break;

        case DRP_High:
            if (*otz - 4 > 0) {
                *otz -= 4;
            }
            else if (*otz > 3) {
                *otz = 3;
            }
            break;

        case DRP_Low:
            if (*otz + 4 < (OTSIZE - 1)) {
                *otz += 4;
            }
            else if (*otz < (OTSIZE - 4)) {
                *otz = OTSIZE - 4;
            }
            break;

        case DRP_Lower:
            if (*otz + 8 < (OTSIZE - 1)) {
                *otz += 8;
            }
            else if (*otz < (OTSIZE - 3)) {
                *otz = OTSIZE - 3;
            }
            break;

        case DRP_Lowest:
            if (*otz + 16 < (OTSIZE - 1)) {
                *otz += 16;
            }
            else if (*otz < (OTSIZE - 2)) {
                *otz = OTSIZE - 2;
            }
            break;

        case DRP_Last:
            *otz = OTSIZE - 1;
            break;
        
        default:
            break;
    }
}

// ============== INITIALISERS/CONSTRUCTORS ==============

PolyData SetupPolyData(TIM_IMAGE* tim, u_char u0, u_char v0, u_char u1, u_char v1) {
    PolyData data = {
        .tpage = getTPage(tim->mode & 0x3, 0, tim->prect->x, tim->prect->y),
        .clut = getClut(tim->crect->x, tim->crect->y),
        .u0 = u0,   .v0 = v0,
        .um = u1,   .vm = v1,
        .r = 128,   .g = 128,   .b = 128
    };

    return data;
}

PolyData SetupPolyDataRGB(TIM_IMAGE* tim, u_char u0, u_char v0, u_char u1, u_char v1, u_char r, u_char g, u_char b) {
    PolyData data = {
        .tpage = getTPage(tim->mode & 0x3, 0, tim->prect->x, tim->prect->y),
        .clut = getClut(tim->crect->x, tim->crect->y),
        .u0 = u0,   .v0 = v0,
        .um = u1,   .vm = v1,
        .r = r,     .g = g,     .b = b
    };

    return data;
}

PolyData SetupPolyDataUVRect(TIM_IMAGE* tim, UVRect* uvRect) {
    PolyData data = {
        .tpage = getTPage(tim->mode & 0x3, 0, tim->prect->x, tim->prect->y),
        .clut = getClut(tim->crect->x, tim->crect->y),
        .u0 = uvRect->u0,   .v0 = uvRect->v0,
        .um = uvRect->um,   .vm = uvRect->vm,
        .r = 128,   .g = 128,   .b = 128
    };

    return data;
}


void CreatePlayer(CVECTOR* col) {
    player = calloc(1, sizeof(PlayerObject));

    CameraObject* camera = calloc(1, sizeof(CameraObject));
    PolyData* polyData = calloc(6, sizeof(PolyData));
    VECTOR pos = { 128, 0, 32 };

    if (player != NULL) {
        setVector(&player->poly.obj.position, pos.vx * ONE, pos.vy * ONE, pos.vz * ONE);
        player->poly.polyLength = 6;
        player->poly.polySides = 4;
        player->poly.verticesPtr = playerBoxVertices;
        player->poly.indicesPtr = cubeIndices;
        player->poly.polyDataPtr = polyData;
        player->poly.drPrio = DRP_Higher;
        player->poly.collides = false;
        player->poly.boxHeight = PLAYERHEIGHT;
        player->poly.boxWidth = PLAYERWIDTHHALF * 2;
        player->poly.obj.maxSpeed = 5 * ONE;
        player->poly.obj.isStatic = false;
        //player->poly.add = &AddPolyF;

        if (camera != NULL) {
            player->cameraPtr = camera;
        }

        for (size_t i = 0; i < 6; ++i) {
            polyData[i].r = col[i].r;
            polyData[i].g = col[i].g;
            polyData[i].b = col[i].b;
        }

        RotMatrix_gte(&player->poly.obj.rotation, &player->poly.obj.transform);
        TransMatrix(&player->poly.obj.transform, &pos);
    }
}

PolyObject* CreatePolyObjectF4(long posX, long posY, long posZ, short rotX, short rotY, short rotZ, ushort plen, ushort psides, SVECTOR* vertPtr, long* indPtr, enum DrawPriority drprio, bool coll, int collH, int collW, bool fixed, CVECTOR* col) {
    PolyObject* pobj = calloc(1, sizeof(PolyObject));
    PolyData* polyData = calloc(plen, sizeof(PolyData));
    VECTOR pos = { posX, posY, posZ };

    if (pobj != NULL) {
        setVector(&pobj->obj.position, pos.vx * ONE, pos.vy * ONE, pos.vz * ONE);
        setVector(&pobj->obj.rotation, rotX, rotY, rotZ);
        pobj->polyLength = plen;
        pobj->polySides = psides;
        pobj->verticesPtr = vertPtr;
        pobj->indicesPtr = indPtr;
        pobj->polyDataPtr = polyData;
        pobj->drPrio = drprio;
        pobj->collides = coll;
        pobj->boxHeight = collH;
        pobj->boxWidth = collW;
        pobj->obj.isStatic = fixed;
        //platform->add = &AddPolyF;

        for (size_t i = 0; i < plen; i++) {
            polyData[i].r = col[i].r;
            polyData[i].g = col[i].g;
            polyData[i].b = col[i].b;
        }
        
        RotMatrix_gte(&pobj->obj.rotation, &pobj->obj.transform);
        TransMatrix(&pobj->obj.transform, &pos);
    }

    return pobj;
}

POLY_FT4* CreateTexturedPolygon4(TIM_IMAGE* tim, u_char u0, u_char v0, u_char u1, u_char v1) {
    POLY_FT4* poly = calloc(1, sizeof(POLY_FT4));
    setPolyFT4(poly);

    poly->tpage = getTPage(tim->mode & 0x3, 0, tim->prect->x, tim->prect->y);
    poly->clut = getClut(tim->crect->x, tim->crect->y);
    setRGB0(poly, 128, 128, 128);
    setUVWH(poly, u0, v0, u1, v1);

    return poly;
}

StaticWorldGeometry* CreateStaticWorldGeometry(
    GenericPtrList* list,
    short posX, short posY, short posZ, 
    long* indPtr, 
    u_char segX, u_char segY, u_char segZ,
    u_char tileX, u_char tileY, u_char tileZ,
    enum DrawPriority drprio, 
    TIM_IMAGE* tim, u_char u0, u_char v0, u_char uvwidth, u_char uvheight) {
    
    StaticWorldGeometry* swg = calloc(1, sizeof(StaticWorldGeometry));

    if (swg != NULL) {
        swg->indicesPtr = indPtr;
        swg->drPrio = drprio;
        swg->tim = tim;

        swg->totalPolys = tileX * tileY * tileZ;

        PolyData* polyData = malloc(sizeof(PolyData));
        *polyData = SetupPolyData(tim, u0, v0, uvwidth, uvheight);

        // Can be heavily optimised by reusing vertices. Check notebook!
        SVECTOR* tiledVerts = calloc(swg->totalPolys * 4, sizeof(SVECTOR));
        
        swg->polyDataPtr = polyData;
        swg->verticesPtr = tiledVerts;

        size_t vertIndex = 0;
        
        if (segX != 0 && segZ == 0) {
            for (size_t y = 0; y < tileY; y++) {
                for (size_t x = 0; x < tileX; x++) {
                    setVector(&tiledVerts[vertIndex + 0], posX + segX * x,             posY - (-segY * y) - segY,  posZ + 0);
                    setVector(&tiledVerts[vertIndex + 1], posX + (segX * x) + segX,    posY - (-segY * y) - segY,  posZ + 0);
                    setVector(&tiledVerts[vertIndex + 2], posX + (segX * x) + segX,    posY - -segY * y,           posZ + 0);
                    setVector(&tiledVerts[vertIndex + 3], posX + segX * x,             posY - -segY * y,           posZ + 0);

                    vertIndex += 4;
                }
            }
        }
        else if (segZ != 0 && segX == 0) {
            for (size_t y = 0; y < tileY; y++) {
                for (size_t z = 0; z < tileZ; z++) {
                    setVector(&tiledVerts[vertIndex + 0], posX + 0, posY - (-segY * y) - segY,    posZ + segZ * z);
                    setVector(&tiledVerts[vertIndex + 1], posX + 0, posY - (-segY * y) - segY,    posZ + (segZ * z) + segZ);
                    setVector(&tiledVerts[vertIndex + 2], posX + 0, posY - -segY * y,             posZ + (segZ * z) + segZ);
                    setVector(&tiledVerts[vertIndex + 3], posX + 0, posY - -segY * y,             posZ + segZ * z);

                    vertIndex += 4;
                }
            }
        }
        else if (segY == 0 && segX != 0 && segZ != 0) {
            for (size_t z = 0; z < tileZ; z++) {
                for (size_t x = 0; x < tileX; x++) {
                    setVector(&tiledVerts[vertIndex + 0], posX + segX * x,             posY - 0,  posZ + (segZ * z) + segZ);
                    setVector(&tiledVerts[vertIndex + 1], posX + (segX * x) + segX,    posY - 0,  posZ + (segZ * z) + segZ);
                    setVector(&tiledVerts[vertIndex + 2], posX + (segX * x) + segX,    posY - 0,  posZ + segZ * z);
                    setVector(&tiledVerts[vertIndex + 3], posX + segX * x,             posY - 0,  posZ + segZ * z);

                    vertIndex += 4;
                }
            }
        }
    }

    AddItemToGenericPtrList(&list, swg);

    return swg;
}

StaticWorldGeometry* CreateStaticWorldGeometryUVRect(
    GenericPtrList* list,
    short posX, short posY, short posZ, 
    long* indPtr, 
    u_char segX, u_char segY, u_char segZ,
    u_char tileX, u_char tileY, u_char tileZ,
    enum DrawPriority drprio, 
    TIM_IMAGE* tim, UVRect* uvRect) {

    return CreateStaticWorldGeometry(
        list, posX, posY, posZ, indPtr,
        segX, segY, segZ, tileX, tileY, tileZ,
        drprio, tim, uvRect->u0, uvRect->v0, uvRect->um, uvRect->vm
    );
}

// Mask, in order (least sig to most sig bit, from starting camera position): Front, Right, Back, Left, Top, Bottom
StaticWorldPolyBox* CreateStaticWorldPolyBox(
    GenericPtrList* pbList, GenericPtrList* colList,
    short posX, short posY, short posZ,
    short lenX, short lenY, short lenZ,
    short padX, short padY, short padZ,
    enum DrawPriority drprio,
    size_t polyMask,
    PolyData* pd) {

    StaticWorldPolyBox* swpb = malloc(sizeof(StaticWorldPolyBox));
    
    size_t numOfPolys = 0;
    for (size_t i = 0; i < 6; i++) {
        if (polyMask & (1 << i)) {
            numOfPolys++;
        }
    }
    
    PolyData* polyData = malloc(sizeof(PolyData) * numOfPolys);
    for (size_t i = 0; i < numOfPolys; i++) {
        polyData[i] = pd[i];
    }

    setVector(&swpb->vertices[0],    posX,          posY - lenY,    posZ         );
    setVector(&swpb->vertices[1],    posX + lenX,   posY - lenY,    posZ         );
    setVector(&swpb->vertices[2],    posX + lenX,   posY,           posZ         );
    setVector(&swpb->vertices[3],    posX,          posY,           posZ         );
    setVector(&swpb->vertices[4],    posX,          posY - lenY,    posZ + lenZ  );
    setVector(&swpb->vertices[5],    posX + lenX,   posY - lenY,    posZ + lenZ  );
    setVector(&swpb->vertices[6],    posX + lenX,   posY,           posZ + lenZ  );
    setVector(&swpb->vertices[7],    posX,          posY,           posZ + lenZ  );

    swpb->polyMask = polyMask;
    swpb->polyDataPtr = polyData;
    swpb->drPrio = drprio;

    setVector(&swpb->colBox.gridPos, posX, posY + padY, posZ);
    setVector(&swpb->colBox.dimensions, lenX + padX, lenY + padY, lenZ + padZ);

    AddItemToGenericPtrList(&pbList, swpb);
    AddItemToGenericPtrList(&colList, &swpb->colBox);

    return swpb;
}

CollectibleObject* CreateCollectibleObject(long posX, long posY, long posZ, enum CollectibleType cType) {
    CollectibleObject* cobj = calloc(1, sizeof(CollectibleObject));
    VECTOR pos = { posX, posY, posZ };
    CVECTOR colour = { 128, 128, 128, 0 };

    if (cobj != NULL) {
        setVector(&cobj->position, pos.vx * ONE, pos.vy * ONE, pos.vz * ONE);
        cobj->cType = cType;

        RotMatrix_gte(&cobj->rotation, &cobj->transform);
        TransMatrix(&cobj->transform, &pos);
    }

    return cobj;
}

TestTileMultiPoly* CreateTestMultiPoly(
    long posX, long posY, long posZ, 
    short rotX, short rotY, short rotZ, 
    u_char repeats, u_char subdivs, bool reverseOrder,
    u_char rx, u_char ry, u_char rz,
    u_char width, u_char height, u_char depth,
    TIM_IMAGE* tim, 
    u_char u0, u_char v0, u_char uvwidth, u_char uvheight) {
    
    TestTileMultiPoly* tmp = calloc(1, sizeof(TestTileMultiPoly));

    SVECTOR* vertices = malloc(sizeof(SVECTOR) * 4);
    if (vertices != NULL) {
        if (width == 0) {
            setVector(&vertices[0], 0, -height, 0);
            setVector(&vertices[1], 0, -height, depth);
            setVector(&vertices[2], 0, 0, depth);
            setVector(&vertices[3], 0, 0, 0);
        }
        else if (height == 0) {
            //setVector(&vertices[0], width, 0, 0);
            //setVector(&vertices[1], width, 0, depth);
            //setVector(&vertices[2], 0, 0, depth);
            //setVector(&vertices[3], 0, 0, 0);

            setVector(&vertices[0], 0, 0, depth);
            setVector(&vertices[1], width, 0, depth);
            setVector(&vertices[2], width, 0, 0);
            setVector(&vertices[3], 0, 0, 0);
            
        }
        else {
            setVector(&vertices[0], 0, -height, 0);
            setVector(&vertices[1], width, -height, 0);
            setVector(&vertices[2], width, 0, 0);
            setVector(&vertices[3], 0, 0, 0);
        }
    }

    VECTOR pos = { posX, posY, posZ };
    CVECTOR colour = { 128, 128, 128, 0 };

    if (tmp != NULL) {
        setVector(&tmp->obj.position, pos.vx * ONE, pos.vy * ONE, pos.vz * ONE);
        setVector(&tmp->obj.rotation, rotX, rotY, rotZ);
        tmp->repeats = repeats;
        tmp->subdivs = subdivs;
        tmp->totalPolys = repeats * (subdivs * subdivs);
        tmp->width = width;
        tmp->height = height;
        tmp->depth = depth;
        tmp->tim = tim;
        tmp->reverseOrder = reverseOrder;
        tmp->verticesPtr = vertices;

        if (tmp->reverseOrder) {
            tmp->indicesPtr = reverseWindingIndices;
        }
        else {
            tmp->indicesPtr = windingIndices;
        }

        POLY_FT4* poly = calloc(tmp->totalPolys, sizeof(POLY_FT4));
        for (size_t i = 0; i < tmp->totalPolys; ++i) {
            setPolyFT4(&poly[i]);
            poly[i].tpage = getTPage(tim->mode & 0x3, 0, tim->prect->x, tim->prect->y);
            poly[i].clut = getClut(tim->crect->x, tim->crect->y);
            setRGB0(&poly[i], colour.r, colour.g, colour.b);
        }

        u_char utemp = u0;
        u_char vtemp = v0;
        ushort uvtemp = 0;

        if (subdivs > 1) {
            ushort polyCount = 0;
            u_char divuwidth = uvwidth / subdivs;
            u_char divvheight = uvheight / subdivs;
            
            for (size_t i = 0; i < subdivs; i++) {
                for (size_t j = 0; j < repeats * subdivs; j++) {
                    setUVWH(&poly[polyCount], utemp, vtemp, divuwidth, divvheight);
                    polyCount++;

                    uvtemp = utemp + divuwidth;
                    if ((uvtemp - u0) >= uvwidth || uvtemp > 256) {
                        utemp = u0;
                    }
                    else if (uvtemp == 256) {
                        utemp = 255;
                    }
                    else {
                        utemp += divuwidth;
                    }
                }

                uvtemp = vtemp + divvheight;
                if ((uvtemp - v0) >= uvheight || uvtemp > 256) {
                    vtemp = v0;
                }
                else if (uvtemp == 256) {
                    vtemp = 255;
                }
                else {
                    vtemp += divvheight;
                }

                utemp = u0;
            }
        }
        else {
            for (size_t i = 0; i < tmp->totalPolys; i++) {
                setUVWH(&poly[i], u0, v0, uvwidth, uvheight);
            }
        }

        tmp->polyPtr = poly;

        RotMatrix_gte(&tmp->obj.rotation, &tmp->obj.transform);
        TransMatrix(&tmp->obj.transform, &pos);
    }

    return tmp;
}



// ============== RENDERING ROUTINES ==============


void AddPolyF(PolyObject* pobj) {
    POLY_F4* poly;
    long p, otz, flg;
    int nclip;
    size_t colIndex = 0;

    if (pobj->polySides == 4) {
        for (size_t i = 0; i < (pobj->polyLength * pobj->polySides); i += pobj->polySides, ++poly, ++colIndex) {
            poly = (POLY_F4*)primPtr;

            gte_ldv3(&pobj->verticesPtr[pobj->indicesPtr[i + 0]], &pobj->verticesPtr[pobj->indicesPtr[i + 1]], &pobj->verticesPtr[pobj->indicesPtr[i + 3]]);
            gte_rtpt();
            gte_nclip();
            gte_stopz(&nclip);

            if (nclip <= 0) {
                continue;
            }

            setPolyF4(poly);
            setRGB0(poly, pobj->polyDataPtr[colIndex].r, pobj->polyDataPtr[colIndex].g, pobj->polyDataPtr[colIndex].b);

            gte_stsxy3(&poly->x0, &poly->x1, &poly->x2);
            gte_ldv0(&pobj->verticesPtr[pobj->indicesPtr[i + 2]]);
            gte_rtps();

            gte_avsz4();
            gte_stotz(&otz);
            gte_stsxy(&poly->x3);
            
            if ((otz > 0) && (otz < OTSIZE)) {
                OrderThing(&otz, pobj->drPrio);
                addPrim(cdb->ot[otz], poly);
            }

            primPtr += sizeof(POLY_F4);
        }
    }
    /*
    else if (pobj->polySides == 3) {
        POLY_F3* poly = (POLY_F3*)pobj->polyPtr;

        for (size_t i = 0; i < (pobj->polyLength * pobj->polySides); i += pobj->polySides, ++poly) {
            nclip = RotAverageNclip3(
                &pobj->verticesPtr[pobj->indicesPtr[i + 0]], &pobj->verticesPtr[pobj->indicesPtr[i + 1]],
                &pobj->verticesPtr[pobj->indicesPtr[i + 2]],
                (long*)&poly->x0, (long*)&poly->x1, (long*)&poly->x2, &p, &otz, &flg
            );

            if (nclip <= 0) {
                continue;
            }

            if ((otz > 0) && (otz < OTSIZE)) {
                OrderThing(&otz, pobj->drPrio);
                addPrim(&ot[otz], poly);
            }
        }
    }
    */
}

void AddPolyFT(TexturedPolyObject* tpobj) {
    POLY_FT4* poly;
    long p, otz, flg;
    int nclip;

    if (tpobj->polyObj.polySides == 4) {
        for (size_t i = 0; i < (tpobj->polyObj.polyLength * tpobj->polyObj.polySides); i += tpobj->polyObj.polySides, ++poly) {
            poly = (POLY_FT4*)primPtr;

            gte_ldv3(&tpobj->polyObj.verticesPtr[tpobj->polyObj.indicesPtr[i + 0]], &tpobj->polyObj.verticesPtr[tpobj->polyObj.indicesPtr[i + 1]], &tpobj->polyObj.verticesPtr[tpobj->polyObj.indicesPtr[i + 3]]);
            gte_rtpt();
            gte_nclip();
            gte_stopz(&nclip);

            if (nclip <= 0) {
                continue;
            }

            setPolyFT4(poly);
            poly->tpage = tpobj->polyObj.polyDataPtr->tpage;
            poly->clut = tpobj->polyObj.polyDataPtr->clut;
            setRGB0(poly, tpobj->polyObj.polyDataPtr->r, tpobj->polyObj.polyDataPtr->g, tpobj->polyObj.polyDataPtr->b);
            setUVWH(poly, tpobj->polyObj.polyDataPtr->u0, tpobj->polyObj.polyDataPtr->v0, tpobj->polyObj.polyDataPtr->um, tpobj->polyObj.polyDataPtr->vm);

            gte_stsxy3(&poly->x0, &poly->x1, &poly->x2);
            gte_ldv0(&tpobj->polyObj.verticesPtr[tpobj->polyObj.indicesPtr[i + 2]]);
            gte_rtps();

            gte_avsz4();
            gte_stotz(&otz);
            gte_stsxy(&poly->x3);
            
            if ((otz > 0) && (otz < OTSIZE)) {
                OrderThing(&otz, tpobj->polyObj.drPrio);
                addPrim(cdb->ot[otz], poly);
            }

            primPtr += sizeof(POLY_FT4);
        }
    }
    /*
    else if (tpobj->polyObj.polySides == 3) {
        POLY_FT3* poly = (POLY_FT3*)tpobj->polyObj.polyPtr;

        for (size_t i = 0; i < (tpobj->polyObj.polyLength * tpobj->polyObj.polySides); i += tpobj->polyObj.polySides, ++poly) {
            nclip = RotAverageNclip3(
                &tpobj->polyObj.verticesPtr[tpobj->polyObj.indicesPtr[i + 0]], &tpobj->polyObj.verticesPtr[tpobj->polyObj.indicesPtr[i + 1]],
                &tpobj->polyObj.verticesPtr[tpobj->polyObj.indicesPtr[i + 2]],
                (long*)&poly->x0, (long*)&poly->x1, (long*)&poly->x2, &p, &otz, &flg
            );

            if (nclip <= 0) {
                continue;
            }

            if ((otz > 0) && (otz < OTSIZE)) {
                OrderThing(&otz, tpobj->polyObj.drPrio);
                addPrim(&ot[otz], poly);
            }
        }
    }
    */
}

// Tiles polys side by side
void AddStaticWorldGeometry(StaticWorldGeometry* swg) {
    POLY_FT4* poly;
    long p, otz, flg;
    int nclip;

    for (size_t i = 0; i < swg->totalPolys * 4; i += 4, ++poly) {
        poly = (POLY_FT4*)primPtr;

        gte_ldv3(&swg->verticesPtr[i + swg->indicesPtr[0]], &swg->verticesPtr[i + swg->indicesPtr[1]], &swg->verticesPtr[i + swg->indicesPtr[3]]);
        gte_rtpt();
        gte_nclip();
        gte_stopz(&nclip);
        
        if (nclip <= 0) {
            continue;
        }

        setPolyFT4(poly);
        poly->tpage = swg->polyDataPtr->tpage;
        poly->clut = swg->polyDataPtr->clut;
        setRGB0(poly, swg->polyDataPtr->r, swg->polyDataPtr->g, swg->polyDataPtr->b);
        setUVWH(poly, swg->polyDataPtr->u0, swg->polyDataPtr->v0, swg->polyDataPtr->um, swg->polyDataPtr->vm);

        gte_stsxy3(&poly->x0, &poly->x1, &poly->x2);
        gte_ldv0(&swg->verticesPtr[i + swg->indicesPtr[2]]);
        gte_rtps();

        gte_avsz4();
        gte_stotz(&otz);
        gte_stsxy(&poly->x3);
        
        if ((otz > 0) && (otz < OTSIZE)) {
            OrderThing(&otz, swg->drPrio);
            addPrim(cdb->ot[otz], poly);
        }

        primPtr += sizeof(POLY_FT4);
    }
}

// Polygon box wrapped around collision box
void AddStaticWorldPolyBox(StaticWorldPolyBox* swpb) {
    POLY_FT4* poly;
    long p, otz, flg;
    int nclip;
    
    size_t polyIndex = 0;

    for (size_t i = 0; i < 6; ++i) {
        if (!(swpb->polyMask & (1 << i))) {
            continue;
        }

        poly = (POLY_FT4*)primPtr;

        gte_ldv3(&swpb->vertices[cubeIndices[(4 * i) + 0]], &swpb->vertices[cubeIndices[(4 * i) + 1]], &swpb->vertices[cubeIndices[(4 * i) + 3]]);
        gte_rtpt();
        gte_nclip();
        gte_stopz(&nclip);

        if (nclip <= 0) {
            polyIndex++;
            continue;
        }

        setPolyFT4(poly);
        poly->tpage = swpb->polyDataPtr[polyIndex].tpage;
        poly->clut = swpb->polyDataPtr[polyIndex].clut;
        setRGB0(poly, swpb->polyDataPtr[polyIndex].r, swpb->polyDataPtr[polyIndex].g, swpb->polyDataPtr[polyIndex].b);
        setUVWH(poly, swpb->polyDataPtr[polyIndex].u0, swpb->polyDataPtr[polyIndex].v0, swpb->polyDataPtr[polyIndex].um, swpb->polyDataPtr[polyIndex].vm);

        gte_stsxy3(&poly->x0, &poly->x1, &poly->x2);
        gte_ldv0(&swpb->vertices[cubeIndices[(4 * i) + 2]]);
        gte_rtps();

        gte_avsz4();
        gte_stotz(&otz);
        gte_stsxy(&poly->x3);
        
        if ((otz > 0) && (otz < OTSIZE)) {
            OrderThing(&otz, swpb->drPrio);
            addPrim(cdb->ot[otz], poly);
        }

        primPtr += sizeof(POLY_FT4);
        polyIndex++;
    }
}

void AddCollectible(CollectibleObject* cobj) {
    POLY_FT4* poly;
    long p, otz, flg;

    poly = (POLY_FT4*)primPtr;
    SVECTOR* verticesPtr;
    PolyData* polyData;

    if (cobj->cType == CT_Coin) {
        verticesPtr = collectibleVertices;
        polyData = &coinPolyData;
    }

    gte_ldv3(&verticesPtr[windingIndices[0]], &verticesPtr[windingIndices[1]], &verticesPtr[windingIndices[3]]);
    gte_rtpt();

    setPolyFT4(poly);
    poly->tpage = polyData->tpage;
    poly->clut = polyData->clut;
    setRGB0(poly, polyData->r, polyData->g, polyData->b);
    setUVWH(poly, polyData->u0, polyData->v0, polyData->um, polyData->vm);

    gte_stsxy3(&poly->x0, &poly->x1, &poly->x2);
    gte_ldv0(&verticesPtr[windingIndices[2]]);
    gte_rtps();

    gte_avsz4();
    gte_stotz(&otz);
    gte_stsxy(&poly->x3);
    
    if ((otz > 0) && (otz < OTSIZE)) {
        if ((otz - 1) > 0) {
            otz -= 1;
        }
        addPrim(cdb->ot[otz], poly);
    }

    primPtr += sizeof(POLY_FT4);
}

// Tiles and passively tessellates polys side by side (VERY WIP!!!)
void AddMultiPoly(TestTileMultiPoly* tmp, u_long* ot) {
    long p, otz, flg;
    int nclip;

    POLY_FT4* poly = (POLY_FT4*)tmp->polyPtr;

    u_char vertX = 0;
    u_char vertY = 0;
    u_char vertYinv = tmp->subdivs - 1;

    u_char sw = tmp->width / tmp->subdivs;
    u_char sh = tmp->height / tmp->subdivs;
    u_char sd = tmp->depth / tmp->subdivs;

    for (size_t i = 0; i < tmp->totalPolys; ++i, ++poly) {
        SVECTOR modVertices[4];

        if (tmp->depth == 0) {
            for (size_t v = 0; v < 4; v++) {
                modVertices[v] = tmp->verticesPtr[tmp->indicesPtr[v]];

                modVertices[v].vx /= tmp->subdivs;
                if (modVertices[v].vy == 0) {
                    modVertices[v].vy = -sh * vertYinv;
                }
                else {
                    modVertices[v].vy = -sh * (vertYinv + 1);
                }

                modVertices[v].vx += sw * vertX;
            }
        }
        else if (tmp->height == 0) {
            for (size_t v = 0; v < 4; v++) {
                modVertices[v] = tmp->verticesPtr[tmp->indicesPtr[v]];

                modVertices[v].vx /= tmp->subdivs;

                if (modVertices[v].vz == 0) {
                    modVertices[v].vz = sd * vertYinv;
                }
                else {
                    modVertices[v].vz = sd * (vertYinv + 1);
                }
                
                modVertices[v].vx += sw * vertX;
            }
        }

        if (vertX == (tmp->repeats * tmp->subdivs) - 1) {
            vertX = 0;
            vertY++;
            vertYinv--;
        }
        else {
            vertX++;
        }

        nclip = RotAverageNclip4(
            &modVertices[tmp->indicesPtr[0]], &modVertices[tmp->indicesPtr[1]],
            &modVertices[tmp->indicesPtr[2]], &modVertices[tmp->indicesPtr[3]],
            (long*)&poly->x0, (long*)&poly->x1, (long*)&poly->x3, (long*)&poly->x2, &p, &otz, &flg
        );
        
        if (nclip <= 0) {
            continue;
        }
        
        if ((otz > 0) && (otz < OTSIZE)) {
            //OrderThing(&otz, tpobj->polyObj.drPrio);
            addPrim(&ot[otz], poly);
        }
    }
}
