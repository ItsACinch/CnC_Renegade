/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
** OpenAL Audio Backend Implementation
**
** This module dynamically loads OpenAL Soft and provides audio playback
** functionality compatible with the Miles Sound System API.
*/

#include "openal_audio.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// ============================================================================
// OpenAL Types and Constants (from al.h and alc.h)
// ============================================================================

typedef char ALboolean;
typedef char ALchar;
typedef signed char ALbyte;
typedef unsigned char ALubyte;
typedef short ALshort;
typedef unsigned short ALushort;
typedef int ALint;
typedef unsigned int ALuint;
typedef int ALsizei;
typedef int ALenum;
typedef float ALfloat;
typedef double ALdouble;
typedef void ALvoid;

typedef struct ALCdevice_struct ALCdevice;
typedef struct ALCcontext_struct ALCcontext;
typedef char ALCboolean;
typedef char ALCchar;
typedef signed char ALCbyte;
typedef unsigned char ALCubyte;
typedef short ALCshort;
typedef unsigned short ALCushort;
typedef int ALCint;
typedef unsigned int ALCuint;
typedef int ALCsizei;
typedef int ALCenum;
typedef float ALCfloat;
typedef double ALCdouble;
typedef void ALCvoid;

// AL Constants
#define AL_NONE                                   0
#define AL_FALSE                                  0
#define AL_TRUE                                   1
#define AL_SOURCE_RELATIVE                        0x202
#define AL_CONE_INNER_ANGLE                       0x1001
#define AL_CONE_OUTER_ANGLE                       0x1002
#define AL_PITCH                                  0x1003
#define AL_POSITION                               0x1004
#define AL_DIRECTION                              0x1005
#define AL_VELOCITY                               0x1006
#define AL_LOOPING                                0x1007
#define AL_BUFFER                                 0x1009
#define AL_GAIN                                   0x100A
#define AL_MIN_GAIN                               0x100D
#define AL_MAX_GAIN                               0x100E
#define AL_ORIENTATION                            0x100F
#define AL_SOURCE_STATE                           0x1010
#define AL_INITIAL                                0x1011
#define AL_PLAYING                                0x1012
#define AL_PAUSED                                 0x1013
#define AL_STOPPED                                0x1014
#define AL_BUFFERS_QUEUED                         0x1015
#define AL_BUFFERS_PROCESSED                      0x1016
#define AL_REFERENCE_DISTANCE                     0x1020
#define AL_ROLLOFF_FACTOR                         0x1021
#define AL_CONE_OUTER_GAIN                        0x1022
#define AL_MAX_DISTANCE                           0x1023
#define AL_SEC_OFFSET                             0x1024
#define AL_SAMPLE_OFFSET                          0x1025
#define AL_BYTE_OFFSET                            0x1026
#define AL_SOURCE_TYPE                            0x1027
#define AL_FORMAT_MONO8                           0x1100
#define AL_FORMAT_MONO16                          0x1101
#define AL_FORMAT_STEREO8                         0x1102
#define AL_FORMAT_STEREO16                        0x1103
#define AL_FREQUENCY                              0x2001
#define AL_BITS                                   0x2002
#define AL_CHANNELS                               0x2003
#define AL_SIZE                                   0x2004
#define AL_NO_ERROR                               0
#define AL_INVALID_NAME                           0xA001
#define AL_INVALID_ENUM                           0xA002
#define AL_INVALID_VALUE                          0xA003
#define AL_INVALID_OPERATION                      0xA004
#define AL_OUT_OF_MEMORY                          0xA005
#define AL_DOPPLER_FACTOR                         0xC000
#define AL_DOPPLER_VELOCITY                       0xC001
#define AL_SPEED_OF_SOUND                         0xC003
#define AL_DISTANCE_MODEL                         0xD000
#define AL_INVERSE_DISTANCE                       0xD001
#define AL_INVERSE_DISTANCE_CLAMPED               0xD002
#define AL_LINEAR_DISTANCE                        0xD003
#define AL_LINEAR_DISTANCE_CLAMPED                0xD004
#define AL_EXPONENT_DISTANCE                      0xD005
#define AL_EXPONENT_DISTANCE_CLAMPED              0xD006

// ALC Constants
#define ALC_FALSE                                 0
#define ALC_TRUE                                  1
#define ALC_FREQUENCY                             0x1007
#define ALC_REFRESH                               0x1008
#define ALC_SYNC                                  0x1009
#define ALC_MONO_SOURCES                          0x1010
#define ALC_STEREO_SOURCES                        0x1011
#define ALC_NO_ERROR                              0
#define ALC_INVALID_DEVICE                        0xA001
#define ALC_INVALID_CONTEXT                       0xA002
#define ALC_INVALID_ENUM                          0xA003
#define ALC_INVALID_VALUE                         0xA004
#define ALC_OUT_OF_MEMORY                         0xA005
#define ALC_MAJOR_VERSION                         0x1000
#define ALC_MINOR_VERSION                         0x1001
#define ALC_ATTRIBUTES_SIZE                       0x1002
#define ALC_ALL_ATTRIBUTES                        0x1003
#define ALC_DEFAULT_DEVICE_SPECIFIER              0x1004
#define ALC_DEVICE_SPECIFIER                      0x1005
#define ALC_EXTENSIONS                            0x1006

// ============================================================================
// OpenAL Function Pointers
// ============================================================================

typedef ALCdevice* (*LPALCOPENDEVICE)(const ALCchar* devicename);
typedef ALCboolean (*LPALCCLOSEDEVICE)(ALCdevice* device);
typedef ALCcontext* (*LPALCCREATECONTEXT)(ALCdevice* device, const ALCint* attrlist);
typedef void (*LPALCDESTROYCONTEXT)(ALCcontext* context);
typedef ALCboolean (*LPALCMAKECONTEXTCURRENT)(ALCcontext* context);
typedef ALCenum (*LPALCGETERROR)(ALCdevice* device);
typedef const ALCchar* (*LPALCGETSTRING)(ALCdevice* device, ALCenum param);

typedef void (*LPALGENBUFFERS)(ALsizei n, ALuint* buffers);
typedef void (*LPALDELETEBUFFERS)(ALsizei n, const ALuint* buffers);
typedef void (*LPALBUFFERDATA)(ALuint buffer, ALenum format, const ALvoid* data, ALsizei size, ALsizei freq);
typedef void (*LPALGETBUFFERI)(ALuint buffer, ALenum param, ALint* value);

typedef void (*LPALGENSOURCES)(ALsizei n, ALuint* sources);
typedef void (*LPALDELETESOURCES)(ALsizei n, const ALuint* sources);
typedef void (*LPALSOURCEI)(ALuint source, ALenum param, ALint value);
typedef void (*LPALSOURCEF)(ALuint source, ALenum param, ALfloat value);
typedef void (*LPALSOURCE3F)(ALuint source, ALenum param, ALfloat v1, ALfloat v2, ALfloat v3);
typedef void (*LPALSOURCEFV)(ALuint source, ALenum param, const ALfloat* values);
typedef void (*LPALGETSOURCEI)(ALuint source, ALenum param, ALint* value);
typedef void (*LPALGETSOURCEF)(ALuint source, ALenum param, ALfloat* value);
typedef void (*LPALGETSOURCE3F)(ALuint source, ALenum param, ALfloat* v1, ALfloat* v2, ALfloat* v3);
typedef void (*LPALSOURCEPLAY)(ALuint source);
typedef void (*LPALSOURCESTOP)(ALuint source);
typedef void (*LPALSOURCEPAUSE)(ALuint source);
typedef void (*LPALSOURCEREWIND)(ALuint source);
typedef void (*LPALSOURCEQUEUEBUFFERS)(ALuint source, ALsizei n, const ALuint* buffers);
typedef void (*LPALSOURCEUNQUEUEBUFFERS)(ALuint source, ALsizei n, ALuint* buffers);

typedef void (*LPALLISTENERF)(ALenum param, ALfloat value);
typedef void (*LPALLISTENER3F)(ALenum param, ALfloat v1, ALfloat v2, ALfloat v3);
typedef void (*LPALLISTENERFV)(ALenum param, const ALfloat* values);
typedef void (*LPALGETLISTENERF)(ALenum param, ALfloat* value);
typedef void (*LPALGETLISTENER3F)(ALenum param, ALfloat* v1, ALfloat* v2, ALfloat* v3);

typedef ALenum (*LPALGETERROR)(void);
typedef void (*LPALDOPPLERFACTOR)(ALfloat value);
typedef void (*LPALDISTANCEMODEL)(ALenum value);
typedef void (*LPALSPEEDOFSOUND)(ALfloat value);

// ============================================================================
// Global State
// ============================================================================

static HMODULE g_OpenALDLL = NULL;
static ALCdevice* g_Device = NULL;
static ALCcontext* g_Context = NULL;
static char g_LastError[256] = "No error";
static bool g_Initialized = false;

