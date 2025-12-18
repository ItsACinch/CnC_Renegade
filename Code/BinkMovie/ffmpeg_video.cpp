/*
** FFmpeg Video Backend Implementation
**
** Uses FFmpeg libraries for video decoding as a replacement for Bink.
** FFmpeg is loaded dynamically so the game can run without it (no video).
**
** Required DLLs (place in game directory):
** - avcodec-*.dll
** - avformat-*.dll
** - avutil-*.dll
** - swscale-*.dll
** - swresample-*.dll (for audio)
*/

#include "ffmpeg_video.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// FFmpeg Type Definitions (subset needed for video playback)
// ============================================================================

// AVRational - fraction representation
typedef struct AVRational { int num; int den; } AVRational;

// Pixel formats we care about
#define AV_PIX_FMT_RGB565LE 44
#define AV_PIX_FMT_RGB24    2
#define AV_PIX_FMT_RGBA     26

// Media types
#define AVMEDIA_TYPE_VIDEO 0
#define AVMEDIA_TYPE_AUDIO 1

// Error codes
#define AVERROR_EOF (-541478725)  // FFERRTAG('E','O','F',' ')

// Seek flags
#define AVSEEK_FLAG_BACKWARD 1
#define AVSEEK_FLAG_FRAME    8

// Sample formats
#define AV_SAMPLE_FMT_FLTP 8
#define AV_SAMPLE_FMT_S16  1

// Forward declarations
typedef struct AVFormatContext AVFormatContext;
typedef struct AVCodecContext AVCodecContext;
typedef struct AVCodec AVCodec;
typedef struct AVFrame AVFrame;
typedef struct AVPacket AVPacket;
typedef struct SwsContext SwsContext;
typedef struct SwrContext SwrContext;

// ============================================================================
// FFmpeg Function Pointer Types
// ============================================================================

// Format/Demuxer functions
typedef int (*PFN_avformat_open_input)(AVFormatContext**, const char*, void*, void**);
typedef void (*PFN_avformat_close_input)(AVFormatContext**);
typedef int (*PFN_avformat_find_stream_info)(AVFormatContext*, void**);
typedef int (*PFN_av_find_best_stream)(AVFormatContext*, int, int, int, const AVCodec**, int);
typedef int (*PFN_av_read_frame)(AVFormatContext*, AVPacket*);
typedef int (*PFN_av_seek_frame)(AVFormatContext*, int, long long, int);

// Codec functions
typedef AVCodecContext* (*PFN_avcodec_alloc_context3)(const AVCodec*);
typedef int (*PFN_avcodec_parameters_to_context)(AVCodecContext*, void*);
typedef int (*PFN_avcodec_open2)(AVCodecContext*, const AVCodec*, void**);
typedef void (*PFN_avcodec_free_context)(AVCodecContext**);
typedef int (*PFN_avcodec_send_packet)(AVCodecContext*, const AVPacket*);
typedef int (*PFN_avcodec_receive_frame)(AVCodecContext*, AVFrame*);
typedef const AVCodec* (*PFN_avcodec_find_decoder)(int);

// Frame/Packet functions
typedef AVFrame* (*PFN_av_frame_alloc)(void);
typedef void (*PFN_av_frame_free)(AVFrame**);
typedef void (*PFN_av_frame_unref)(AVFrame*);
typedef AVPacket* (*PFN_av_packet_alloc)(void);
typedef void (*PFN_av_packet_free)(AVPacket**);
typedef void (*PFN_av_packet_unref)(AVPacket*);

// Scale/convert functions
typedef SwsContext* (*PFN_sws_getContext)(int, int, int, int, int, int, int, void*, void*, double*);
typedef int (*PFN_sws_scale)(SwsContext*, const unsigned char* const*, const int*, int, int, unsigned char* const*, const int*);
typedef void (*PFN_sws_freeContext)(SwsContext*);

// Utility functions
typedef void (*PFN_av_log_set_level)(int);

// ============================================================================
// FFmpeg Function Pointers Storage
// ============================================================================

