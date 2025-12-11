#ifndef __GRAPHICS_H
#define __GRAPHICS_H

#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
#include <inline_n.h>
#include <gtemac.h>

/* TIM ref
// Multi-purpose TIM image
typedef struct {
	u_long  mode;		// pixel mode
	RECT	*crect;		// CLUT rectangle on frame buffer
	u_long	*caddr;		// CLUT address on main memory
	RECT	*prect;		// texture image rectangle on frame buffer
	u_long	*paddr;		// texture image address on main memory
} TIM_IMAGE;
*/

#define OTSIZE 2048
#define RENDERX 320 // 512
#define RENDERY 240

extern u_long woodPanel_start[];
extern u_long woodPanel_end[];
extern u_long woodDoor_start[];
extern u_long woodDoor_end[];
extern u_long cobble_start[];
extern u_long cobble_end[];

extern TIM_IMAGE woodPanel_tim;
extern TIM_IMAGE woodDoor_tim;
extern TIM_IMAGE cobble_tim;

// (Double) Buffer struct
typedef struct DB {
    DRAWENV draw;
    DISPENV disp;
    u_long ot[OTSIZE];
} DB;

extern DB db[2];
extern DB* cdb;

void LoadTexture(u_long* tim, TIM_IMAGE* tparam);
void InitGraphics();
void DrawFrame();

#endif