// Function pointers
static LPALCOPENDEVICE alc_OpenDevice = NULL;
static LPALCCLOSEDEVICE alc_CloseDevice = NULL;
static LPALCCREATECONTEXT alc_CreateContext = NULL;
static LPALCDESTROYCONTEXT alc_DestroyContext = NULL;
static LPALCMAKECONTEXTCURRENT alc_MakeContextCurrent = NULL;
static LPALCGETERROR alc_GetError = NULL;
static LPALCGETSTRING alc_GetString = NULL;

static LPALGENBUFFERS al_GenBuffers = NULL;
static LPALDELETEBUFFERS al_DeleteBuffers = NULL;
static LPALBUFFERDATA al_BufferData = NULL;
static LPALGETBUFFERI al_GetBufferi = NULL;

static LPALGENSOURCES al_GenSources = NULL;
static LPALDELETESOURCES al_DeleteSources = NULL;
static LPALSOURCEI al_Sourcei = NULL;
static LPALSOURCEF al_Sourcef = NULL;
static LPALSOURCE3F al_Source3f = NULL;
static LPALSOURCEFV al_Sourcefv = NULL;
static LPALGETSOURCEI al_GetSourcei = NULL;
static LPALGETSOURCEF al_GetSourcef = NULL;
static LPALGETSOURCE3F al_GetSource3f = NULL;
static LPALSOURCEPLAY al_SourcePlay = NULL;
static LPALSOURCESTOP al_SourceStop = NULL;
static LPALSOURCEPAUSE al_SourcePause = NULL;
static LPALSOURCEREWIND al_SourceRewind = NULL;
static LPALSOURCEQUEUEBUFFERS al_SourceQueueBuffers = NULL;
static LPALSOURCEUNQUEUEBUFFERS al_SourceUnqueueBuffers = NULL;

static LPALLISTENERF al_Listenerf = NULL;
static LPALLISTENER3F al_Listener3f = NULL;
static LPALLISTENERFV al_Listenerfv = NULL;
static LPALGETLISTENERF al_GetListenerf = NULL;
static LPALGETLISTENER3F al_GetListener3f = NULL;

static LPALGETERROR al_GetError = NULL;
static LPALDOPPLERFACTOR al_DopplerFactor = NULL;
static LPALDISTANCEMODEL al_DistanceModel = NULL;
static LPALSPEEDOFSOUND al_SpeedOfSound = NULL;

// File callbacks
static OAL_FileOpenCallback g_FileOpen = NULL;
static OAL_FileCloseCallback g_FileClose = NULL;
static OAL_FileSeekCallback g_FileSeek = NULL;
static OAL_FileReadCallback g_FileRead = NULL;

// ============================================================================
// Sample Data Structure
// ============================================================================

#define MAX_USER_DATA 4

struct Sample2D {
    bool in_use;
    ALuint source;
    ALuint buffer;
    OAL_S32 volume;          // 0-127
    OAL_S32 pan;             // 0-127
    OAL_S32 loop_count;      // 0=infinite, 1=once, etc.
    OAL_S32 playback_rate;
    OAL_S32 original_rate;
    OAL_U32 total_samples;
    OAL_S32 bits_per_sample;
    OAL_S32 channels;
    OAL_S32 user_data[MAX_USER_DATA];
    bool paused;
};

struct Sample3D {
    bool in_use;
    ALuint source;
    ALuint buffer;
    OAL_S32 volume;
    OAL_S32 loop_count;
    OAL_S32 playback_rate;
    OAL_S32 original_rate;
    OAL_U32 total_samples;
    OAL_S32 bits_per_sample;
    OAL_S32 channels;
    OAL_S32 user_data[MAX_USER_DATA];
    float pos_x, pos_y, pos_z;
    float min_dist, max_dist;
    bool paused;
};

static Sample2D g_Samples2D[OAL_MAX_2D_SAMPLES];
static Sample3D g_Samples3D[OAL_MAX_3D_SAMPLES];
static CRITICAL_SECTION g_AudioLock;

// ============================================================================
// Timer Data Structure
// ============================================================================

#define OAL_MAX_TIMERS 8

struct OALTimer {
    bool in_use;
    HANDLE thread;
    HANDLE stop_event;
    OAL_TimerCallback callback;
    OAL_U32 period_us;
    OAL_U32 user_data;
    bool running;
};

static OALTimer g_Timers[OAL_MAX_TIMERS];

// ============================================================================
// Load OpenAL DLL
// ============================================================================

static bool LoadOpenAL(void)
{
    // Try to load OpenAL Soft (preferred) or system OpenAL
    const char* dll_names[] = {
        "soft_oal.dll",      // OpenAL Soft renamed
        "OpenAL32.dll",      // Standard name
        NULL
    };

    for (int i = 0; dll_names[i] != NULL; i++) {
        g_OpenALDLL = LoadLibraryA(dll_names[i]);
        if (g_OpenALDLL) break;
    }

    if (!g_OpenALDLL) {
        strcpy(g_LastError, "Failed to load OpenAL32.dll or soft_oal.dll");
        return false;
    }

    // Load ALC functions
    alc_OpenDevice = (LPALCOPENDEVICE)GetProcAddress(g_OpenALDLL, "alcOpenDevice");
    alc_CloseDevice = (LPALCCLOSEDEVICE)GetProcAddress(g_OpenALDLL, "alcCloseDevice");
    alc_CreateContext = (LPALCCREATECONTEXT)GetProcAddress(g_OpenALDLL, "alcCreateContext");
    alc_DestroyContext = (LPALCDESTROYCONTEXT)GetProcAddress(g_OpenALDLL, "alcDestroyContext");
    alc_MakeContextCurrent = (LPALCMAKECONTEXTCURRENT)GetProcAddress(g_OpenALDLL, "alcMakeContextCurrent");
    alc_GetError = (LPALCGETERROR)GetProcAddress(g_OpenALDLL, "alcGetError");
    alc_GetString = (LPALCGETSTRING)GetProcAddress(g_OpenALDLL, "alcGetString");

    // Load AL functions
    al_GenBuffers = (LPALGENBUFFERS)GetProcAddress(g_OpenALDLL, "alGenBuffers");
    al_DeleteBuffers = (LPALDELETEBUFFERS)GetProcAddress(g_OpenALDLL, "alDeleteBuffers");
    al_BufferData = (LPALBUFFERDATA)GetProcAddress(g_OpenALDLL, "alBufferData");
    al_GetBufferi = (LPALGETBUFFERI)GetProcAddress(g_OpenALDLL, "alGetBufferi");

    al_GenSources = (LPALGENSOURCES)GetProcAddress(g_OpenALDLL, "alGenSources");
    al_DeleteSources = (LPALDELETESOURCES)GetProcAddress(g_OpenALDLL, "alDeleteSources");
    al_Sourcei = (LPALSOURCEI)GetProcAddress(g_OpenALDLL, "alSourcei");
    al_Sourcef = (LPALSOURCEF)GetProcAddress(g_OpenALDLL, "alSourcef");
    al_Source3f = (LPALSOURCE3F)GetProcAddress(g_OpenALDLL, "alSource3f");
    al_Sourcefv = (LPALSOURCEFV)GetProcAddress(g_OpenALDLL, "alSourcefv");
    al_GetSourcei = (LPALGETSOURCEI)GetProcAddress(g_OpenALDLL, "alGetSourcei");
    al_GetSourcef = (LPALGETSOURCEF)GetProcAddress(g_OpenALDLL, "alGetSourcef");
    al_GetSource3f = (LPALGETSOURCE3F)GetProcAddress(g_OpenALDLL, "alGetSource3f");
    al_SourcePlay = (LPALSOURCEPLAY)GetProcAddress(g_OpenALDLL, "alSourcePlay");
    al_SourceStop = (LPALSOURCESTOP)GetProcAddress(g_OpenALDLL, "alSourceStop");
    al_SourcePause = (LPALSOURCEPAUSE)GetProcAddress(g_OpenALDLL, "alSourcePause");
    al_SourceRewind = (LPALSOURCEREWIND)GetProcAddress(g_OpenALDLL, "alSourceRewind");
    al_SourceQueueBuffers = (LPALSOURCEQUEUEBUFFERS)GetProcAddress(g_OpenALDLL, "alSourceQueueBuffers");
    al_SourceUnqueueBuffers = (LPALSOURCEUNQUEUEBUFFERS)GetProcAddress(g_OpenALDLL, "alSourceUnqueueBuffers");

    al_Listenerf = (LPALLISTENERF)GetProcAddress(g_OpenALDLL, "alListenerf");
    al_Listener3f = (LPALLISTENER3F)GetProcAddress(g_OpenALDLL, "alListener3f");
    al_Listenerfv = (LPALLISTENERFV)GetProcAddress(g_OpenALDLL, "alListenerfv");
    al_GetListenerf = (LPALGETLISTENERF)GetProcAddress(g_OpenALDLL, "alGetListenerf");
    al_GetListener3f = (LPALGETLISTENER3F)GetProcAddress(g_OpenALDLL, "alGetListener3f");

    al_GetError = (LPALGETERROR)GetProcAddress(g_OpenALDLL, "alGetError");
    al_DopplerFactor = (LPALDOPPLERFACTOR)GetProcAddress(g_OpenALDLL, "alDopplerFactor");
    al_DistanceModel = (LPALDISTANCEMODEL)GetProcAddress(g_OpenALDLL, "alDistanceModel");
    al_SpeedOfSound = (LPALSPEEDOFSOUND)GetProcAddress(g_OpenALDLL, "alSpeedOfSound");

    // Verify critical functions loaded
    if (!alc_OpenDevice || !alc_CreateContext || !alc_MakeContextCurrent ||
        !al_GenBuffers || !al_GenSources || !al_BufferData ||
        !al_SourcePlay || !al_SourceStop) {
        strcpy(g_LastError, "Failed to load required OpenAL functions");
        FreeLibrary(g_OpenALDLL);
        g_OpenALDLL = NULL;
        return false;
    }

    return true;
}

