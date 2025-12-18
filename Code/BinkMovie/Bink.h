/*
** RAD Bink SDK Compatibility Header
**
** This header provides Bink-compatible API using FFmpeg as the backend.
** The original Bink SDK is proprietary; this uses FFmpeg for video decoding.
**
** Required: FFmpeg DLLs in the game directory:
** - avcodec-XX.dll, avformat-XX.dll, avutil-XX.dll, swscale-XX.dll
** where XX is the FFmpeg version number (e.g., 60, 61)
**
** If FFmpeg DLLs are not present, video playback will be disabled
** but the game will still run.
*/

#ifndef BINK_H
#define BINK_H

#include "ffmpeg_video.h"

// Bink structure - mapped to FFVideoInfo
typedef struct BINK {
    unsigned int Width;
    unsigned int Height;
    unsigned int Frames;
    unsigned int FrameNum;
    unsigned int FrameRate;
    unsigned int FrameRateDiv;
    unsigned int LastFrameNum;
    void* ReadError;
} BINK;

// Handle types
typedef BINK* HBINK;
typedef void* HBINKBUFFER;

typedef struct {
    void* TargetType;
    void* DIBBuffer;
    unsigned int WindowWidth;
    unsigned int WindowHeight;
} BINKBUFFER;

// Bink flags (mapped to FFVideo equivalents)
#define BINKSURFACE32     FFVIDEO_SURFACE32
#define BINKSURFACE24     FFVIDEO_SURFACE24
#define BINKSURFACE16     FFVIDEO_SURFACE16
#define BINKSURFACE8      3
#define BINKSURFACE565    FFVIDEO_SURFACE565
#define BINKSURFACE555    FFVIDEO_SURFACE555
#define BINKSURFACE32R    FFVIDEO_SURFACE32R
#define BINKSURFACE32A    FFVIDEO_SURFACE32A
#define BINKSURFACE8P     8
#define BINKSURFACEYUY2   9
#define BINKSURFACEU8V8   10
#define BINKSURFACEYV12   11

#define BINKNOFRAMEBUFFERS  0x00000400
#define BINKNOSKIP          0x00080000
#define BINKPRELOADALL      0x00002000
#define BINKSNDTRACK        0x00004000

// Copy flags
#define BINKCOPYNOSCALING   FFVIDEO_COPY_NOSCALING
#define BINKCOPYALL         FFVIDEO_COPY_ALL

// Internal video handle storage (maps HBINK to FFVideo*)
// We store a small header before the FFVideo pointer to provide BINK struct
typedef struct BinkWrapper {
    BINK info;           // BINK-compatible info structure
    FFVideo* ffVideo;    // FFmpeg video handle
} BinkWrapper;

// ============================================================================
// Bink API Functions (inline wrappers around FFmpeg)
// ============================================================================

inline HBINK BinkOpen(const char* name, unsigned int flags) {
    // Initialize FFmpeg if not already done
    if (!FFVideo_Is_Ready()) {
        if (FFVideo_Startup() != FFVIDEO_OK) {
            return NULL;
        }
    }

    FFVideo* ffVideo = FFVideo_Open(name, flags);
    if (!ffVideo) {
        return NULL;
    }

    // Allocate wrapper structure
    BinkWrapper* wrapper = (BinkWrapper*)malloc(sizeof(BinkWrapper));
    if (!wrapper) {
        FFVideo_Close(ffVideo);
        return NULL;
    }

    wrapper->ffVideo = ffVideo;

    // Copy info from FFVideo to BINK struct
    FFVideoInfo* info = FFVideo_Get_Info(ffVideo);
    if (info) {
        wrapper->info.Width = info->Width;
        wrapper->info.Height = info->Height;
        wrapper->info.Frames = info->Frames;
        wrapper->info.FrameNum = info->FrameNum;
        wrapper->info.FrameRate = info->FrameRate;
        wrapper->info.FrameRateDiv = info->FrameRateDiv;
        wrapper->info.LastFrameNum = info->LastFrameNum;
        wrapper->info.ReadError = info->ReadError ? (void*)1 : NULL;
    }

    return &wrapper->info;
}