static struct {
    HMODULE hAvFormat;
    HMODULE hAvCodec;
    HMODULE hAvUtil;
    HMODULE hSwScale;
    HMODULE hSwResample;

    // Format functions
    PFN_avformat_open_input avformat_open_input;
    PFN_avformat_close_input avformat_close_input;
    PFN_avformat_find_stream_info avformat_find_stream_info;
    PFN_av_find_best_stream av_find_best_stream;
    PFN_av_read_frame av_read_frame;
    PFN_av_seek_frame av_seek_frame;

    // Codec functions
    PFN_avcodec_alloc_context3 avcodec_alloc_context3;
    PFN_avcodec_parameters_to_context avcodec_parameters_to_context;
    PFN_avcodec_open2 avcodec_open2;
    PFN_avcodec_free_context avcodec_free_context;
    PFN_avcodec_send_packet avcodec_send_packet;
    PFN_avcodec_receive_frame avcodec_receive_frame;
    PFN_avcodec_find_decoder avcodec_find_decoder;

    // Frame/Packet functions
    PFN_av_frame_alloc av_frame_alloc;
    PFN_av_frame_free av_frame_free;
    PFN_av_frame_unref av_frame_unref;
    PFN_av_packet_alloc av_packet_alloc;
    PFN_av_packet_free av_packet_free;
    PFN_av_packet_unref av_packet_unref;

    // Scale functions
    PFN_sws_getContext sws_getContext;
    PFN_sws_scale sws_scale;
    PFN_sws_freeContext sws_freeContext;

    // Utility
    PFN_av_log_set_level av_log_set_level;
} g_FF = {0};

// ============================================================================
// Global State
// ============================================================================

static int g_Initialized = 0;
static char g_LastError[512] = "No error";
static DWORD g_StartTime = 0;

// ============================================================================
// Video Context Structure
// ============================================================================

// We need to access some AVFormatContext fields directly
// This is a simplified structure matching key FFmpeg internals
typedef struct FFVideoStream {
    void* codecpar;         // AVCodecParameters*
    AVRational time_base;
    AVRational r_frame_rate;
    long long duration;
    long long nb_frames;
} FFVideoStream;

struct FFVideo {
    // FFmpeg contexts
    AVFormatContext* formatCtx;
    AVCodecContext* videoCodecCtx;
    AVCodecContext* audioCodecCtx;
    AVFrame* frame;
    AVFrame* frameRGB;
    AVPacket* packet;
    SwsContext* swsCtx;

    // Stream indices
    int videoStreamIdx;
    int audioStreamIdx;

    // Video info (public interface)
    FFVideoInfo info;

    // Internal state
    unsigned char* rgbBuffer;
    int rgbBufferSize;
    int currentPixFmt;
    DWORD frameStartTime;
    DWORD frameDuration;
    int frameReady;
    int eof;

    // Audio state
    int audioVolume;
    int audioPan;
};

// ============================================================================
// Helper Functions
// ============================================================================

static void SetError(const char* error) {
    strncpy(g_LastError, error, sizeof(g_LastError) - 1);
    g_LastError[sizeof(g_LastError) - 1] = '\0';
}

// Try to load a DLL with various version suffixes
static HMODULE LoadFFmpegDLL(const char* baseName) {
    char dllName[256];
    HMODULE hModule = NULL;

    // Try common FFmpeg version numbers (newest first)
    int versions[] = {61, 60, 59, 58, 57, 56, 55, -1};

    for (int i = 0; versions[i] >= 0; i++) {
        sprintf(dllName, "%s-%d.dll", baseName, versions[i]);
        hModule = LoadLibraryA(dllName);
        if (hModule) return hModule;
    }

    // Try without version number
    sprintf(dllName, "%s.dll", baseName);
    hModule = LoadLibraryA(dllName);

    return hModule;
}

// ============================================================================
// Core System Functions
// ============================================================================

