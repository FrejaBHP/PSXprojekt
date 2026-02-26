#include "graphics.h"
#include "player.h"

DB db[2] = { 0 };
DB* cdb = 0;
size_t cdbIndex = 0;
u_char* primPtr = 0;

MATRIX globalRenderTransform = { 0 };

TIM_IMAGE woodPanel_tim;
TIM_IMAGE woodDoor_tim;
TIM_IMAGE cobble_tim;
TIM_IMAGE dlv_metalpanel_tim;
TIM_IMAGE dlv_slate_tim;
TIM_IMAGE dlv_stonebrick_tim;
TIM_IMAGE goldCoin_tim;
TIM_IMAGE grass_tim;
TIM_IMAGE dirt_tim;
TIM_IMAGE grassydirt_tim;

UVRect woodPanel_UVR = { 0, 0, 63, 127 };
UVRect woodDoor_UVR = { 64, 0, 63, 127 };
UVRect cobble_UVR = { 0, 128, 127, 127 };
UVRect dlv_metalpanel_UVR = { 128, 0, 127, 127 };
UVRect dlv_slate_UVR = { 0, 128, 63, 63 };
UVRect dlv_stonebrick_UVR = { 0, 0, 127, 127 };
UVRect grass_UVR = { 0, 192, 63, 63 };
UVRect dirt_UVR = { 64, 128, 63, 63 };
UVRect grassydirt_UVR = { 64, 192, 63, 63 };

UVRect goldCoin_UVR = { 0, 128, 31, 31 };

MATRIX identity = {
	{
		{ 4096, 0, 0 },
		{ 0, 4096, 0 },
		{ 0, 0, 4096 }
	},
	{ 0, 0, 0 }
};

void InvertMatrix(MATRIX *a, MATRIX *b) {
	b->m[0][0] = a->m[0][0];
	b->m[0][1] = a->m[1][0];
	b->m[0][2] = a->m[2][0];
	b->m[1][0] = a->m[0][1];
	b->m[1][1] = a->m[1][1];
	b->m[1][2] = a->m[2][1];
	b->m[2][0] = a->m[0][2];
	b->m[2][1] = a->m[1][2];
	b->m[2][2] = a->m[2][2];
}

void LoadTexture(u_long* tim, TIM_IMAGE* tparam) {      // This part is from Lameguy64's tutorial series : lameguy64.net/svn/pstutorials/chapter1/3-textures.html login/pw: annoyingmous
    OpenTIM(tim);                                       // Open the tim binary data, feed it the address of the data in memory
    ReadTIM(tparam);                                    // This read the header of the TIM data and sets the corresponding members of the TIM_IMAGE structure
    
    LoadImage(tparam->prect, tparam->paddr);            // Transfer the data from memory to VRAM at position prect.x, prect.y
    DrawSync(0);                                        // Wait for the drawing to end
    
    if (tparam->mode & 0x8) { // check 4th bit          // If 4th bit == 1, TIM has a CLUT
        LoadImage(tparam->crect, tparam->caddr);        // Load it to VRAM at position crect.x, crect.y
        DrawSync(0);                                    // Wait for drawing to end
    }
}

void DebugPrintMatrix(MATRIX* matrix, char descriptor) {
    FntPrint("%cM0: %04d, %04d, %04d\n", descriptor, matrix->m[0][0], matrix->m[0][1], matrix->m[0][2]);
    FntPrint("%cM1: %04d, %04d, %04d\n", descriptor, matrix->m[1][0], matrix->m[1][1], matrix->m[1][2]);
    FntPrint("%cM2: %04d, %04d, %04d\n", descriptor, matrix->m[2][0], matrix->m[2][1], matrix->m[2][2]);
    FntPrint("%cT0: %04d, %04d, %04d\n\n", descriptor, matrix->t[0], matrix->t[1], matrix->t[2]);
}