// ============================================================================
// Initialization
// ============================================================================

OAL_S32 OAL_Startup(void)
{
    // TEMPORARILY DISABLED FOR DEBUGGING - audio disabled
    strcpy(g_LastError, "OpenAL disabled for debugging");
    return OAL_ERROR;

#if 0  // Disabled for debugging
    if (g_Initialized) {
        return OAL_NO_ERROR;
    }

    InitializeCriticalSection(&g_AudioLock);

    // Clear sample arrays
    memset(g_Samples2D, 0, sizeof(g_Samples2D));
    memset(g_Samples3D, 0, sizeof(g_Samples3D));
    memset(g_Timers, 0, sizeof(g_Timers));

    // Load OpenAL DLL
    if (!LoadOpenAL()) {
        return OAL_ERROR;
    }
#endif

    // Open default device
    g_Device = alc_OpenDevice(NULL);
    if (!g_Device) {
        strcpy(g_LastError, "Failed to open OpenAL device");
        FreeLibrary(g_OpenALDLL);
        g_OpenALDLL = NULL;
        return OAL_ERROR;
    }

    // Create context
    g_Context = alc_CreateContext(g_Device, NULL);
    if (!g_Context) {
        strcpy(g_LastError, "Failed to create OpenAL context");
        alc_CloseDevice(g_Device);
        g_Device = NULL;
        FreeLibrary(g_OpenALDLL);
        g_OpenALDLL = NULL;
        return OAL_ERROR;
    }

    // Make context current
    if (!alc_MakeContextCurrent(g_Context)) {
        strcpy(g_LastError, "Failed to make OpenAL context current");
        alc_DestroyContext(g_Context);
        g_Context = NULL;
        alc_CloseDevice(g_Device);
        g_Device = NULL;
        FreeLibrary(g_OpenALDLL);
        g_OpenALDLL = NULL;
        return OAL_ERROR;
    }

    // Set default distance model
    if (al_DistanceModel) {
        al_DistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
    }

    g_Initialized = true;
    strcpy(g_LastError, "No error");
    return OAL_NO_ERROR;
}

void OAL_Shutdown(void)
{
    if (!g_Initialized) {
        return;
    }

    EnterCriticalSection(&g_AudioLock);

    // Stop and release all timers
    for (int i = 0; i < OAL_MAX_TIMERS; i++) {
        if (g_Timers[i].in_use) {
            OAL_Stop_Timer(i);
            OAL_Release_Timer(i);
        }
    }

    // Release all 2D samples
    for (OAL_U32 i = 0; i < OAL_MAX_2D_SAMPLES; i++) {
        if (g_Samples2D[i].in_use) {
            if (al_DeleteSources && g_Samples2D[i].source) {
                al_SourceStop(g_Samples2D[i].source);
                al_DeleteSources(1, &g_Samples2D[i].source);
            }
            if (al_DeleteBuffers && g_Samples2D[i].buffer) {
                al_DeleteBuffers(1, &g_Samples2D[i].buffer);
            }
            g_Samples2D[i].in_use = false;
        }
    }

    // Release all 3D samples
    for (OAL_U32 i = 0; i < OAL_MAX_3D_SAMPLES; i++) {
        if (g_Samples3D[i].in_use) {
            if (al_DeleteSources && g_Samples3D[i].source) {
                al_SourceStop(g_Samples3D[i].source);
                al_DeleteSources(1, &g_Samples3D[i].source);
            }
            if (al_DeleteBuffers && g_Samples3D[i].buffer) {
                al_DeleteBuffers(1, &g_Samples3D[i].buffer);
            }
            g_Samples3D[i].in_use = false;
        }
    }

    LeaveCriticalSection(&g_AudioLock);

    // Shutdown OpenAL
    if (alc_MakeContextCurrent) {
        alc_MakeContextCurrent(NULL);
    }
    if (g_Context && alc_DestroyContext) {
        alc_DestroyContext(g_Context);
        g_Context = NULL;
    }
    if (g_Device && alc_CloseDevice) {
        alc_CloseDevice(g_Device);
        g_Device = NULL;
    }
    if (g_OpenALDLL) {
        FreeLibrary(g_OpenALDLL);
        g_OpenALDLL = NULL;
    }

    DeleteCriticalSection(&g_AudioLock);
    g_Initialized = false;
}

const char* OAL_Get_Last_Error(void)
{
    return g_LastError;
}

int OAL_Is_Available(void)
{
    return g_Initialized ? 1 : 0;
}

// ============================================================================
// WAV Parsing
// ============================================================================

#pragma pack(push, 1)
struct RIFFHeader {
    char riff[4];
    unsigned int file_size;
    char wave[4];
};

struct ChunkHeader {
    char id[4];
    unsigned int size;
};

struct FmtChunk {
    unsigned short format;
    unsigned short channels;
    unsigned int sample_rate;
    unsigned int byte_rate;
    unsigned short block_align;
    unsigned short bits_per_sample;
};
#pragma pack(pop)

OAL_S32 OAL_Parse_WAV(const void* file_image, OAL_U32 file_size, OAL_SoundInfo* info)
{
    if (!file_image || !info || file_size < sizeof(RIFFHeader)) {
        return 0;
    }

    memset(info, 0, sizeof(OAL_SoundInfo));

    const unsigned char* data = (const unsigned char*)file_image;
    const RIFFHeader* riff = (const RIFFHeader*)data;

    // Check RIFF header
    if (memcmp(riff->riff, "RIFF", 4) != 0 || memcmp(riff->wave, "WAVE", 4) != 0) {
        return 0;
    }

    data += sizeof(RIFFHeader);
    const unsigned char* end = (const unsigned char*)file_image + file_size;

    bool found_fmt = false;
    bool found_data = false;

    while (data + sizeof(ChunkHeader) <= end) {
        const ChunkHeader* chunk = (const ChunkHeader*)data;
        data += sizeof(ChunkHeader);

        if (data + chunk->size > end) {
            break;
        }

        if (memcmp(chunk->id, "fmt ", 4) == 0) {
            if (chunk->size >= sizeof(FmtChunk)) {
                const FmtChunk* fmt = (const FmtChunk*)data;
                info->format = fmt->format;
                info->channels = fmt->channels;
                info->rate = fmt->sample_rate;
                info->bits = fmt->bits_per_sample;
                info->block_size = fmt->block_align;
                found_fmt = true;
            }
        }
        else if (memcmp(chunk->id, "data", 4) == 0) {
            info->data_ptr = (void*)data;
            info->data_len = chunk->size;
            info->initial_ptr = (void*)file_image;
            found_data = true;
        }

        // Align to word boundary
        OAL_U32 padded_size = (chunk->size + 1) & ~1;
        data += padded_size;
    }

    if (found_fmt && found_data) {
        // Calculate total samples
        if (info->bits > 0 && info->channels > 0) {
            int bytes_per_sample = (info->bits / 8) * info->channels;
            if (bytes_per_sample > 0) {
                info->samples = info->data_len / bytes_per_sample;
            }
        }
        return 1;
    }

    return 0;
}

// ============================================================================
// 2D Sample Functions
// ============================================================================

OAL_HSAMPLE OAL_Allocate_Sample(void)
{
    if (!g_Initialized) return OAL_INVALID_SAMPLE;

    EnterCriticalSection(&g_AudioLock);

    for (OAL_U32 i = 0; i < OAL_MAX_2D_SAMPLES; i++) {
        if (!g_Samples2D[i].in_use) {
            memset(&g_Samples2D[i], 0, sizeof(Sample2D));
            g_Samples2D[i].in_use = true;
            g_Samples2D[i].volume = 127;
            g_Samples2D[i].pan = 64;
            g_Samples2D[i].loop_count = 1;
            g_Samples2D[i].playback_rate = 22050;

            // Create OpenAL source
            if (al_GenSources) {
                al_GenSources(1, &g_Samples2D[i].source);
            }

            LeaveCriticalSection(&g_AudioLock);
            return i;
        }
    }

    LeaveCriticalSection(&g_AudioLock);
    return OAL_INVALID_SAMPLE;
}

void OAL_Release_Sample(OAL_HSAMPLE sample)
{
    if (!g_Initialized || sample >= OAL_MAX_2D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples2D[sample].in_use) {
        if (al_SourceStop && g_Samples2D[sample].source) {
            al_SourceStop(g_Samples2D[sample].source);
        }
        if (al_DeleteSources && g_Samples2D[sample].source) {
            al_DeleteSources(1, &g_Samples2D[sample].source);
        }
        if (al_DeleteBuffers && g_Samples2D[sample].buffer) {
            al_DeleteBuffers(1, &g_Samples2D[sample].buffer);
        }
        g_Samples2D[sample].in_use = false;
    }

    LeaveCriticalSection(&g_AudioLock);
}

