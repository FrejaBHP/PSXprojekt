#include "sound.h"

#define SWAP_ENDIAN32(x) (((x)>>24) | (((x)>>8) & 0xFF00) | (((x)<<8) & 0x00FF0000) | ((x)<<24))
#define SPU_MALLOC_MAX 5

typedef struct VAGheader{               // All the values in this header must be big endian
        char id[4];                     // VAGp         4 bytes -> 1 char * 4
        u_long version;                 // 4 bytes
        u_long reserved;                // 4 bytes
        u_long dataSize;                // (in bytes) 4 bytes
        u_long samplingFrequency;       // 4 bytes
        char  reserved2[12];            // 12 bytes -> 1 char * 12
        char  name[16];                 // 16 bytes -> 1 char * 16
        // Waveform data after that
} VAGhdr;

const VAGhdr* ThatsAllFolks = (VAGhdr*)thats_all_start;
const VAGhdr* Jump = (VAGhdr*)jump_start;

VAGhdr* Cultist;

u_long vag_spu_address;                  // address allocated in memory for first sound file
// DEBUG : these allow printing values for debugging
u_long spu_start_address;                
u_long get_start_addr;
u_long transSize;

char spuMallocRec[SPU_MALLOC_RECSIZ * (SPU_MALLOC_MAX + 1)];

SpuCommonAttr commonAttr;
SpuVoiceAttr voiceAttr;

long cultistLoaded = 0;

void SetupVoiceAttributes(u_long pitch, long channel, u_long soundAddr);

u_long SendVAGtoRAM(u_long vagDataSize, u_char* vagData) {
    u_long size;

    SpuSetTransferMode(SpuTransByDMA);
    size = SpuWrite(vagData + sizeof(VAGhdr), vagDataSize);
    SpuIsTransferCompleted(SPU_TRANSFER_WAIT);

    return size;
}

void PlaySoundFolks() {
    SpuSetKey(SpuOn, SPU_0CH);
}

void PlaySoundJump() {
    SpuSetKey(SpuOn, SPU_1CH);
}

void PlaySoundCultist() {
    if (cultistLoaded) {
        SpuSetKey(SpuOn, SPU_2CH);
    }
}

void InitSound() {
    SpuInitMalloc(SPU_MALLOC_MAX, spuMallocRec);

    commonAttr.mask = (
        SPU_COMMON_MVOLL | 
        SPU_COMMON_MVOLR |
        SPU_COMMON_CDVOLL |
        SPU_COMMON_CDVOLR |
        SPU_COMMON_CDMIX
    );

    commonAttr.mvol.left = 0x3fff;
    commonAttr.mvol.right = 0x3fff;
    commonAttr.cd.volume.left = 0x3fff;
    commonAttr.cd.volume.right = 0x3fff;
    commonAttr.cd.mix = SPU_ON;
    
    SpuSetCommonAttr(&commonAttr);
    SpuSetIRQ(SPU_OFF);


    // Setting up sounds for future use (should be moved later)
    u_long pitch = (SWAP_ENDIAN32(ThatsAllFolks->samplingFrequency) << 12) / 44100L;
    vag_spu_address = SpuMalloc(SWAP_ENDIAN32(ThatsAllFolks->dataSize));
    spu_start_address = SpuSetTransferStartAddr(vag_spu_address);
    get_start_addr = SpuGetTransferStartAddr();
    transSize = SendVAGtoRAM(SWAP_ENDIAN32(ThatsAllFolks->dataSize), thats_all_start);

    SetupVoiceAttributes(pitch, SPU_0CH, vag_spu_address);


    pitch = (SWAP_ENDIAN32(Jump->samplingFrequency) << 12) / 44100L;
    vag_spu_address = SpuMalloc(SWAP_ENDIAN32(Jump->dataSize));
    spu_start_address = SpuSetTransferStartAddr(vag_spu_address);
    get_start_addr = SpuGetTransferStartAddr();
    transSize = SendVAGtoRAM(SWAP_ENDIAN32(Jump->dataSize), jump_start);

    SetupVoiceAttributes(pitch, SPU_1CH, vag_spu_address);
}

// Voice seems to largely refer to the configuration that can be applied to sound channels
void SetupVoiceAttributes(u_long pitch, long channel, u_long soundAddr) {
    voiceAttr.mask = (
        SPU_VOICE_VOLL |
        SPU_VOICE_VOLR |
        SPU_VOICE_PITCH |
        SPU_VOICE_WDSA |
        SPU_VOICE_ADSR_AMODE |
        SPU_VOICE_ADSR_SMODE |
        SPU_VOICE_ADSR_RMODE |
        SPU_VOICE_ADSR_AR |
        SPU_VOICE_ADSR_DR |
        SPU_VOICE_ADSR_SR |
        SPU_VOICE_ADSR_RR |
        SPU_VOICE_ADSR_SL
    );

    voiceAttr.voice = channel;
    voiceAttr.volume.left = 0x1000;
    voiceAttr.volume.right = 0x1000;
    voiceAttr.pitch = pitch;
    voiceAttr.addr = soundAddr;
    voiceAttr.a_mode = SPU_VOICE_LINEARIncN;
    voiceAttr.s_mode = SPU_VOICE_LINEARIncN;
    voiceAttr.r_mode = SPU_VOICE_LINEARDecN;
    voiceAttr.ar = 0x0;
    voiceAttr.dr = 0x0;
    voiceAttr.rr = 0x0;
    voiceAttr.sr = 0x0;
    voiceAttr.sl = 0x0;

    // Seems to copy the values from the struct, rather than referring back to it, so can be used to set and forget, if special settings aren't needed?
    SpuSetVoiceAttr(&voiceAttr);
}

void LoadCultist(u_long* address) {
    Cultist = (VAGhdr*)address;

    u_long pitch = (SWAP_ENDIAN32(Cultist->samplingFrequency) << 12) / 44100L;
    vag_spu_address = SpuMalloc(SWAP_ENDIAN32(Cultist->dataSize));
    spu_start_address = SpuSetTransferStartAddr(vag_spu_address);
    get_start_addr = SpuGetTransferStartAddr();
    transSize = SendVAGtoRAM(SWAP_ENDIAN32(Cultist->dataSize), (u_char*)address);

    SetupVoiceAttributes(pitch, SPU_2CH, vag_spu_address);
    cultistLoaded = 1;
}