void InitGraphics() {
    RECT clearRect;

    SetDispMask(0);

    // Initialises drawing engine (param = 0 -> complete reset)
    ResetGraph(0);

    // Initialises the Geometry Transformation Engine (GTE)
    InitGeom();

    // Set graphics debugging level
    // 0 = No checking (fastest)
    // 1 = Checks vertices and drawn primitives
    // 2 = Same as above but dumps them instead
    SetGraphDebug(0);

    SetDefDrawEnv(&db[0].draw, 0, 0, RENDERX, 240);
    SetDefDrawEnv(&db[1].draw, 0, 256, RENDERX, 240);
    SetDefDispEnv(&db[0].disp, 0, 256, RENDERX, 240);
    SetDefDispEnv(&db[1].disp, 0, 0, RENDERX, 240);

    db[0].draw.ofs[1] = 0;
    db[1].draw.ofs[1] = 256;
    //db[0].draw.dfe = 1;
    //db[1].draw.dfe = 1;
    db[0].draw.isbg = 1;
    db[1].draw.isbg = 1;
    db[0].draw.dtd = 1;
    db[1].draw.dtd = 1;

    setRGB0(&db[0].draw, 128, 128, 255);
    setRGB0(&db[1].draw, 128, 128, 255);

    setRECT(&clearRect, 0, 0, 1024, 512);
    ClearImage(&clearRect, 0, 0, 0);

    // Initialises and allows use of debug text
    FntLoad(960, 256);
    SetDumpFnt(FntOpen(8, 8, 256, 192, 0, 512));
   
    gte_SetGeomOffset(RENDERX / 2, RENDERY / 2);
    //gte_SetGeomScreen(341);
    gte_SetGeomScreen(RENDERX / 2);

    PutDrawEnv(&db[0].draw);
    PutDispEnv(&db[0].disp);

    cdb = &db[0];
    primPtr = cdb->primBuffer;
    ClearOTagR(db[0].ot, OTSIZE);
    ClearOTagR(db[1].ot, OTSIZE);

    DrawSync(0);

    LoadTexture(woodPanel_start, &woodPanel_tim);
    LoadTexture(woodDoor_start, &woodDoor_tim);
    LoadTexture(cobble_start, &cobble_tim);
    LoadTexture(dlv_metalpanel_start, &dlv_metalpanel_tim);
    LoadTexture(dlv_slate_start, &dlv_slate_tim);
    LoadTexture(dlv_stonebrick_start, &dlv_stonebrick_tim);
    LoadTexture(grass_start, &grass_tim);
    LoadTexture(dirt_start, &dirt_tim);
    LoadTexture(grassydirt_start, &grassydirt_tim);

    LoadTexture(goldCoin_start, &goldCoin_tim);

    // Actually display the things on screen
    SetDispMask(1);
}

void DrawFrame() {
    // Debug print stuff

    //DebugPrintMatrix(&player->cameraPtr->transform, 'C');
    //DebugPrintMatrix(&player->poly.obj.transform, 'P');

    //FntPrint("PV : %06d, %06d, %06d\n", player->poly.obj.velocity.vx, player->poly.obj.velocity.vy, player->poly.obj.velocity.vz);
    //FntPrint("PP : %06d, %06d, %06d\n", player->poly.obj.position.vx, player->poly.obj.position.vy, player->poly.obj.position.vz);

    // Wait for previous frame to have finished drawing if needed
    DrawSync(0);

    // Waits for VBLANK (param = 0 -> waits for generated vertical sync)
    VSync(0);

    PutDispEnv(&cdb->disp);
    PutDrawEnv(&cdb->draw);

    // Draw from ordering table
    DrawOTag(&cdb->ot[OTSIZE - 1]);
    
    // Draw debug text set in SetDumpFnt with value -1
    FntFlush(-1);

    // Swap used buffer
    cdb = (cdb == &db[0]) ? &db[1] : &db[0];
    cdbIndex = (cdbIndex == 0) ? 1 : 0;
    primPtr = cdb->primBuffer;

    // Initialises a linked list for OT / clears (zeroes?) OT for current frame in reverse order (faster)
    // "When an OT is initialized, the polygons are unlinked, and only then is a re-sort possible. 
    // Therefore, it is always necessary to initialize an OT prior to executing a sort." - Library Overview, 10-8
    ClearOTagR(cdb->ot, OTSIZE);
}
