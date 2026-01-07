#include "graphics.h"

DB db[2] = { 0 };
DB* cdb = 0;
DR_MODE* drModeList = 0;
//DR_MODE resetDRMODE;
//RECT resetRect = { 0, 0, 0, 0 };
u_char curdrModeIndex = 0;
u_char curTPage = 0;

MATRIX globalRenderTransform = { 0 };

TIM_IMAGE woodPanel_tim;
TIM_IMAGE woodDoor_tim;
TIM_IMAGE cobble_tim;

void LoadTexture(u_long* tim, TIM_IMAGE* tparam) {     // This part is from Lameguy64's tutorial series : lameguy64.net/svn/pstutorials/chapter1/3-textures.html login/pw: annoyingmous
    OpenTIM(tim);                                   // Open the tim binary data, feed it the address of the data in memory
    ReadTIM(tparam);                                // This read the header of the TIM data and sets the corresponding members of the TIM_IMAGE structure
    
    LoadImage(tparam->prect, tparam->paddr);        // Transfer the data from memory to VRAM at position prect.x, prect.y
    DrawSync(0);                                    // Wait for the drawing to end
    
    if (tparam->mode & 0x8) { // check 4th bit       // If 4th bit == 1, TIM has a CLUT
        LoadImage(tparam->crect, tparam->caddr);    // Load it to VRAM at position crect.x, crect.y
        DrawSync(0);                                // Wait for drawing to end
    }
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

    // Set drawenv defs
    /*
    SetDefDrawEnv(&db[0].draw, 0, 8, RENDERX, 224);
    SetDefDrawEnv(&db[1].draw, 0, 248, RENDERX, 224);
    SetDefDispEnv(&db[0].disp, 0, 240, RENDERX, 240);
    SetDefDispEnv(&db[1].disp, 0, 0, RENDERX, 240);
    */

    SetDefDrawEnv(&db[0].draw, 0, 0, RENDERX, 240);
    SetDefDrawEnv(&db[1].draw, 0, 256, RENDERX, 240);
    SetDefDispEnv(&db[0].disp, 0, 256, RENDERX, 240);
    SetDefDispEnv(&db[1].disp, 0, 0, RENDERX, 240);

    db[0].draw.ofs[1] = 0;
    db[1].draw.ofs[1] = 256;
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

    DrawSync(0);

    // Actually display the things on screen
    SetDispMask(1);

    //setDrawMode(&resetDRMODE, 0, 1, 0, &resetRect);

    LoadTexture(woodPanel_start, &woodPanel_tim);
    LoadTexture(woodDoor_start, &woodDoor_tim);
    LoadTexture(cobble_start, &cobble_tim);
}

void DrawFrame() {
    //FntPrint("Buffer: %d\n", usedBuffer);
    //FntPrint("B0 Offset: %d %d\n", db[0].draw.ofs[0], db[0].draw.ofs[1]);
    //FntPrint("B1 Offset: %d %d\n", db[1].draw.ofs[0], db[1].draw.ofs[1]);

    //FntPrint("Status: %x\n", pad0.status);
    //FntPrint("Type: %x\n", pad0.type);
    //FntPrint("Buttons: %04x\n", pad0.buttons);
    //FntPrint("LastInput: %04x\n", lastInput);
    //FntPrint("Stick L XY: (%02x, %02x)\n", pad0.leftstick.x, pad0.leftstick.y);
    //FntPrint("Stick R XY: (%02x, %02x)\n\n", pad0.rightstick.x, pad0.rightstick.y);

    /*
    FntPrint("CM0: %04d, %04d, %04d\n", player->cameraPtr->transform.m[0][0], player->cameraPtr->transform.m[0][1], player->cameraPtr->transform.m[0][2]);
    FntPrint("CM1: %04d, %04d, %04d\n", player->cameraPtr->transform.m[1][0], player->cameraPtr->transform.m[1][1], player->cameraPtr->transform.m[1][2]);
    FntPrint("CM2: %04d, %04d, %04d\n", player->cameraPtr->transform.m[2][0], player->cameraPtr->transform.m[2][1], player->cameraPtr->transform.m[2][2]);
    FntPrint("CT0: %04d, %04d, %04d\n\n", player->cameraPtr->transform.t[0], player->cameraPtr->transform.t[1], player->cameraPtr->transform.t[2]);
    */

    //FntPrint("PM0: %04d, %04d, %04d\n", player->poly.obj.transform.m[0][0], player->poly.obj.transform.m[0][1], player->poly.obj.transform.m[0][2]);
    //FntPrint("PM1: %04d, %04d, %04d\n", player->poly.obj.transform.m[1][0], player->poly.obj.transform.m[1][1], player->poly.obj.transform.m[1][2]);
    //FntPrint("PM2: %04d, %04d, %04d\n", player->poly.obj.transform.m[2][0], player->poly.obj.transform.m[2][1], player->poly.obj.transform.m[2][2]);
    //FntPrint("PV : %06d, %06d, %06d\n", player->poly.obj.velocity.vx, player->poly.obj.velocity.vy, player->poly.obj.velocity.vz);
    //FntPrint("PP : %06d, %06d, %06d\n", player->poly.obj.position.vx, player->poly.obj.position.vy, player->poly.obj.position.vz);
    //FntPrint("PT0: %06d, %06d, %06d\n\n", player->poly.obj.transform.t[0], player->poly.obj.transform.t[1], player->poly.obj.transform.t[2]);
    
    //FntPrint("HDif: %d\n", heightDif);
    //FntPrint("Space: %d\n", occupiesSameSpace);

    // Wait for previous frame to have finished drawing if needed
    DrawSync(0);

    // Waits for VBLANK (param = 0 -> waits for generated vertical sync)
    VSync(0);

    PutDispEnv(&cdb->disp);
    PutDrawEnv(&cdb->draw);

    // Draw from ordering table
    DrawOTag(&cdb->ot[OTSIZE - 1]);
    curdrModeIndex = 0;
    
    // Draw debug text set in SetDumpFnt with value -1
    FntFlush(-1);
}