void OAL_Init_Sample(OAL_HSAMPLE sample)
{
    if (!g_Initialized || sample >= OAL_MAX_2D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples2D[sample].in_use) {
        // Stop and reset
        if (al_SourceStop && g_Samples2D[sample].source) {
            al_SourceStop(g_Samples2D[sample].source);
            al_Sourcei(g_Samples2D[sample].source, AL_BUFFER, 0);
        }
        // Delete old buffer if exists
        if (al_DeleteBuffers && g_Samples2D[sample].buffer) {
            al_DeleteBuffers(1, &g_Samples2D[sample].buffer);
            g_Samples2D[sample].buffer = 0;
        }
        g_Samples2D[sample].volume = 127;
        g_Samples2D[sample].pan = 64;
        g_Samples2D[sample].loop_count = 1;
        g_Samples2D[sample].paused = false;
    }

    LeaveCriticalSection(&g_AudioLock);
}

OAL_S32 OAL_Set_Sample_File(OAL_HSAMPLE sample, const void* file_image, OAL_U32 file_size, const char* filename)
{
    if (!g_Initialized || sample >= OAL_MAX_2D_SAMPLES || !file_image) return 0;

    EnterCriticalSection(&g_AudioLock);

    if (!g_Samples2D[sample].in_use) {
        LeaveCriticalSection(&g_AudioLock);
        return 0;
    }

    // Parse WAV
    OAL_SoundInfo info;
    if (!OAL_Parse_WAV(file_image, file_size, &info)) {
        LeaveCriticalSection(&g_AudioLock);
        return 0;
    }

    // Only support PCM format
    if (info.format != 1) {  // WAVE_FORMAT_PCM = 1
        LeaveCriticalSection(&g_AudioLock);
        return 0;
    }

    // Determine OpenAL format
    ALenum format;
    if (info.channels == 1) {
        format = (info.bits == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
    } else {
        format = (info.bits == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
    }

    // Delete old buffer if exists
    if (g_Samples2D[sample].buffer) {
        al_Sourcei(g_Samples2D[sample].source, AL_BUFFER, 0);
        al_DeleteBuffers(1, &g_Samples2D[sample].buffer);
    }

    // Create and fill buffer
    al_GenBuffers(1, &g_Samples2D[sample].buffer);
    al_BufferData(g_Samples2D[sample].buffer, format, info.data_ptr, info.data_len, info.rate);

    // Attach buffer to source
    al_Sourcei(g_Samples2D[sample].source, AL_BUFFER, g_Samples2D[sample].buffer);

    // Store info
    g_Samples2D[sample].playback_rate = info.rate;
    g_Samples2D[sample].original_rate = info.rate;
    g_Samples2D[sample].total_samples = info.samples;
    g_Samples2D[sample].bits_per_sample = info.bits;
    g_Samples2D[sample].channels = info.channels;

    // Set as non-3D (relative to listener)
    al_Sourcei(g_Samples2D[sample].source, AL_SOURCE_RELATIVE, AL_TRUE);
    al_Source3f(g_Samples2D[sample].source, AL_POSITION, 0.0f, 0.0f, 0.0f);

    LeaveCriticalSection(&g_AudioLock);
    return 1;
}

static void ApplyPan2D(OAL_HSAMPLE sample)
{
    // Pan: 0=left, 64=center, 127=right
    // Map to OpenAL position: -1 to +1 on X axis
    float pan = (g_Samples2D[sample].pan - 64) / 63.0f;
    if (pan < -1.0f) pan = -1.0f;
    if (pan > 1.0f) pan = 1.0f;
    al_Source3f(g_Samples2D[sample].source, AL_POSITION, pan, 0.0f, 0.0f);
}

void OAL_Start_Sample(OAL_HSAMPLE sample)
{
    if (!g_Initialized || sample >= OAL_MAX_2D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples2D[sample].in_use && g_Samples2D[sample].source) {
        // Apply settings
        float gain = g_Samples2D[sample].volume / 127.0f;
        al_Sourcef(g_Samples2D[sample].source, AL_GAIN, gain);
        ApplyPan2D(sample);

        // Set looping
        al_Sourcei(g_Samples2D[sample].source, AL_LOOPING,
            g_Samples2D[sample].loop_count == 0 ? AL_TRUE : AL_FALSE);

        // Apply pitch for playback rate changes
        if (g_Samples2D[sample].original_rate > 0) {
            float pitch = (float)g_Samples2D[sample].playback_rate / (float)g_Samples2D[sample].original_rate;
            al_Sourcef(g_Samples2D[sample].source, AL_PITCH, pitch);
        }

        al_SourcePlay(g_Samples2D[sample].source);
        g_Samples2D[sample].paused = false;
    }

    LeaveCriticalSection(&g_AudioLock);
}

void OAL_Stop_Sample(OAL_HSAMPLE sample)
{
    if (!g_Initialized || sample >= OAL_MAX_2D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples2D[sample].in_use && g_Samples2D[sample].source) {
        al_SourcePause(g_Samples2D[sample].source);
        g_Samples2D[sample].paused = true;
    }

    LeaveCriticalSection(&g_AudioLock);
}

void OAL_Resume_Sample(OAL_HSAMPLE sample)
{
    if (!g_Initialized || sample >= OAL_MAX_2D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples2D[sample].in_use && g_Samples2D[sample].source && g_Samples2D[sample].paused) {
        al_SourcePlay(g_Samples2D[sample].source);
        g_Samples2D[sample].paused = false;
    }

    LeaveCriticalSection(&g_AudioLock);
}

void OAL_End_Sample(OAL_HSAMPLE sample)
{
    if (!g_Initialized || sample >= OAL_MAX_2D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples2D[sample].in_use && g_Samples2D[sample].source) {
        al_SourceStop(g_Samples2D[sample].source);
        al_SourceRewind(g_Samples2D[sample].source);
        g_Samples2D[sample].paused = false;
    }

    LeaveCriticalSection(&g_AudioLock);
}

OAL_S32 OAL_Sample_Status(OAL_HSAMPLE sample)
{
    if (!g_Initialized || sample >= OAL_MAX_2D_SAMPLES) return OAL_SMP_DONE;

    EnterCriticalSection(&g_AudioLock);

    OAL_S32 status = OAL_SMP_DONE;

    if (g_Samples2D[sample].in_use && g_Samples2D[sample].source) {
        ALint state;
        al_GetSourcei(g_Samples2D[sample].source, AL_SOURCE_STATE, &state);

        switch (state) {
            case AL_INITIAL:
                status = OAL_SMP_STOPPED;
                break;
            case AL_PLAYING:
                status = OAL_SMP_PLAYING;
                break;
            case AL_PAUSED:
                status = OAL_SMP_STOPPED;
                break;
            case AL_STOPPED:
                status = OAL_SMP_DONE;
                break;
            default:
                status = OAL_SMP_FREE;
                break;
        }
    }

    LeaveCriticalSection(&g_AudioLock);
    return status;
}

void OAL_Set_Sample_Volume(OAL_HSAMPLE sample, OAL_S32 volume)
{
    if (!g_Initialized || sample >= OAL_MAX_2D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples2D[sample].in_use) {
        g_Samples2D[sample].volume = (volume < 0) ? 0 : ((volume > 127) ? 127 : volume);
        if (g_Samples2D[sample].source) {
            float gain = g_Samples2D[sample].volume / 127.0f;
            al_Sourcef(g_Samples2D[sample].source, AL_GAIN, gain);
        }
    }

    LeaveCriticalSection(&g_AudioLock);
}

OAL_S32 OAL_Get_Sample_Volume(OAL_HSAMPLE sample)
{
    if (!g_Initialized || sample >= OAL_MAX_2D_SAMPLES) return 127;

    EnterCriticalSection(&g_AudioLock);
    OAL_S32 vol = g_Samples2D[sample].in_use ? g_Samples2D[sample].volume : 127;
    LeaveCriticalSection(&g_AudioLock);
    return vol;
}

void OAL_Set_Sample_Pan(OAL_HSAMPLE sample, OAL_S32 pan)
{
    if (!g_Initialized || sample >= OAL_MAX_2D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples2D[sample].in_use) {
        g_Samples2D[sample].pan = (pan < 0) ? 0 : ((pan > 127) ? 127 : pan);
        if (g_Samples2D[sample].source) {
            ApplyPan2D(sample);
        }
    }

    LeaveCriticalSection(&g_AudioLock);
}

OAL_S32 OAL_Get_Sample_Pan(OAL_HSAMPLE sample)
{
    if (!g_Initialized || sample >= OAL_MAX_2D_SAMPLES) return 64;

    EnterCriticalSection(&g_AudioLock);
    OAL_S32 pan = g_Samples2D[sample].in_use ? g_Samples2D[sample].pan : 64;
    LeaveCriticalSection(&g_AudioLock);
    return pan;
}

void OAL_Set_Sample_Loop_Count(OAL_HSAMPLE sample, OAL_S32 count)
{
    if (!g_Initialized || sample >= OAL_MAX_2D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples2D[sample].in_use) {
        g_Samples2D[sample].loop_count = count;
        if (g_Samples2D[sample].source) {
            al_Sourcei(g_Samples2D[sample].source, AL_LOOPING, count == 0 ? AL_TRUE : AL_FALSE);
        }
    }

    LeaveCriticalSection(&g_AudioLock);
}

OAL_S32 OAL_Get_Sample_Loop_Count(OAL_HSAMPLE sample)
{
    if (!g_Initialized || sample >= OAL_MAX_2D_SAMPLES) return 1;

    EnterCriticalSection(&g_AudioLock);
    OAL_S32 count = g_Samples2D[sample].in_use ? g_Samples2D[sample].loop_count : 1;
    LeaveCriticalSection(&g_AudioLock);
    return count;
}

void OAL_Set_Sample_Playback_Rate(OAL_HSAMPLE sample, OAL_S32 rate)
{
    if (!g_Initialized || sample >= OAL_MAX_2D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples2D[sample].in_use) {
        g_Samples2D[sample].playback_rate = rate;
        if (g_Samples2D[sample].source && g_Samples2D[sample].original_rate > 0) {
            float pitch = (float)rate / (float)g_Samples2D[sample].original_rate;
            al_Sourcef(g_Samples2D[sample].source, AL_PITCH, pitch);
        }
    }

    LeaveCriticalSection(&g_AudioLock);
}

OAL_S32 OAL_Get_Sample_Playback_Rate(OAL_HSAMPLE sample)
{
    if (!g_Initialized || sample >= OAL_MAX_2D_SAMPLES) return 22050;

    EnterCriticalSection(&g_AudioLock);
    OAL_S32 rate = g_Samples2D[sample].in_use ? g_Samples2D[sample].playback_rate : 22050;
    LeaveCriticalSection(&g_AudioLock);
    return rate;
}

void OAL_Set_Sample_MS_Position(OAL_HSAMPLE sample, OAL_S32 ms)
{
    if (!g_Initialized || sample >= OAL_MAX_2D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples2D[sample].in_use && g_Samples2D[sample].source) {
        float seconds = ms / 1000.0f;
        al_Sourcef(g_Samples2D[sample].source, AL_SEC_OFFSET, seconds);
    }

    LeaveCriticalSection(&g_AudioLock);
}

void OAL_Get_Sample_MS_Position(OAL_HSAMPLE sample, OAL_S32* total_ms, OAL_S32* current_ms)
{
    if (total_ms) *total_ms = 0;
    if (current_ms) *current_ms = 0;

    if (!g_Initialized || sample >= OAL_MAX_2D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples2D[sample].in_use && g_Samples2D[sample].source) {
        if (current_ms) {
            float seconds;
            al_GetSourcef(g_Samples2D[sample].source, AL_SEC_OFFSET, &seconds);
            *current_ms = (OAL_S32)(seconds * 1000.0f);
        }
        if (total_ms && g_Samples2D[sample].original_rate > 0) {
            *total_ms = (OAL_S32)((g_Samples2D[sample].total_samples * 1000) / g_Samples2D[sample].original_rate);
        }
    }

    LeaveCriticalSection(&g_AudioLock);
}

void OAL_Set_Sample_User_Data(OAL_HSAMPLE sample, OAL_U32 index, OAL_S32 value)
{
    if (!g_Initialized || sample >= OAL_MAX_2D_SAMPLES || index >= MAX_USER_DATA) return;

    EnterCriticalSection(&g_AudioLock);
    if (g_Samples2D[sample].in_use) {
        g_Samples2D[sample].user_data[index] = value;
    }
    LeaveCriticalSection(&g_AudioLock);
}

OAL_S32 OAL_Get_Sample_User_Data(OAL_HSAMPLE sample, OAL_U32 index)
{
    if (!g_Initialized || sample >= OAL_MAX_2D_SAMPLES || index >= MAX_USER_DATA) return 0;

    EnterCriticalSection(&g_AudioLock);
    OAL_S32 val = g_Samples2D[sample].in_use ? g_Samples2D[sample].user_data[index] : 0;
    LeaveCriticalSection(&g_AudioLock);
    return val;
}

// ============================================================================
// 3D Sample Functions
// ============================================================================

OAL_H3DSAMPLE OAL_Allocate_3D_Sample(void)
{
    if (!g_Initialized) return OAL_INVALID_3D_SAMPLE;

    EnterCriticalSection(&g_AudioLock);

    for (OAL_U32 i = 0; i < OAL_MAX_3D_SAMPLES; i++) {
        if (!g_Samples3D[i].in_use) {
            memset(&g_Samples3D[i], 0, sizeof(Sample3D));
            g_Samples3D[i].in_use = true;
            g_Samples3D[i].volume = 127;
            g_Samples3D[i].loop_count = 1;
            g_Samples3D[i].playback_rate = 22050;
            g_Samples3D[i].min_dist = 1.0f;
            g_Samples3D[i].max_dist = 1000.0f;

            if (al_GenSources) {
                al_GenSources(1, &g_Samples3D[i].source);
                // 3D sources are NOT relative to listener
                al_Sourcei(g_Samples3D[i].source, AL_SOURCE_RELATIVE, AL_FALSE);
            }

            LeaveCriticalSection(&g_AudioLock);
            return i;
        }
    }

    LeaveCriticalSection(&g_AudioLock);
    return OAL_INVALID_3D_SAMPLE;
}

void OAL_Release_3D_Sample(OAL_H3DSAMPLE sample)
{
    if (!g_Initialized || sample >= OAL_MAX_3D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples3D[sample].in_use) {
        if (al_SourceStop && g_Samples3D[sample].source) {
            al_SourceStop(g_Samples3D[sample].source);
        }
        if (al_DeleteSources && g_Samples3D[sample].source) {
            al_DeleteSources(1, &g_Samples3D[sample].source);
        }
        if (al_DeleteBuffers && g_Samples3D[sample].buffer) {
            al_DeleteBuffers(1, &g_Samples3D[sample].buffer);
        }
        g_Samples3D[sample].in_use = false;
    }

    LeaveCriticalSection(&g_AudioLock);
}

OAL_S32 OAL_Set_3D_Sample_File(OAL_H3DSAMPLE sample, const void* file_image, OAL_U32 file_size)
{
    if (!g_Initialized || sample >= OAL_MAX_3D_SAMPLES || !file_image) return 0;

    EnterCriticalSection(&g_AudioLock);

    if (!g_Samples3D[sample].in_use) {
        LeaveCriticalSection(&g_AudioLock);
        return 0;
    }

    OAL_SoundInfo info;
    if (!OAL_Parse_WAV(file_image, file_size, &info)) {
        LeaveCriticalSection(&g_AudioLock);
        return 0;
    }

    // Only support PCM, and for 3D audio, only mono
    if (info.format != 1 || info.channels != 1) {
        LeaveCriticalSection(&g_AudioLock);
        return 0;
    }

    ALenum format = (info.bits == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;

    if (g_Samples3D[sample].buffer) {
        al_Sourcei(g_Samples3D[sample].source, AL_BUFFER, 0);
        al_DeleteBuffers(1, &g_Samples3D[sample].buffer);
    }

    al_GenBuffers(1, &g_Samples3D[sample].buffer);
    al_BufferData(g_Samples3D[sample].buffer, format, info.data_ptr, info.data_len, info.rate);
    al_Sourcei(g_Samples3D[sample].source, AL_BUFFER, g_Samples3D[sample].buffer);

    g_Samples3D[sample].playback_rate = info.rate;
    g_Samples3D[sample].original_rate = info.rate;
    g_Samples3D[sample].total_samples = info.samples;
    g_Samples3D[sample].bits_per_sample = info.bits;
    g_Samples3D[sample].channels = info.channels;

    LeaveCriticalSection(&g_AudioLock);
    return 1;
}

void OAL_Start_3D_Sample(OAL_H3DSAMPLE sample)
{
    if (!g_Initialized || sample >= OAL_MAX_3D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples3D[sample].in_use && g_Samples3D[sample].source) {
        float gain = g_Samples3D[sample].volume / 127.0f;
        al_Sourcef(g_Samples3D[sample].source, AL_GAIN, gain);
        al_Sourcei(g_Samples3D[sample].source, AL_LOOPING,
            g_Samples3D[sample].loop_count == 0 ? AL_TRUE : AL_FALSE);

        if (g_Samples3D[sample].original_rate > 0) {
            float pitch = (float)g_Samples3D[sample].playback_rate / (float)g_Samples3D[sample].original_rate;
            al_Sourcef(g_Samples3D[sample].source, AL_PITCH, pitch);
        }

        al_Sourcef(g_Samples3D[sample].source, AL_REFERENCE_DISTANCE, g_Samples3D[sample].min_dist);
        al_Sourcef(g_Samples3D[sample].source, AL_MAX_DISTANCE, g_Samples3D[sample].max_dist);

        al_SourcePlay(g_Samples3D[sample].source);
        g_Samples3D[sample].paused = false;
    }

    LeaveCriticalSection(&g_AudioLock);
}

void OAL_Stop_3D_Sample(OAL_H3DSAMPLE sample)
{
    if (!g_Initialized || sample >= OAL_MAX_3D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples3D[sample].in_use && g_Samples3D[sample].source) {
        al_SourcePause(g_Samples3D[sample].source);
        g_Samples3D[sample].paused = true;
    }

    LeaveCriticalSection(&g_AudioLock);
}

void OAL_Resume_3D_Sample(OAL_H3DSAMPLE sample)
{
    if (!g_Initialized || sample >= OAL_MAX_3D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples3D[sample].in_use && g_Samples3D[sample].source && g_Samples3D[sample].paused) {
        al_SourcePlay(g_Samples3D[sample].source);
        g_Samples3D[sample].paused = false;
    }

    LeaveCriticalSection(&g_AudioLock);
}

void OAL_End_3D_Sample(OAL_H3DSAMPLE sample)
{
    if (!g_Initialized || sample >= OAL_MAX_3D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples3D[sample].in_use && g_Samples3D[sample].source) {
        al_SourceStop(g_Samples3D[sample].source);
        al_SourceRewind(g_Samples3D[sample].source);
        g_Samples3D[sample].paused = false;
    }

    LeaveCriticalSection(&g_AudioLock);
}

OAL_S32 OAL_3D_Sample_Status(OAL_H3DSAMPLE sample)
{
    if (!g_Initialized || sample >= OAL_MAX_3D_SAMPLES) return OAL_SMP_DONE;

    EnterCriticalSection(&g_AudioLock);

    OAL_S32 status = OAL_SMP_DONE;

    if (g_Samples3D[sample].in_use && g_Samples3D[sample].source) {
        ALint state;
        al_GetSourcei(g_Samples3D[sample].source, AL_SOURCE_STATE, &state);

        switch (state) {
            case AL_INITIAL: status = OAL_SMP_STOPPED; break;
            case AL_PLAYING: status = OAL_SMP_PLAYING; break;
            case AL_PAUSED: status = OAL_SMP_STOPPED; break;
            case AL_STOPPED: status = OAL_SMP_DONE; break;
            default: status = OAL_SMP_FREE; break;
        }
    }

    LeaveCriticalSection(&g_AudioLock);
    return status;
}

void OAL_Set_3D_Sample_Volume(OAL_H3DSAMPLE sample, OAL_S32 volume)
{
    if (!g_Initialized || sample >= OAL_MAX_3D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples3D[sample].in_use) {
        g_Samples3D[sample].volume = (volume < 0) ? 0 : ((volume > 127) ? 127 : volume);
        if (g_Samples3D[sample].source) {
            float gain = g_Samples3D[sample].volume / 127.0f;
            al_Sourcef(g_Samples3D[sample].source, AL_GAIN, gain);
        }
    }

    LeaveCriticalSection(&g_AudioLock);
}

OAL_S32 OAL_Get_3D_Sample_Volume(OAL_H3DSAMPLE sample)
{
    if (!g_Initialized || sample >= OAL_MAX_3D_SAMPLES) return 127;

    EnterCriticalSection(&g_AudioLock);
    OAL_S32 vol = g_Samples3D[sample].in_use ? g_Samples3D[sample].volume : 127;
    LeaveCriticalSection(&g_AudioLock);
    return vol;
}

void OAL_Set_3D_Sample_Loop_Count(OAL_H3DSAMPLE sample, OAL_S32 count)
{
    if (!g_Initialized || sample >= OAL_MAX_3D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples3D[sample].in_use) {
        g_Samples3D[sample].loop_count = count;
        if (g_Samples3D[sample].source) {
            al_Sourcei(g_Samples3D[sample].source, AL_LOOPING, count == 0 ? AL_TRUE : AL_FALSE);
        }
    }

    LeaveCriticalSection(&g_AudioLock);
}

OAL_S32 OAL_Get_3D_Sample_Loop_Count(OAL_H3DSAMPLE sample)
{
    if (!g_Initialized || sample >= OAL_MAX_3D_SAMPLES) return 1;

    EnterCriticalSection(&g_AudioLock);
    OAL_S32 count = g_Samples3D[sample].in_use ? g_Samples3D[sample].loop_count : 1;
    LeaveCriticalSection(&g_AudioLock);
    return count;
}

void OAL_Set_3D_Sample_Playback_Rate(OAL_H3DSAMPLE sample, OAL_S32 rate)
{
    if (!g_Initialized || sample >= OAL_MAX_3D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples3D[sample].in_use) {
        g_Samples3D[sample].playback_rate = rate;
        if (g_Samples3D[sample].source && g_Samples3D[sample].original_rate > 0) {
            float pitch = (float)rate / (float)g_Samples3D[sample].original_rate;
            al_Sourcef(g_Samples3D[sample].source, AL_PITCH, pitch);
        }
    }

    LeaveCriticalSection(&g_AudioLock);
}

OAL_S32 OAL_Get_3D_Sample_Playback_Rate(OAL_H3DSAMPLE sample)
{
    if (!g_Initialized || sample >= OAL_MAX_3D_SAMPLES) return 22050;

    EnterCriticalSection(&g_AudioLock);
    OAL_S32 rate = g_Samples3D[sample].in_use ? g_Samples3D[sample].playback_rate : 22050;
    LeaveCriticalSection(&g_AudioLock);
    return rate;
}

void OAL_Set_3D_Sample_Position(OAL_H3DSAMPLE sample, OAL_F32 x, OAL_F32 y, OAL_F32 z)
{
    if (!g_Initialized || sample >= OAL_MAX_3D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples3D[sample].in_use && g_Samples3D[sample].source) {
        g_Samples3D[sample].pos_x = x;
        g_Samples3D[sample].pos_y = y;
        g_Samples3D[sample].pos_z = z;
        al_Source3f(g_Samples3D[sample].source, AL_POSITION, x, y, z);
    }

    LeaveCriticalSection(&g_AudioLock);
}

void OAL_Get_3D_Sample_Position(OAL_H3DSAMPLE sample, OAL_F32* x, OAL_F32* y, OAL_F32* z)
{
    if (x) *x = 0;
    if (y) *y = 0;
    if (z) *z = 0;

    if (!g_Initialized || sample >= OAL_MAX_3D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples3D[sample].in_use) {
        if (x) *x = g_Samples3D[sample].pos_x;
        if (y) *y = g_Samples3D[sample].pos_y;
        if (z) *z = g_Samples3D[sample].pos_z;
    }

    LeaveCriticalSection(&g_AudioLock);
}

void OAL_Set_3D_Sample_Velocity(OAL_H3DSAMPLE sample, OAL_F32 dx, OAL_F32 dy, OAL_F32 dz)
{
    if (!g_Initialized || sample >= OAL_MAX_3D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples3D[sample].in_use && g_Samples3D[sample].source) {
        al_Source3f(g_Samples3D[sample].source, AL_VELOCITY, dx, dy, dz);
    }

    LeaveCriticalSection(&g_AudioLock);
}

void OAL_Set_3D_Sample_Distances(OAL_H3DSAMPLE sample, OAL_F32 max_dist, OAL_F32 min_dist)
{
    if (!g_Initialized || sample >= OAL_MAX_3D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples3D[sample].in_use) {
        g_Samples3D[sample].min_dist = min_dist;
        g_Samples3D[sample].max_dist = max_dist;
        if (g_Samples3D[sample].source) {
            al_Sourcef(g_Samples3D[sample].source, AL_REFERENCE_DISTANCE, min_dist);
            al_Sourcef(g_Samples3D[sample].source, AL_MAX_DISTANCE, max_dist);
        }
    }

    LeaveCriticalSection(&g_AudioLock);
}

void OAL_Set_3D_Sample_User_Data(OAL_H3DSAMPLE sample, OAL_U32 index, OAL_S32 value)
{
    if (!g_Initialized || sample >= OAL_MAX_3D_SAMPLES || index >= MAX_USER_DATA) return;

    EnterCriticalSection(&g_AudioLock);
    if (g_Samples3D[sample].in_use) {
        g_Samples3D[sample].user_data[index] = value;
    }
    LeaveCriticalSection(&g_AudioLock);
}

OAL_S32 OAL_Get_3D_Sample_User_Data(OAL_H3DSAMPLE sample, OAL_U32 index)
{
    if (!g_Initialized || sample >= OAL_MAX_3D_SAMPLES || index >= MAX_USER_DATA) return 0;

    EnterCriticalSection(&g_AudioLock);
    OAL_S32 val = g_Samples3D[sample].in_use ? g_Samples3D[sample].user_data[index] : 0;
    LeaveCriticalSection(&g_AudioLock);
    return val;
}

void OAL_Set_3D_Sample_Offset(OAL_H3DSAMPLE sample, OAL_U32 offset)
{
    if (!g_Initialized || sample >= OAL_MAX_3D_SAMPLES) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Samples3D[sample].in_use && g_Samples3D[sample].source) {
        al_Sourcei(g_Samples3D[sample].source, AL_SAMPLE_OFFSET, offset);
    }

    LeaveCriticalSection(&g_AudioLock);
}

OAL_U32 OAL_Get_3D_Sample_Offset(OAL_H3DSAMPLE sample)
{
    if (!g_Initialized || sample >= OAL_MAX_3D_SAMPLES) return 0;

    EnterCriticalSection(&g_AudioLock);

    OAL_U32 offset = 0;
    if (g_Samples3D[sample].in_use && g_Samples3D[sample].source) {
        ALint off;
        al_GetSourcei(g_Samples3D[sample].source, AL_SAMPLE_OFFSET, &off);
        offset = (OAL_U32)off;
    }

    LeaveCriticalSection(&g_AudioLock);
    return offset;
}

OAL_U32 OAL_Get_3D_Sample_Length(OAL_H3DSAMPLE sample)
{
    if (!g_Initialized || sample >= OAL_MAX_3D_SAMPLES) return 0;

    EnterCriticalSection(&g_AudioLock);
    OAL_U32 len = g_Samples3D[sample].in_use ? g_Samples3D[sample].total_samples : 0;
    LeaveCriticalSection(&g_AudioLock);
    return len;
}

// ============================================================================
// Listener Functions
// ============================================================================

void OAL_Set_Listener_Position(OAL_F32 x, OAL_F32 y, OAL_F32 z)
{
    if (!g_Initialized || !al_Listener3f) return;
    al_Listener3f(AL_POSITION, x, y, z);
}

void OAL_Get_Listener_Position(OAL_F32* x, OAL_F32* y, OAL_F32* z)
{
    if (!g_Initialized || !al_GetListener3f) {
        if (x) *x = 0;
        if (y) *y = 0;
        if (z) *z = 0;
        return;
    }
    float fx = 0, fy = 0, fz = 0;
    al_GetListener3f(AL_POSITION, &fx, &fy, &fz);
    if (x) *x = fx;
    if (y) *y = fy;
    if (z) *z = fz;
}

void OAL_Set_Listener_Orientation(OAL_F32 fx, OAL_F32 fy, OAL_F32 fz, OAL_F32 ux, OAL_F32 uy, OAL_F32 uz)
{
    if (!g_Initialized || !al_Listenerfv) return;
    float orientation[6] = { fx, fy, fz, ux, uy, uz };
    al_Listenerfv(AL_ORIENTATION, orientation);
}

void OAL_Set_Listener_Velocity(OAL_F32 dx, OAL_F32 dy, OAL_F32 dz)
{
    if (!g_Initialized || !al_Listener3f) return;
    al_Listener3f(AL_VELOCITY, dx, dy, dz);
}

void OAL_Set_Doppler_Factor(OAL_F32 factor)
{
    if (!g_Initialized || !al_DopplerFactor) return;
    al_DopplerFactor(factor);
}

void OAL_Set_Distance_Factor(OAL_F32 factor)
{
    // OpenAL uses speed of sound for this
    if (!g_Initialized || !al_SpeedOfSound) return;
    // Miles uses meters, OpenAL uses meters by default
    // Speed of sound in air: ~343 m/s
    al_SpeedOfSound(343.0f * factor);
}

void OAL_Set_Rolloff_Factor(OAL_F32 factor)
{
    // Applied per-source in OpenAL, but we can set a global hint
    // This would need to be applied to each source
    (void)factor;
}

// ============================================================================
// Streaming (basic implementation - streams from file)
// ============================================================================

// For now, streaming loads the entire file into a buffer
// A full implementation would stream chunks

struct StreamData {
    bool in_use;
    ALuint source;
    ALuint buffer;
    OAL_S32 volume;
    OAL_S32 pan;
    OAL_S32 loop_count;
    bool paused;
    void* file_data;
    OAL_U32 file_size;
    OAL_U32 total_samples;
    OAL_U32 sample_rate;
};

static StreamData g_Streams[OAL_MAX_STREAMS];

OAL_HSTREAM OAL_Open_Stream(const char* filename)
{
    if (!g_Initialized || !filename) return OAL_INVALID_STREAM;

    // Try to open file
    FILE* f = fopen(filename, "rb");
    if (!f && g_FileOpen) {
        // Try using file callbacks
        OAL_U32 handle = 0;
        if (g_FileOpen(filename, &handle) && handle) {
            // Read file using callbacks
            // For now, just return invalid
            return OAL_INVALID_STREAM;
        }
    }
    if (!f) return OAL_INVALID_STREAM;

    // Get file size
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0) {
        fclose(f);
        return OAL_INVALID_STREAM;
    }

    // Find free stream slot
    EnterCriticalSection(&g_AudioLock);

    OAL_HSTREAM stream_id = OAL_INVALID_STREAM;
    for (OAL_U32 i = 0; i < OAL_MAX_STREAMS; i++) {
        if (!g_Streams[i].in_use) {
            stream_id = i;
            break;
        }
    }

    if (stream_id == OAL_INVALID_STREAM) {
        LeaveCriticalSection(&g_AudioLock);
        fclose(f);
        return OAL_INVALID_STREAM;
    }

    // Allocate and read file
    void* data = malloc(size);
    if (!data) {
        LeaveCriticalSection(&g_AudioLock);
        fclose(f);
        return OAL_INVALID_STREAM;
    }

    if (fread(data, 1, size, f) != (size_t)size) {
        free(data);
        LeaveCriticalSection(&g_AudioLock);
        fclose(f);
        return OAL_INVALID_STREAM;
    }
    fclose(f);

    // Parse WAV
    OAL_SoundInfo info;
    if (!OAL_Parse_WAV(data, size, &info) || info.format != 1) {
        free(data);
        LeaveCriticalSection(&g_AudioLock);
        return OAL_INVALID_STREAM;
    }

    // Setup stream
    memset(&g_Streams[stream_id], 0, sizeof(StreamData));
    g_Streams[stream_id].in_use = true;
    g_Streams[stream_id].volume = 127;
    g_Streams[stream_id].pan = 64;
    g_Streams[stream_id].loop_count = 1;
    g_Streams[stream_id].file_data = data;
    g_Streams[stream_id].file_size = size;
    g_Streams[stream_id].total_samples = info.samples;
    g_Streams[stream_id].sample_rate = info.rate;

    // Create OpenAL objects
    al_GenSources(1, &g_Streams[stream_id].source);
    al_GenBuffers(1, &g_Streams[stream_id].buffer);

    ALenum format;
    if (info.channels == 1) {
        format = (info.bits == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
    } else {
        format = (info.bits == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
    }

    al_BufferData(g_Streams[stream_id].buffer, format, info.data_ptr, info.data_len, info.rate);
    al_Sourcei(g_Streams[stream_id].source, AL_BUFFER, g_Streams[stream_id].buffer);
    al_Sourcei(g_Streams[stream_id].source, AL_SOURCE_RELATIVE, AL_TRUE);

    LeaveCriticalSection(&g_AudioLock);
    return stream_id;
}

void OAL_Close_Stream(OAL_HSTREAM stream)
{
    if (!g_Initialized || stream >= OAL_MAX_STREAMS) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Streams[stream].in_use) {
        if (g_Streams[stream].source) {
            al_SourceStop(g_Streams[stream].source);
            al_DeleteSources(1, &g_Streams[stream].source);
        }
        if (g_Streams[stream].buffer) {
            al_DeleteBuffers(1, &g_Streams[stream].buffer);
        }
        if (g_Streams[stream].file_data) {
            free(g_Streams[stream].file_data);
        }
        g_Streams[stream].in_use = false;
    }

    LeaveCriticalSection(&g_AudioLock);
}

void OAL_Start_Stream(OAL_HSTREAM stream)
{
    if (!g_Initialized || stream >= OAL_MAX_STREAMS) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Streams[stream].in_use && g_Streams[stream].source) {
        float gain = g_Streams[stream].volume / 127.0f;
        al_Sourcef(g_Streams[stream].source, AL_GAIN, gain);
        al_Sourcei(g_Streams[stream].source, AL_LOOPING,
            g_Streams[stream].loop_count == 0 ? AL_TRUE : AL_FALSE);
        al_SourcePlay(g_Streams[stream].source);
        g_Streams[stream].paused = false;
    }

    LeaveCriticalSection(&g_AudioLock);
}

void OAL_Stop_Stream(OAL_HSTREAM stream)
{
    if (!g_Initialized || stream >= OAL_MAX_STREAMS) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Streams[stream].in_use && g_Streams[stream].source) {
        al_SourceStop(g_Streams[stream].source);
        g_Streams[stream].paused = false;
    }

    LeaveCriticalSection(&g_AudioLock);
}

