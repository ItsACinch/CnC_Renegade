# Phase 4: Video Playback Modernization

## Objective

Replace the proprietary RAD Bink SDK with FFmpeg-based video playback while maintaining the existing BINKMovie wrapper interface.

## Current Bink Integration

### API Functions Used
| Function | Purpose |
|----------|---------|
| `BinkOpen()` | Load .BIK video file |
| `BinkClose()` | Release video |
| `BinkWait()` | Frame timing sync |
| `BinkDoFrame()` | Decompress frame |
| `BinkCopyToBuffer()` | Copy frame to buffer |
| `BinkNextFrame()` | Advance to next frame |
| `BinkSoundUseDirectSound()` | Audio output config |

### Wrapper Interface
```cpp
class BINKMovie {
public:
    static void Init();
    static void Shutdown();
    static void Play(const char* filename, const char* subtitlename, FontCharsClass* font);
    static void Stop();
    static void Update();
    static void Render();
    static bool Is_Complete();
};
```

### Video Files
- Format: Bink Video (.BIK)
- Resolution: Various (up to 640x480 for this era)
- Audio: Bink Audio (compressed)
- Files: `DATA\MOVIES\*.BIK`

## Replacement Strategy

### Option A: FFmpeg (Recommended)
- Supports virtually all video formats
- Can decode original Bink files
- LGPL license with care
- Well-maintained

### Option B: Convert Videos to Modern Format
- Pre-convert .BIK to .MP4/.WEBM
- Simpler runtime code
- Requires asset preprocessing

### Recommendation: FFmpeg with Bink Support

FFmpeg can decode Bink video (bink/binkaudio decoders are built-in).

## Implementation Plan

### 4.1 FFmpeg Video Player Class

```cpp
// Code/BinkMovie/FFmpegPlayer.h

class FFmpegPlayer {
    AVFormatContext* formatCtx = nullptr;
    AVCodecContext* videoCodecCtx = nullptr;
    AVCodecContext* audioCodecCtx = nullptr;
    AVFrame* frame = nullptr;
    AVFrame* rgbFrame = nullptr;
    SwsContext* swsCtx = nullptr;

    int videoStreamIdx = -1;
    int audioStreamIdx = -1;

    // Timing
    double videoClock = 0.0;
    double frameTimer = 0.0;
    double lastPTS = 0.0;

    // Audio buffer for OpenAL
    std::vector<uint8_t> audioBuffer;

public:
    bool Open(const char* filename);
    void Close();
    bool DecodeFrame();
    bool GetVideoFrame(uint8_t* buffer, int pitch);
    bool GetAudioSamples(int16_t* buffer, int& sampleCount);
    bool IsFinished() const;

    int GetWidth() const { return videoCodecCtx ? videoCodecCtx->width : 0; }
    int GetHeight() const { return videoCodecCtx ? videoCodecCtx->height : 0; }
    double GetFrameRate() const;
};
```

### 4.2 FFmpeg Initialization

```cpp
// Code/BinkMovie/FFmpegPlayer.cpp

bool FFmpegPlayer::Open(const char* filename) {
    // Open file
    if (avformat_open_input(&formatCtx, filename, nullptr, nullptr) < 0)
        return false;

    if (avformat_find_stream_info(formatCtx, nullptr) < 0)
        return false;

    // Find video stream
    for (unsigned i = 0; i < formatCtx->nb_streams; i++) {
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIdx = i;
        } else if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIdx = i;
        }
    }

    if (videoStreamIdx < 0) return false;

    // Open video decoder
    const AVCodec* videoCodec = avcodec_find_decoder(
        formatCtx->streams[videoStreamIdx]->codecpar->codec_id);
    videoCodecCtx = avcodec_alloc_context3(videoCodec);
    avcodec_parameters_to_context(videoCodecCtx,
        formatCtx->streams[videoStreamIdx]->codecpar);
    avcodec_open2(videoCodecCtx, videoCodec, nullptr);

    // Open audio decoder if present
    if (audioStreamIdx >= 0) {
        const AVCodec* audioCodec = avcodec_find_decoder(
            formatCtx->streams[audioStreamIdx]->codecpar->codec_id);
        audioCodecCtx = avcodec_alloc_context3(audioCodec);
        avcodec_parameters_to_context(audioCodecCtx,
            formatCtx->streams[audioStreamIdx]->codecpar);
        avcodec_open2(audioCodecCtx, audioCodec, nullptr);
    }

    // Allocate frames
    frame = av_frame_alloc();
    rgbFrame = av_frame_alloc();

    // Setup scaler for RGB565 output (matching original Bink output)
    swsCtx = sws_getContext(
        videoCodecCtx->width, videoCodecCtx->height, videoCodecCtx->pix_fmt,
        videoCodecCtx->width, videoCodecCtx->height, AV_PIX_FMT_RGB565LE,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    return true;
}
```