inline void BinkClose(HBINK bink) {
    if (!bink) return;

    // Get wrapper from BINK pointer (BINK is first member of BinkWrapper)
    BinkWrapper* wrapper = (BinkWrapper*)bink;
    FFVideo_Close(wrapper->ffVideo);
    free(wrapper);
}

inline int BinkWait(HBINK bink) {
    if (!bink) return 0;
    BinkWrapper* wrapper = (BinkWrapper*)bink;
    return FFVideo_Wait(wrapper->ffVideo);
}

inline int BinkDoFrame(HBINK bink) {
    if (!bink) return -1;
    BinkWrapper* wrapper = (BinkWrapper*)bink;
    int result = FFVideo_Do_Frame(wrapper->ffVideo);

    // Update BINK info from FFVideo
    FFVideoInfo* info = FFVideo_Get_Info(wrapper->ffVideo);
    if (info) {
        wrapper->info.FrameNum = info->FrameNum;
        wrapper->info.Frames = info->Frames;
        wrapper->info.ReadError = info->ReadError ? (void*)1 : NULL;
    }

    return result;
}

inline void BinkNextFrame(HBINK bink) {
    if (!bink) return;
    BinkWrapper* wrapper = (BinkWrapper*)bink;
    FFVideo_Next_Frame(wrapper->ffVideo);

    // Update BINK info
    FFVideoInfo* info = FFVideo_Get_Info(wrapper->ffVideo);
    if (info) {
        wrapper->info.FrameNum = info->FrameNum;
        wrapper->info.LastFrameNum = info->LastFrameNum;
        wrapper->info.Frames = info->Frames;
    }
}

inline void BinkCopyToBuffer(HBINK bink, void* dest, int pitch,
                              unsigned int height, unsigned int x,
                              unsigned int y, unsigned int flags) {
    if (!bink) return;
    BinkWrapper* wrapper = (BinkWrapper*)bink;
    FFVideo_Copy_To_Buffer(wrapper->ffVideo, dest, pitch, height, x, y, flags);
}

inline void BinkGoto(HBINK bink, unsigned int frame, unsigned int flags) {
    if (!bink) return;
    BinkWrapper* wrapper = (BinkWrapper*)bink;
    FFVideo_Goto(wrapper->ffVideo, frame, flags);

    // Update BINK info
    FFVideoInfo* info = FFVideo_Get_Info(wrapper->ffVideo);
    if (info) {
        wrapper->info.FrameNum = info->FrameNum;
    }
}

inline void BinkSetVolume(HBINK bink, unsigned int track, int volume) {
    if (!bink) return;
    BinkWrapper* wrapper = (BinkWrapper*)bink;
    FFVideo_Set_Volume(wrapper->ffVideo, track, volume);
}

inline void BinkSetPan(HBINK bink, unsigned int track, int pan) {
    if (!bink) return;
    BinkWrapper* wrapper = (BinkWrapper*)bink;
    FFVideo_Set_Pan(wrapper->ffVideo, track, pan);
}

inline void BinkService(HBINK bink) {
    if (!bink) return;
    BinkWrapper* wrapper = (BinkWrapper*)bink;
    FFVideo_Service(wrapper->ffVideo);
}

inline void BinkSetSoundSystem(void* open, unsigned int param) {
    (void)open;
    FFVideo_Sound_Init((void*)(size_t)param);
}

inline void* BinkOpenDirectSound(unsigned int param) {
    (void)param;
    return NULL;
}

inline void BinkSetSoundTrack(unsigned int tracks, unsigned int* trackids) {
    (void)tracks;
    (void)trackids;
}

inline char* BinkGetError() {
    return (char*)FFVideo_Last_Error();
}

inline void BinkSoundUseDirectSound(void* param) {
    FFVideo_Sound_Init(param);
}

#endif // BINK_H