void OAL_Pause_Stream(OAL_HSTREAM stream, OAL_S32 pause)
{
    if (!g_Initialized || stream >= OAL_MAX_STREAMS) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Streams[stream].in_use && g_Streams[stream].source) {
        if (pause) {
            al_SourcePause(g_Streams[stream].source);
            g_Streams[stream].paused = true;
        } else {
            al_SourcePlay(g_Streams[stream].source);
            g_Streams[stream].paused = false;
        }
    }

    LeaveCriticalSection(&g_AudioLock);
}

OAL_S32 OAL_Stream_Status(OAL_HSTREAM stream)
{
    if (!g_Initialized || stream >= OAL_MAX_STREAMS) return OAL_SMP_DONE;

    EnterCriticalSection(&g_AudioLock);

    OAL_S32 status = OAL_SMP_DONE;
    if (g_Streams[stream].in_use && g_Streams[stream].source) {
        ALint state;
        al_GetSourcei(g_Streams[stream].source, AL_SOURCE_STATE, &state);
        switch (state) {
            case AL_PLAYING: status = OAL_SMP_PLAYING; break;
            case AL_PAUSED: status = OAL_SMP_STOPPED; break;
            case AL_STOPPED: status = OAL_SMP_DONE; break;
            default: status = OAL_SMP_FREE; break;
        }
    }

    LeaveCriticalSection(&g_AudioLock);
    return status;
}