### 4.3 Frame Decoding

```cpp
bool FFmpegPlayer::DecodeFrame() {
    AVPacket packet;

    while (av_read_frame(formatCtx, &packet) >= 0) {
        if (packet.stream_index == videoStreamIdx) {
            if (avcodec_send_packet(videoCodecCtx, &packet) == 0) {
                if (avcodec_receive_frame(videoCodecCtx, frame) == 0) {
                    // Update timing
                    if (packet.pts != AV_NOPTS_VALUE) {
                        videoClock = av_q2d(formatCtx->streams[videoStreamIdx]->time_base)
                                   * packet.pts;
                    }
                    av_packet_unref(&packet);
                    return true;
                }
            }
        } else if (packet.stream_index == audioStreamIdx) {
            // Queue audio for playback
            DecodeAudioPacket(&packet);
        }
        av_packet_unref(&packet);
    }

    return false;  // End of file
}

bool FFmpegPlayer::GetVideoFrame(uint8_t* buffer, int pitch) {
    // Convert to RGB565
    uint8_t* dest[1] = { buffer };
    int destLinesize[1] = { pitch };

    sws_scale(swsCtx, frame->data, frame->linesize, 0,
              videoCodecCtx->height, dest, destLinesize);

    return true;
}
```

### 4.4 Audio Handling

```cpp
void FFmpegPlayer::DecodeAudioPacket(AVPacket* packet) {
    AVFrame* audioFrame = av_frame_alloc();

    if (avcodec_send_packet(audioCodecCtx, packet) == 0) {
        while (avcodec_receive_frame(audioCodecCtx, audioFrame) == 0) {
            // Convert to 16-bit stereo for OpenAL
            int samples = audioFrame->nb_samples;
            int channels = audioCodecCtx->ch_layout.nb_channels;

            // Resample if needed (to 44100 Hz stereo)
            // ... resampling code ...

            // Add to audio buffer queue
            size_t dataSize = samples * channels * sizeof(int16_t);
            size_t offset = audioBuffer.size();
            audioBuffer.resize(offset + dataSize);
            memcpy(audioBuffer.data() + offset, audioFrame->data[0], dataSize);
        }
    }

    av_frame_free(&audioFrame);
}
```

### 4.5 Updated BINKMovie Wrapper