int FFVideo_Startup(void) {
    if (g_Initialized) return FFVIDEO_OK;

    // Load FFmpeg DLLs
    g_FF.hAvUtil = LoadFFmpegDLL("avutil");
    g_FF.hAvCodec = LoadFFmpegDLL("avcodec");
    g_FF.hAvFormat = LoadFFmpegDLL("avformat");
    g_FF.hSwScale = LoadFFmpegDLL("swscale");
    g_FF.hSwResample = LoadFFmpegDLL("swresample");

    if (!g_FF.hAvFormat || !g_FF.hAvCodec || !g_FF.hAvUtil || !g_FF.hSwScale) {
        SetError("Failed to load FFmpeg DLLs (avformat, avcodec, avutil, swscale)");
        FFVideo_Shutdown();
        return FFVIDEO_ERROR;
    }

    // Load format functions
    g_FF.avformat_open_input = (PFN_avformat_open_input)GetProcAddress(g_FF.hAvFormat, "avformat_open_input");
    g_FF.avformat_close_input = (PFN_avformat_close_input)GetProcAddress(g_FF.hAvFormat, "avformat_close_input");
    g_FF.avformat_find_stream_info = (PFN_avformat_find_stream_info)GetProcAddress(g_FF.hAvFormat, "avformat_find_stream_info");
    g_FF.av_find_best_stream = (PFN_av_find_best_stream)GetProcAddress(g_FF.hAvFormat, "av_find_best_stream");
    g_FF.av_read_frame = (PFN_av_read_frame)GetProcAddress(g_FF.hAvFormat, "av_read_frame");
    g_FF.av_seek_frame = (PFN_av_seek_frame)GetProcAddress(g_FF.hAvFormat, "av_seek_frame");

    // Load codec functions
    g_FF.avcodec_alloc_context3 = (PFN_avcodec_alloc_context3)GetProcAddress(g_FF.hAvCodec, "avcodec_alloc_context3");
    g_FF.avcodec_parameters_to_context = (PFN_avcodec_parameters_to_context)GetProcAddress(g_FF.hAvCodec, "avcodec_parameters_to_context");
    g_FF.avcodec_open2 = (PFN_avcodec_open2)GetProcAddress(g_FF.hAvCodec, "avcodec_open2");
    g_FF.avcodec_free_context = (PFN_avcodec_free_context)GetProcAddress(g_FF.hAvCodec, "avcodec_free_context");
    g_FF.avcodec_send_packet = (PFN_avcodec_send_packet)GetProcAddress(g_FF.hAvCodec, "avcodec_send_packet");
    g_FF.avcodec_receive_frame = (PFN_avcodec_receive_frame)GetProcAddress(g_FF.hAvCodec, "avcodec_receive_frame");
    g_FF.avcodec_find_decoder = (PFN_avcodec_find_decoder)GetProcAddress(g_FF.hAvCodec, "avcodec_find_decoder");

    // Load frame/packet functions (may be in avcodec or avutil)
    g_FF.av_frame_alloc = (PFN_av_frame_alloc)GetProcAddress(g_FF.hAvUtil, "av_frame_alloc");
    g_FF.av_frame_free = (PFN_av_frame_free)GetProcAddress(g_FF.hAvUtil, "av_frame_free");
    g_FF.av_frame_unref = (PFN_av_frame_unref)GetProcAddress(g_FF.hAvUtil, "av_frame_unref");
    g_FF.av_packet_alloc = (PFN_av_packet_alloc)GetProcAddress(g_FF.hAvCodec, "av_packet_alloc");
    g_FF.av_packet_free = (PFN_av_packet_free)GetProcAddress(g_FF.hAvCodec, "av_packet_free");
    g_FF.av_packet_unref = (PFN_av_packet_unref)GetProcAddress(g_FF.hAvCodec, "av_packet_unref");

    // Load scale functions
    g_FF.sws_getContext = (PFN_sws_getContext)GetProcAddress(g_FF.hSwScale, "sws_getContext");
    g_FF.sws_scale = (PFN_sws_scale)GetProcAddress(g_FF.hSwScale, "sws_scale");
    g_FF.sws_freeContext = (PFN_sws_freeContext)GetProcAddress(g_FF.hSwScale, "sws_freeContext");

    // Utility functions
    g_FF.av_log_set_level = (PFN_av_log_set_level)GetProcAddress(g_FF.hAvUtil, "av_log_set_level");

    // Verify critical functions
    if (!g_FF.avformat_open_input || !g_FF.avcodec_alloc_context3 ||
        !g_FF.av_frame_alloc || !g_FF.sws_getContext) {
        SetError("Failed to load required FFmpeg functions");
        FFVideo_Shutdown();
        return FFVIDEO_ERROR;
    }

    // Disable FFmpeg logging
    if (g_FF.av_log_set_level) {
        g_FF.av_log_set_level(-8); // AV_LOG_QUIET
    }

    g_StartTime = GetTickCount();
    g_Initialized = 1;
    SetError("FFmpeg video initialized successfully");

    return FFVIDEO_OK;
}

