#ifndef __GRAPHICS_H
#define __GRAPHICS_H

#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
#include <inline_n.h>
#include <gtemac.h>

#define OTSIZE 4096
#define RENDERX 320 // 512
#define RENDERY 240

// (Double) Buffer struct
typedef struct DB {
    DRAWENV draw;
    DISPENV disp;
    u_long ot[OTSIZE];
} DB;

extern DB db[2];
extern DB* cdb;

void InitGraphics();
void DrawFrame();

#endif