void OAL_Set_Stream_Volume(OAL_HSTREAM stream, OAL_S32 volume)
{
    if (!g_Initialized || stream >= OAL_MAX_STREAMS) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Streams[stream].in_use) {
        g_Streams[stream].volume = (volume < 0) ? 0 : ((volume > 127) ? 127 : volume);
        if (g_Streams[stream].source) {
            float gain = g_Streams[stream].volume / 127.0f;
            al_Sourcef(g_Streams[stream].source, AL_GAIN, gain);
        }
    }

    LeaveCriticalSection(&g_AudioLock);
}

OAL_S32 OAL_Get_Stream_Volume(OAL_HSTREAM stream)
{
    if (!g_Initialized || stream >= OAL_MAX_STREAMS) return 127;

    EnterCriticalSection(&g_AudioLock);
    OAL_S32 vol = g_Streams[stream].in_use ? g_Streams[stream].volume : 127;
    LeaveCriticalSection(&g_AudioLock);
    return vol;
}

void OAL_Set_Stream_Pan(OAL_HSTREAM stream, OAL_S32 pan)
{
    if (!g_Initialized || stream >= OAL_MAX_STREAMS) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Streams[stream].in_use) {
        g_Streams[stream].pan = (pan < 0) ? 0 : ((pan > 127) ? 127 : pan);
        if (g_Streams[stream].source) {
            float p = (g_Streams[stream].pan - 64) / 63.0f;
            al_Source3f(g_Streams[stream].source, AL_POSITION, p, 0.0f, 0.0f);
        }
    }

    LeaveCriticalSection(&g_AudioLock);
}