void FFVideo_Shutdown(void) {
    if (g_FF.hSwResample) FreeLibrary(g_FF.hSwResample);
    if (g_FF.hSwScale) FreeLibrary(g_FF.hSwScale);
    if (g_FF.hAvFormat) FreeLibrary(g_FF.hAvFormat);
    if (g_FF.hAvCodec) FreeLibrary(g_FF.hAvCodec);
    if (g_FF.hAvUtil) FreeLibrary(g_FF.hAvUtil);

    memset(&g_FF, 0, sizeof(g_FF));
    g_Initialized = 0;
}

int FFVideo_Is_Ready(void) {
    return g_Initialized;
}

const char* FFVideo_Last_Error(void) {
    return g_LastError;
}

// ============================================================================
// Video Playback Implementation
// ============================================================================

// Helper to get stream info from format context
// FFmpeg's AVFormatContext has streams as an array of pointers
typedef struct AVStream_Minimal {
    int index;
    int id;
    void* codecpar;           // AVCodecParameters*
    AVRational time_base;
    long long start_time;
    long long duration;
    long long nb_frames;
    AVRational r_frame_rate;
    AVRational avg_frame_rate;
} AVStream_Minimal;

typedef struct AVFormatContext_Minimal {
    void* av_class;
    void* iformat;
    void* oformat;
    void* priv_data;
    void* pb;
    int ctx_flags;
    unsigned int nb_streams;
    AVStream_Minimal** streams;
    // ... more fields we don't need
} AVFormatContext_Minimal;

// AVCodecParameters minimal definition
typedef struct AVCodecParameters_Minimal {
    int codec_type;
    int codec_id;
    unsigned int codec_tag;
    void* extradata;
    int extradata_size;
    int format;
    long long bit_rate;
    int bits_per_coded_sample;
    int bits_per_raw_sample;
    int profile;
    int level;
    int width;
    int height;
    // ... more fields
} AVCodecParameters_Minimal;

// AVCodecContext minimal definition
typedef struct AVCodecContext_Minimal {
    void* av_class;
    int log_level_offset;
    int codec_type;
    void* codec;
    int codec_id;
    unsigned int codec_tag;
    void* priv_data;
    void* internal;
    void* opaque;
    long long bit_rate;
    int bit_rate_tolerance;
    int global_quality;
    int compression_level;
    int flags;
    int flags2;
    void* extradata;
    int extradata_size;
    AVRational time_base;
    int ticks_per_frame;
    int delay;
    int width;
    int height;
    int coded_width;
    int coded_height;
    int gop_size;
    int pix_fmt;
    // ... more fields
} AVCodecContext_Minimal;

// AVFrame minimal definition
typedef struct AVFrame_Minimal {
    unsigned char* data[8];
    int linesize[8];
    unsigned char** extended_data;
    int width;
    int height;
    int nb_samples;
    int format;
    int key_frame;
    int pict_type;
    AVRational sample_aspect_ratio;
    long long pts;
    // ... more fields
} AVFrame_Minimal;

