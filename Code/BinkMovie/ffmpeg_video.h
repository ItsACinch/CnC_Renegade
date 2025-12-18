/*
** FFmpeg Video Backend
**
** Provides video playback functionality using FFmpeg as a replacement
** for the proprietary RAD Bink Video SDK.
**
** FFmpeg DLLs are loaded dynamically so the game can still run without them.
*/

#ifndef FFMPEG_VIDEO_H
#define FFMPEG_VIDEO_H

#include <windows.h>

// Error codes
#define FFVIDEO_OK      0
#define FFVIDEO_ERROR  -1

// Forward declaration for video handle
typedef struct FFVideo FFVideo;

// Video information structure (compatible with Bink interface)
typedef struct FFVideoInfo {
    unsigned int Width;
    unsigned int Height;
    unsigned int Frames;          // Total frames (may be 0 for streaming)
    unsigned int FrameNum;        // Current frame number
    unsigned int FrameRate;       // Frames per second numerator
    unsigned int FrameRateDiv;    // Frames per second denominator
    unsigned int LastFrameNum;    // Last frame number
    int ReadError;                // Non-zero if read error occurred
} FFVideoInfo;

// Surface formats (matches Bink constants for compatibility)
#define FFVIDEO_SURFACE32     0
#define FFVIDEO_SURFACE24     1
#define FFVIDEO_SURFACE16     2   // Generic 16-bit
#define FFVIDEO_SURFACE565    4   // RGB565 (what the game uses)
#define FFVIDEO_SURFACE555    5   // RGB555
#define FFVIDEO_SURFACE32R    6   // RGBX
#define FFVIDEO_SURFACE32A    7   // RGBA

// Copy flags
#define FFVIDEO_COPY_NOSCALING 0x00000000
#define FFVIDEO_COPY_ALL       0x00000001

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Core System Functions
// ============================================================================

// Initialize FFmpeg video system (loads DLLs)
// Returns FFVIDEO_OK on success, FFVIDEO_ERROR on failure
int FFVideo_Startup(void);

// Shutdown FFmpeg video system
void FFVideo_Shutdown(void);

// Check if FFmpeg is initialized and ready
int FFVideo_Is_Ready(void);

// Get last error message
const char* FFVideo_Last_Error(void);

// ============================================================================
// Video Playback Functions
// ============================================================================

// Open a video file
// Returns video handle or NULL on failure
FFVideo* FFVideo_Open(const char* filename, unsigned int flags);

// Close a video file
void FFVideo_Close(FFVideo* video);

// Get video information
FFVideoInfo* FFVideo_Get_Info(FFVideo* video);

// Check if it's time to display the next frame (for timing)
// Returns 0 if we should wait, non-zero if ready for next frame
int FFVideo_Wait(FFVideo* video);

// Decode the current frame
// Returns 0 on success
int FFVideo_Do_Frame(FFVideo* video);

// Advance to the next frame
void FFVideo_Next_Frame(FFVideo* video);

// Copy decoded frame to buffer
// dest: destination buffer
// pitch: bytes per row in destination
// height: height of destination
// x, y: offset in source frame
// flags: surface format (FFVIDEO_SURFACE*)
void FFVideo_Copy_To_Buffer(FFVideo* video, void* dest, int pitch,
                             unsigned int height, unsigned int x,
                             unsigned int y, unsigned int flags);

// Seek to a specific frame
void FFVideo_Goto(FFVideo* video, unsigned int frame, unsigned int flags);

// ============================================================================
// Audio Functions (video audio track)
// ============================================================================

// Set audio volume (0-32768)
void FFVideo_Set_Volume(FFVideo* video, unsigned int track, int volume);

// Set audio pan (-32768 to 32768)
void FFVideo_Set_Pan(FFVideo* video, unsigned int track, int pan);

// Service audio (call periodically to keep audio playing)
void FFVideo_Service(FFVideo* video);

// Initialize audio output (call once during startup)
void FFVideo_Sound_Init(void* param);

#ifdef __cplusplus
}
#endif

#endif // FFMPEG_VIDEO_H
