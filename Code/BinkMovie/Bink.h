/*
** RAD Bink SDK Stub Header
**
** This stub provides minimal definitions to allow compilation
** when the Bink SDK is not available. Video playback will be disabled.
*/

#ifndef BINK_H
#define BINK_H

// Always provide stubs when this file is included
// The real Bink SDK would have different headers

// Bink structures (must be defined before handle typedefs)
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

// Stub type definitions
typedef BINK* HBINK;
typedef void* HBINKBUFFER;

typedef struct {
    void* TargetType;
    void* DIBBuffer;
    unsigned int WindowWidth;
    unsigned int WindowHeight;
} BINKBUFFER;

// Bink flags
#define BINKSURFACE32 0
#define BINKSURFACE24 1
#define BINKSURFACE16 2
#define BINKSURFACE8  3
#define BINKSURFACE565 4
#define BINKSURFACE555 5
#define BINKSURFACE32R 6
#define BINKSURFACE32A 7
#define BINKSURFACE8P  8
#define BINKSURFACEYUY2 9
#define BINKSURFACEU8V8 10
#define BINKSURFACEYV12 11

#define BINKNOFRAMEBUFFERS 0x00000400
#define BINKNOSKIP 0x00080000
#define BINKPRELOADALL 0x00002000
#define BINKSNDTRACK 0x00004000

// Copy flags
#define BINKCOPYNOSCALING 0x00000000
#define BINKCOPYALL 0x00000001

// Stub function definitions (do nothing)
inline HBINK BinkOpen(const char* name, unsigned int flags) { return NULL; }
inline void BinkClose(HBINK bink) {}
inline int BinkWait(HBINK bink) { return 0; }
inline void BinkNextFrame(HBINK bink) {}
inline int BinkDoFrame(HBINK bink) { return 0; }
inline void BinkCopyToBuffer(HBINK bink, void* dest, int pitch, unsigned int height, unsigned int x, unsigned int y, unsigned int flags) {}
inline void BinkSetVolume(HBINK bink, unsigned int track, int volume) {}
inline void BinkSetPan(HBINK bink, unsigned int track, int pan) {}
inline void BinkGoto(HBINK bink, unsigned int frame, unsigned int flags) {}
inline void BinkSetSoundSystem(void* open, unsigned int param) {}
inline void* BinkOpenDirectSound(unsigned int param) { return NULL; }
inline void BinkSetSoundTrack(unsigned int tracks, unsigned int* trackids) {}
inline void BinkService(HBINK bink) {}
inline char* BinkGetError() { return "Video playback disabled (Bink SDK not available)"; }
inline void BinkSoundUseDirectSound(void* param) {}

#endif // BINK_H
