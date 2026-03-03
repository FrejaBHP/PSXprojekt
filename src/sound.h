#ifndef __SOUND_H
#define __SOUND_H

#include <libspu.h>
#include <types.h>

extern u_char thats_all_start[];
extern u_char jump_start[];

void InitSound();
void PlaySoundJump();
void PlaySoundFolks();

#endif