OAL_S32 OAL_Get_Stream_Pan(OAL_HSTREAM stream)
{
    if (!g_Initialized || stream >= OAL_MAX_STREAMS) return 64;

    EnterCriticalSection(&g_AudioLock);
    OAL_S32 pan = g_Streams[stream].in_use ? g_Streams[stream].pan : 64;
    LeaveCriticalSection(&g_AudioLock);
    return pan;
}

void OAL_Set_Stream_Loop_Count(OAL_HSTREAM stream, OAL_S32 count)
{
    if (!g_Initialized || stream >= OAL_MAX_STREAMS) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Streams[stream].in_use) {
        g_Streams[stream].loop_count = count;
        if (g_Streams[stream].source) {
            al_Sourcei(g_Streams[stream].source, AL_LOOPING, count == 0 ? AL_TRUE : AL_FALSE);
        }
    }

    LeaveCriticalSection(&g_AudioLock);
}

OAL_S32 OAL_Get_Stream_Loop_Count(OAL_HSTREAM stream)
{
    if (!g_Initialized || stream >= OAL_MAX_STREAMS) return 1;

    EnterCriticalSection(&g_AudioLock);
    OAL_S32 count = g_Streams[stream].in_use ? g_Streams[stream].loop_count : 1;
    LeaveCriticalSection(&g_AudioLock);
    return count;
}

