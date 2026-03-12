#include "cd.h"
#include <stdlib.h>
#include "sound.h"

static char* loadFile;

// libcd's CD file structure contains size, location and filename
CdlFILE file = { 0 };

CdlLOC location[100];
int numTOC = 0;

CdlFILTER filter = { 0 };

u_long* dataBuffer;

u_char debugCtrlResult[8];

CdlLOC pauseLocation = { 0 };
CdlLOC relpauseLocation = { 0 };

int cdStatus = 0;
int cdReadOK = 0;
int cdReadResult = 0;

int paused = 0;
int trackToPlay = 2;
int lastTrack = 0;

void LoadSoundFromCD() {
    CdlFILE* fileAddress;

    loadFile = "\\CULT_SPOT;1";
    fileAddress = CdSearchFile(&file, loadFile);

    if (fileAddress != NULL) {
        dataBuffer = malloc(BtoS(file.size) * CD_SECTOR_SIZE);

        CdControl(CdlSetloc, (u_char*)&file.pos, debugCtrlResult);
        cdReadOK = CdRead((int)BtoS(file.size), (u_long*)dataBuffer, CdlModeSpeed);
        cdReadResult = CdReadSync(0, 0);

        if (cdReadResult == 0) {
            LoadCultist(dataBuffer);
        }
    }
}

void CDPrepareMusic() {
    // Heavily taken from hello_cdda

    numTOC = CdGetToc(location);

    // Prevent out of bound pos
    for (size_t i = 1; i < numTOC; i++) {
        CdIntToPos(CdPosToInt(&location[i]) - 74, &location[i]);
    }

    u_char params[4];
    params[0] = CdlModeRept | CdlModeDA;
    cdStatus = CdControlB(CdlSetmode, params, debugCtrlResult);
}

void CDPlayTrack(long trackNum) {
    lastTrack = trackNum;

    if (!cdStatus) {
        return;
    }

    if (paused) {
        paused = 0;
    }

    CdControlB(CdlPlay, (u_char*)&location[trackNum], debugCtrlResult);
}

void CDTogglePause() {
    if (!paused) {
        u_char pauseParameters[8];

        CdControlF(CdlPause, 0);
        CdControlB(CdlGetlocP, 0, pauseParameters);

        pauseLocation.minute = pauseParameters[5];
        pauseLocation.second = pauseParameters[6];
        pauseLocation.sector = pauseParameters[7];
        pauseLocation.track = location[(u_char)lastTrack].track;

        relpauseLocation.minute = pauseParameters[2];
        relpauseLocation.second = pauseParameters[3];
        relpauseLocation.sector = pauseParameters[4];
        relpauseLocation.track = location[(u_char)lastTrack].track;

        paused = 1;
    }
    else {
        CdControlB(CdlPlay, 0, 0);
        paused = 0;
    }   
}

void CDIncrementTrack() {
    trackToPlay++;

    if (trackToPlay > numTOC) {
        trackToPlay = 2;
    }
}
