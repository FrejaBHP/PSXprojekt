#ifndef __CD_H
#define __CD_H

#include <libcd.h>
#include <types.h>

#define CD_SECTOR_SIZE 2048
// Converting bytes to sectors SECTOR_SIZE is defined in words, aka int
#define BtoS(len)((len + CD_SECTOR_SIZE - 1) / CD_SECTOR_SIZE)

extern u_char debugCtrlResult[8];
extern CdlLOC location[100];
extern CdlLOC pauseLocation;
extern CdlLOC relpauseLocation;

extern int numTOC;
extern int cdStatus;
extern int trackToPlay;

void LoadSoundFromCD();
void CDPrepareMusic();

void CDPlayTrack(long trackNum);
void CDTogglePause();

void CDIncrementTrack();

#endif