FFVideo* FFVideo_Open(const char* filename, unsigned int flags) {
    (void)flags;

    if (!g_Initialized) {
        SetError("FFmpeg not initialized");
        return NULL;
    }

    if (!filename || filename[0] == '\0') {
        SetError("Invalid filename");
        return NULL;
    }

    FFVideo* video = (FFVideo*)malloc(sizeof(FFVideo));
    if (!video) {
        SetError("Failed to allocate video context");
        return NULL;
    }
    memset(video, 0, sizeof(FFVideo));

    video->videoStreamIdx = -1;
    video->audioStreamIdx = -1;
    video->audioVolume = 32768; // Max volume

    // Open input file
    int ret = g_FF.avformat_open_input(&video->formatCtx, filename, NULL, NULL);
    if (ret < 0) {
        char err[256];
        sprintf(err, "Failed to open video file: %s", filename);
        SetError(err);
        free(video);
        return NULL;
    }

    // Get stream info
    ret = g_FF.avformat_find_stream_info(video->formatCtx, NULL);
    if (ret < 0) {
        SetError("Failed to find stream info");
        g_FF.avformat_close_input(&video->formatCtx);
        free(video);
        return NULL;
    }

    // Find video stream
    const AVCodec* videoCodec = NULL;
    video->videoStreamIdx = g_FF.av_find_best_stream(video->formatCtx,
        AVMEDIA_TYPE_VIDEO, -1, -1, &videoCodec, 0);

    if (video->videoStreamIdx < 0) {
        SetError("No video stream found");
        g_FF.avformat_close_input(&video->formatCtx);
        free(video);
        return NULL;
    }

    // Get stream info through our minimal struct
    AVFormatContext_Minimal* fmtCtx = (AVFormatContext_Minimal*)video->formatCtx;
    AVStream_Minimal* videoStream = fmtCtx->streams[video->videoStreamIdx];
    AVCodecParameters_Minimal* codecPar = (AVCodecParameters_Minimal*)videoStream->codecpar;

    // Create video codec context
    video->videoCodecCtx = g_FF.avcodec_alloc_context3(videoCodec);
    if (!video->videoCodecCtx) {
        SetError("Failed to allocate video codec context");
        g_FF.avformat_close_input(&video->formatCtx);
        free(video);
        return NULL;
    }

    // Copy codec parameters
    ret = g_FF.avcodec_parameters_to_context(video->videoCodecCtx, videoStream->codecpar);
    if (ret < 0) {
        SetError("Failed to copy codec parameters");
        g_FF.avcodec_free_context(&video->videoCodecCtx);
        g_FF.avformat_close_input(&video->formatCtx);
        free(video);
        return NULL;
    }

    // Open codec
    ret = g_FF.avcodec_open2(video->videoCodecCtx, videoCodec, NULL);
    if (ret < 0) {
        SetError("Failed to open video codec");
        g_FF.avcodec_free_context(&video->videoCodecCtx);
        g_FF.avformat_close_input(&video->formatCtx);
        free(video);
        return NULL;
    }

    // Allocate frames
    video->frame = g_FF.av_frame_alloc();
    video->frameRGB = g_FF.av_frame_alloc();
    if (!video->frame || !video->frameRGB) {
        SetError("Failed to allocate frames");
        FFVideo_Close(video);
        return NULL;
    }

    // Allocate packet
    video->packet = g_FF.av_packet_alloc();
    if (!video->packet) {
        SetError("Failed to allocate packet");
        FFVideo_Close(video);
        return NULL;
    }

    // Get video dimensions from codec context
    AVCodecContext_Minimal* codecCtx = (AVCodecContext_Minimal*)video->videoCodecCtx;
    int width = codecCtx->width;
    int height = codecCtx->height;
    int srcPixFmt = codecCtx->pix_fmt;

    // Allocate RGB buffer for converted frames
    video->rgbBufferSize = width * height * 2; // RGB565
    video->rgbBuffer = (unsigned char*)malloc(video->rgbBufferSize);
    if (!video->rgbBuffer) {
        SetError("Failed to allocate RGB buffer");
        FFVideo_Close(video);
        return NULL;
    }

    // Create scaler context for converting to RGB565
    video->swsCtx = g_FF.sws_getContext(
        width, height, srcPixFmt,
        width, height, AV_PIX_FMT_RGB565LE,
        2, // SWS_BILINEAR
        NULL, NULL, NULL);

    if (!video->swsCtx) {
        SetError("Failed to create scaler context");
        FFVideo_Close(video);
        return NULL;
    }

    // Fill in video info
    video->info.Width = width;
    video->info.Height = height;
    video->info.FrameNum = 0;
    video->info.LastFrameNum = 0;
    video->info.ReadError = 0;

    // Calculate frame count and rate
    if (videoStream->nb_frames > 0) {
        video->info.Frames = (unsigned int)videoStream->nb_frames;
    } else {
        // Estimate from duration
        video->info.Frames = 0; // Unknown
    }

    // Frame rate
    if (videoStream->r_frame_rate.den > 0) {
        video->info.FrameRate = videoStream->r_frame_rate.num;
        video->info.FrameRateDiv = videoStream->r_frame_rate.den;
    } else if (videoStream->avg_frame_rate.den > 0) {
        video->info.FrameRate = videoStream->avg_frame_rate.num;
        video->info.FrameRateDiv = videoStream->avg_frame_rate.den;
    } else {
        video->info.FrameRate = 30;
        video->info.FrameRateDiv = 1;
    }

    // Calculate frame duration in milliseconds
    if (video->info.FrameRateDiv > 0 && video->info.FrameRate > 0) {
        video->frameDuration = (video->info.FrameRateDiv * 1000) / video->info.FrameRate;
    } else {
        video->frameDuration = 33; // ~30fps default
    }

    video->frameStartTime = GetTickCount();
    video->currentPixFmt = FFVIDEO_SURFACE565;

    return video;
}