void OAL_Set_Stream_MS_Position(OAL_HSTREAM stream, OAL_S32 ms)
{
    if (!g_Initialized || stream >= OAL_MAX_STREAMS) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Streams[stream].in_use && g_Streams[stream].source) {
        float seconds = ms / 1000.0f;
        al_Sourcef(g_Streams[stream].source, AL_SEC_OFFSET, seconds);
    }

    LeaveCriticalSection(&g_AudioLock);
}

void OAL_Get_Stream_MS_Position(OAL_HSTREAM stream, OAL_S32* total, OAL_S32* current)
{
    if (total) *total = 0;
    if (current) *current = 0;

    if (!g_Initialized || stream >= OAL_MAX_STREAMS) return;

    EnterCriticalSection(&g_AudioLock);

    if (g_Streams[stream].in_use && g_Streams[stream].source) {
        if (current) {
            float seconds;
            al_GetSourcef(g_Streams[stream].source, AL_SEC_OFFSET, &seconds);
            *current = (OAL_S32)(seconds * 1000.0f);
        }
        if (total && g_Streams[stream].sample_rate > 0) {
            *total = (OAL_S32)((g_Streams[stream].total_samples * 1000) / g_Streams[stream].sample_rate);
        }
    }

    LeaveCriticalSection(&g_AudioLock);
}

void OAL_Service_Stream(OAL_HSTREAM stream)
{
    // For the current implementation (full buffer), nothing to do
    (void)stream;
}

// ============================================================================
// File Callbacks
// ============================================================================

void OAL_Set_File_Callbacks(
    OAL_FileOpenCallback open_cb,
    OAL_FileCloseCallback close_cb,
    OAL_FileSeekCallback seek_cb,
    OAL_FileReadCallback read_cb)
{
    g_FileOpen = open_cb;
    g_FileClose = close_cb;
    g_FileSeek = seek_cb;
    g_FileRead = read_cb;
}

// ============================================================================
// Timer Functions
// ============================================================================

static DWORD WINAPI TimerThreadProc(LPVOID param)
{
    OALTimer* timer = (OALTimer*)param;

    while (timer->running) {
        DWORD wait_ms = timer->period_us / 1000;
        if (wait_ms < 1) wait_ms = 1;

        DWORD result = WaitForSingleObject(timer->stop_event, wait_ms);
        if (result == WAIT_OBJECT_0) {
            break;  // Stop requested
        }

        if (timer->callback && timer->running) {
            timer->callback(timer->user_data);
        }
    }

    return 0;
}

OAL_HTIMER OAL_Register_Timer(OAL_TimerCallback callback)
{
    if (!callback) return OAL_INVALID_TIMER;

    for (OAL_U32 i = 0; i < OAL_MAX_TIMERS; i++) {
        if (!g_Timers[i].in_use) {
            memset(&g_Timers[i], 0, sizeof(OALTimer));
            g_Timers[i].in_use = true;
            g_Timers[i].callback = callback;
            g_Timers[i].period_us = 1000;  // Default 1ms
            g_Timers[i].stop_event = CreateEvent(NULL, TRUE, FALSE, NULL);
            return i;
        }
    }

    return OAL_INVALID_TIMER;
}

void OAL_Release_Timer(OAL_HTIMER timer)
{
    if (timer >= OAL_MAX_TIMERS || !g_Timers[timer].in_use) return;

    OAL_Stop_Timer(timer);

    if (g_Timers[timer].stop_event) {
        CloseHandle(g_Timers[timer].stop_event);
    }

    g_Timers[timer].in_use = false;
}

void OAL_Set_Timer_Period(OAL_HTIMER timer, OAL_U32 microseconds)
{
    if (timer >= OAL_MAX_TIMERS || !g_Timers[timer].in_use) return;
    g_Timers[timer].period_us = microseconds;
}

void OAL_Set_Timer_User(OAL_HTIMER timer, OAL_U32 user_data)
{
    if (timer >= OAL_MAX_TIMERS || !g_Timers[timer].in_use) return;
    g_Timers[timer].user_data = user_data;
}

void OAL_Start_Timer(OAL_HTIMER timer)
{
    if (timer >= OAL_MAX_TIMERS || !g_Timers[timer].in_use) return;
    if (g_Timers[timer].running) return;

    ResetEvent(g_Timers[timer].stop_event);
    g_Timers[timer].running = true;
    g_Timers[timer].thread = CreateThread(NULL, 0, TimerThreadProc, &g_Timers[timer], 0, NULL);
}

void OAL_Stop_Timer(OAL_HTIMER timer)
{
    if (timer >= OAL_MAX_TIMERS || !g_Timers[timer].in_use) return;
    if (!g_Timers[timer].running) return;

    g_Timers[timer].running = false;
    SetEvent(g_Timers[timer].stop_event);

    if (g_Timers[timer].thread) {
        WaitForSingleObject(g_Timers[timer].thread, 1000);
        CloseHandle(g_Timers[timer].thread);
        g_Timers[timer].thread = NULL;
    }
}

// ============================================================================
// Utility
// ============================================================================

void OAL_Delay(OAL_S32 milliseconds)
{
    Sleep(milliseconds);
}

OAL_U32 OAL_Microseconds(void)
{
    static LARGE_INTEGER freq = {0};
    if (freq.QuadPart == 0) {
        QueryPerformanceFrequency(&freq);
    }

    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);

    return (OAL_U32)((count.QuadPart * 1000000) / freq.QuadPart);
}