```cpp
// Code/BinkMovie/BINKMovie.cpp

#include "FFmpegPlayer.h"

static FFmpegPlayer* g_Player = nullptr;
static TextureClass* g_Textures[16];  // Texture array for large videos
static int g_TextureCount = 0;

void BINKMovie::Init() {
    // FFmpeg doesn't need global init in modern versions
}

void BINKMovie::Play(const char* filename, const char* subtitlename, FontCharsClass* font) {
    Stop();  // Clean up any existing playback

    g_Player = new FFmpegPlayer();
    if (!g_Player->Open(filename)) {
        delete g_Player;
        g_Player = nullptr;
        return;
    }

    // Create textures for video frames
    CreateVideoTextures(g_Player->GetWidth(), g_Player->GetHeight());

    // Initialize subtitles if provided
    if (subtitlename) {
        SubTitleManagerClass::Create(subtitlename, font);
    }

    // Disable game audio during movie
    WWAudioClass::Get_Instance()->Temp_Disable_Audio(true);
}

void BINKMovie::Update() {
    if (!g_Player) return;

    // Frame timing - check if it's time for next frame
    static DWORD lastTick = GetTickCount();
    DWORD currentTick = GetTickCount();
    double frameTime = 1000.0 / g_Player->GetFrameRate();

    if (currentTick - lastTick >= frameTime) {
        g_Player->DecodeFrame();
        lastTick = currentTick;
    }
}

void BINKMovie::Render() {
    if (!g_Player) return;

    // Get frame data
    static std::vector<uint8_t> frameBuffer;
    int width = g_Player->GetWidth();
    int height = g_Player->GetHeight();
    frameBuffer.resize(width * height * 2);  // RGB565

    g_Player->GetVideoFrame(frameBuffer.data(), width * 2);

    // Upload to textures and render
    UpdateVideoTextures(frameBuffer.data(), width, height);
    RenderVideoTextures();

    // Render subtitles
    SubTitleManagerClass::Render();
}

bool BINKMovie::Is_Complete() {
    return g_Player ? g_Player->IsFinished() : true;
}

void BINKMovie::Stop() {
    if (g_Player) {
        g_Player->Close();
        delete g_Player;
        g_Player = nullptr;
    }

    DestroyVideoTextures();
    SubTitleManagerClass::Destroy();
    WWAudioClass::Get_Instance()->Temp_Disable_Audio(false);
}
```

### 4.6 CMake Integration

```cmake
# Find FFmpeg
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED
    libavcodec
    libavformat
    libavutil
    libswscale
    libswresample
)

# BinkMovie library
add_library(BinkMovie STATIC
    FFmpegPlayer.cpp
    BINKMovie.cpp
    subtitle.cpp
    subtitlemanager.cpp
    subtitleparser.cpp
)

target_include_directories(BinkMovie PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_libraries(BinkMovie PRIVATE ${FFMPEG_LIBRARIES} ww3d2)
```

## Video Format Conversion (Alternative)

If you prefer to convert videos rather than decode Bink at runtime:

```bash
# Convert Bink to H.264 MP4
ffmpeg -i input.bik -c:v libx264 -crf 18 -c:a aac -b:a 192k output.mp4

# Convert to VP9 WebM
ffmpeg -i input.bik -c:v libvpx-vp9 -crf 30 -b:v 0 -c:a libopus output.webm
```

## File Changes

| Original File | Action |
|--------------|--------|
| BINKMovie.cpp | Major rewrite |
| BINKMovie.h | Keep interface, update internals |
| subtitle.cpp | Keep as-is |
| subtitlemanager.cpp | Keep as-is |
| subtitleparser.cpp | Keep as-is |

## New Files

| File | Purpose |
|------|---------|
| FFmpegPlayer.h | FFmpeg wrapper header |
| FFmpegPlayer.cpp | FFmpeg wrapper implementation |

## Runtime Requirements

| DLL | Version | Notes |
|-----|---------|-------|
| avcodec-XX.dll | FFmpeg 5.x+ | Video/audio decoding |
| avformat-XX.dll | FFmpeg 5.x+ | Container handling |
| avutil-XX.dll | FFmpeg 5.x+ | Utilities |
| swscale-XX.dll | FFmpeg 5.x+ | Pixel format conversion |
| swresample-XX.dll | FFmpeg 5.x+ | Audio resampling |

## Verification

1. All original .BIK files play correctly
2. Video/audio sync is maintained
3. Subtitles display with correct timing
4. Frame rate matches original
5. No visual artifacts or color issues
6. Smooth playback without stuttering

## Estimated Effort

- FFmpeg player implementation: 3-4 days
- Texture management: 1-2 days
- Audio integration: 1-2 days
- Testing/debugging: 2-3 days
- **Total: 7-11 days**

## Risks

1. **Bink-specific features**: Some Bink features may not have FFmpeg equivalents
2. **Audio sync**: A/V sync can be tricky with separate decode paths
3. **Memory usage**: FFmpeg can use more memory than Bink
4. **DLL dependencies**: FFmpeg has several runtime DLLs