void FFVideo_Close(FFVideo* video) {
    if (!video) return;

    if (video->swsCtx) {
        g_FF.sws_freeContext(video->swsCtx);
    }

    if (video->rgbBuffer) {
        free(video->rgbBuffer);
    }

    if (video->packet) {
        g_FF.av_packet_free(&video->packet);
    }

    if (video->frameRGB) {
        g_FF.av_frame_free(&video->frameRGB);
    }

    if (video->frame) {
        g_FF.av_frame_free(&video->frame);
    }

    if (video->audioCodecCtx) {
        g_FF.avcodec_free_context(&video->audioCodecCtx);
    }

    if (video->videoCodecCtx) {
        g_FF.avcodec_free_context(&video->videoCodecCtx);
    }

    if (video->formatCtx) {
        g_FF.avformat_close_input(&video->formatCtx);
    }

    free(video);
}

FFVideoInfo* FFVideo_Get_Info(FFVideo* video) {
    return video ? &video->info : NULL;
}

int FFVideo_Wait(FFVideo* video) {
    if (!video) return 1;
    if (video->eof) return 1;

    // Check if enough time has passed for next frame
    DWORD currentTime = GetTickCount();
    DWORD elapsed = currentTime - video->frameStartTime;

    return (elapsed >= video->frameDuration) ? 0 : 1;
}

