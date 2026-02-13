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

#define OTSIZE 1024
#define RENDERX 320 // 512
#define RENDERY 240

extern u_long woodPanel_start[];
extern u_long woodDoor_start[];
extern u_long cobble_start[];
extern u_long dlv_metalpanel_start[];
extern u_long dlv_slate_start[];
extern u_long dlv_stonebrick_start[];
extern u_long goldCoin_start[];

extern TIM_IMAGE woodPanel_tim;
extern TIM_IMAGE woodDoor_tim;
extern TIM_IMAGE cobble_tim;
extern TIM_IMAGE dlv_metalpanel_tim;
extern TIM_IMAGE dlv_slate_tim;
extern TIM_IMAGE dlv_stonebrick_tim;
extern TIM_IMAGE goldCoin_tim;

// (Double) Buffer struct
typedef struct DB {
    DRAWENV draw;
    DISPENV disp;
    u_long ot[OTSIZE];
	u_char primBuffer[32768];
} DB;

extern DB db[2];
extern DB* cdb;
extern size_t cdbIndex;
extern u_char* primPtr;

extern MATRIX globalRenderTransform;
extern MATRIX identity;

void InvertMatrix(MATRIX *a, MATRIX *b);
void LoadTexture(u_long* tim, TIM_IMAGE* tparam);
void InitGraphics();
void DrawFrame();
void DebugPrintMatrix(MATRIX* matrix, char descriptor);

#endif