int FFVideo_Do_Frame(FFVideo* video) {
    if (!video || !video->formatCtx || !video->videoCodecCtx) return -1;
    if (video->eof) return -1;

    int ret;
    int gotFrame = 0;

    while (!gotFrame && !video->eof) {
        // Read packets until we get a video frame
        ret = g_FF.av_read_frame(video->formatCtx, video->packet);

        if (ret < 0) {
            if (ret == AVERROR_EOF) {
                video->eof = 1;
            }
            video->info.ReadError = 1;
            return ret;
        }

        // Check if this is a video packet
        AVFormatContext_Minimal* fmtCtx = (AVFormatContext_Minimal*)video->formatCtx;
        int streamIdx = *((int*)video->packet); // First field of AVPacket is stream_index

        if (streamIdx == video->videoStreamIdx) {
            // Send packet to decoder
            ret = g_FF.avcodec_send_packet(video->videoCodecCtx, video->packet);
            if (ret < 0) {
                g_FF.av_packet_unref(video->packet);
                continue;
            }

            // Receive decoded frame
            ret = g_FF.avcodec_receive_frame(video->videoCodecCtx, video->frame);
            if (ret == 0) {
                gotFrame = 1;

                // Convert to RGB565
                AVFrame_Minimal* frame = (AVFrame_Minimal*)video->frame;

                unsigned char* destData[1] = { video->rgbBuffer };
                int destLinesize[1] = { (int)(video->info.Width * 2) };

                g_FF.sws_scale(video->swsCtx,
                    (const unsigned char* const*)frame->data,
                    frame->linesize,
                    0, video->info.Height,
                    destData, destLinesize);

                video->frameReady = 1;
            }
        }

        g_FF.av_packet_unref(video->packet);
    }

    return gotFrame ? 0 : -1;
}

void FFVideo_Next_Frame(FFVideo* video) {
    if (!video) return;

    video->info.LastFrameNum = video->info.FrameNum;
    video->info.FrameNum++;
    video->frameStartTime = GetTickCount();
    video->frameReady = 0;

    // Update frame count if we're past the known count
    if (video->info.FrameNum > video->info.Frames) {
        video->info.Frames = video->info.FrameNum;
    }
}

void FFVideo_Copy_To_Buffer(FFVideo* video, void* dest, int pitch,
                             unsigned int height, unsigned int x,
                             unsigned int y, unsigned int flags) {
    if (!video || !video->rgbBuffer || !dest) return;

    (void)flags; // We always output RGB565 for now

    unsigned int srcWidth = video->info.Width;
    unsigned int srcHeight = video->info.Height;
    unsigned int copyWidth = srcWidth - x;
    unsigned int copyHeight = srcHeight - y;

    if (copyHeight > height) copyHeight = height;

    unsigned char* src = video->rgbBuffer + (y * srcWidth + x) * 2;
    unsigned char* dst = (unsigned char*)dest;

    for (unsigned int row = 0; row < copyHeight; row++) {
        memcpy(dst, src, copyWidth * 2);
        src += srcWidth * 2;
        dst += pitch;
    }
}

void FFVideo_Goto(FFVideo* video, unsigned int frame, unsigned int flags) {
    if (!video || !video->formatCtx) return;
    (void)flags;

    AVFormatContext_Minimal* fmtCtx = (AVFormatContext_Minimal*)video->formatCtx;
    AVStream_Minimal* stream = fmtCtx->streams[video->videoStreamIdx];

    // Calculate timestamp from frame number
    long long timestamp = 0;
    if (stream->time_base.den > 0 && video->info.FrameRate > 0) {
        // pts = frame * time_base_den / frame_rate
        timestamp = (long long)frame * stream->time_base.den *
                    video->info.FrameRateDiv / video->info.FrameRate;
    }

    g_FF.av_seek_frame(video->formatCtx, video->videoStreamIdx,
                       timestamp, AVSEEK_FLAG_BACKWARD);

    video->info.FrameNum = frame;
    video->eof = 0;
    video->frameReady = 0;
}

// ============================================================================
// Audio Functions (stubs for now - audio handled by OpenAL)
// ============================================================================

void FFVideo_Set_Volume(FFVideo* video, unsigned int track, int volume) {
    (void)track;
    if (video) video->audioVolume = volume;
}

void FFVideo_Set_Pan(FFVideo* video, unsigned int track, int pan) {
    (void)track;
    if (video) video->audioPan = pan;
}

void FFVideo_Service(FFVideo* video) {
    // Audio servicing would go here
    // For now, video audio is not implemented - games can use separate audio files
    (void)video;
}

void FFVideo_Sound_Init(void* param) {
    // Sound initialization - uses OpenAL backend already set up
    (void)param;
}
