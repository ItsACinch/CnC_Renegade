/*
** OpenAL Audio Backend Implementation
**
** Uses OpenAL Soft for audio playback as a replacement for Miles Sound System.
** OpenAL is loaded dynamically so the game can still run without it (silent).
*/

#include "openal_audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// ============================================================================
// OpenAL Constants and Types (from al.h and alc.h)
// ============================================================================

// Boolean
#define AL_FALSE    0
#define AL_TRUE     1

// Source/Listener properties
#define AL_POSITION                 0x1004
#define AL_VELOCITY                 0x1006
#define AL_ORIENTATION              0x100F
#define AL_GAIN                     0x100A
#define AL_BUFFER                   0x1009
#define AL_SOURCE_STATE             0x1010
#define AL_LOOPING                  0x1007
#define AL_PITCH                    0x1003
#define AL_BUFFERS_QUEUED           0x1015
#define AL_BUFFERS_PROCESSED        0x1016
#define AL_SEC_OFFSET               0x1024
#define AL_BYTE_OFFSET              0x1026
#define AL_SAMPLE_OFFSET            0x1025
#define AL_SOURCE_RELATIVE          0x0202
#define AL_REFERENCE_DISTANCE       0x1020
#define AL_MAX_DISTANCE             0x1023
#define AL_ROLLOFF_FACTOR           0x1021

// Source state
#define AL_INITIAL                  0x1011
#define AL_PLAYING                  0x1012
#define AL_PAUSED                   0x1013
#define AL_STOPPED                  0x1014

// Buffer formats
#define AL_FORMAT_MONO8             0x1100
#define AL_FORMAT_MONO16            0x1101
#define AL_FORMAT_STEREO8           0x1102
#define AL_FORMAT_STEREO16          0x1103

// Errors
#define AL_NO_ERROR                 0
#define AL_INVALID_NAME             0xA001
#define AL_INVALID_ENUM             0xA002
#define AL_INVALID_VALUE            0xA003
#define AL_INVALID_OPERATION        0xA004
#define AL_OUT_OF_MEMORY            0xA005

// Distance models
#define AL_INVERSE_DISTANCE_CLAMPED 0xD002
#define AL_DOPPLER_FACTOR           0xC000
#define AL_SPEED_OF_SOUND           0xC003

// ALC
#define ALC_FALSE                   0
#define ALC_TRUE                    1
#define ALC_DEFAULT_DEVICE_SPECIFIER 0x1004
#define ALC_DEVICE_SPECIFIER        0x1005

// ============================================================================
// OpenAL Function Pointers (loaded dynamically)
// ============================================================================

// ALC functions
typedef ALCdevice* (*LPALCOPENDEVICE)(const char*);
typedef ALboolean (*LPALCCLOSEDEVICE)(ALCdevice*);
typedef ALCcontext* (*LPALCCREATECONTEXT)(ALCdevice*, const ALint*);
typedef ALboolean (*LPALCMAKECONTEXTCURRENT)(ALCcontext*);
typedef void (*LPALCDESTROYCONTEXT)(ALCcontext*);
typedef ALCcontext* (*LPALCGETCURRENTCONTEXT)(void);
typedef const char* (*LPALCGETSTRING)(ALCdevice*, ALenum);
typedef ALenum (*LPALCGETERROR)(ALCdevice*);

// AL source functions
typedef void (*LPALGENSOURCES)(ALint, ALuint*);
typedef void (*LPALDELETESOURCES)(ALint, const ALuint*);
typedef void (*LPALSOURCEI)(ALuint, ALenum, ALint);
typedef void (*LPALSOURCEF)(ALuint, ALenum, ALfloat);
typedef void (*LPALSOURCE3F)(ALuint, ALenum, ALfloat, ALfloat, ALfloat);
typedef void (*LPALSOURCEFV)(ALuint, ALenum, const ALfloat*);
typedef void (*LPALGETSOURCEI)(ALuint, ALenum, ALint*);
typedef void (*LPALGETSOURCEF)(ALuint, ALenum, ALfloat*);
typedef void (*LPALGETSOURCE3F)(ALuint, ALenum, ALfloat*, ALfloat*, ALfloat*);
typedef void (*LPALSOURCEPLAY)(ALuint);
typedef void (*LPALSOURCESTOP)(ALuint);
typedef void (*LPALSOURCEPAUSE)(ALuint);
typedef void (*LPALSOURCEREWIND)(ALuint);
typedef void (*LPALSOURCEQUEUEBUFFERS)(ALuint, ALint, const ALuint*);
typedef void (*LPALSOURCEUNQUEUEBUFFERS)(ALuint, ALint, ALuint*);

// AL buffer functions
typedef void (*LPALGENBUFFERS)(ALint, ALuint*);
typedef void (*LPALDELETEBUFFERS)(ALint, const ALuint*);
typedef void (*LPALBUFFERDATA)(ALuint, ALenum, const void*, ALint, ALint);
typedef void (*LPALGETBUFFERI)(ALuint, ALenum, ALint*);

// AL listener functions
typedef void (*LPALLISTENERF)(ALenum, ALfloat);
typedef void (*LPALLISTENER3F)(ALenum, ALfloat, ALfloat, ALfloat);
typedef void (*LPALLISTENERFV)(ALenum, const ALfloat*);

// AL global functions
typedef ALenum (*LPALGETERROR)(void);
typedef void (*LPALDISTANCEMODEL)(ALenum);
typedef void (*LPALDOPPLERFACTOR)(ALfloat);
typedef void (*LPALSPEEDOFSOUND)(ALfloat);

// Function pointer storage
static struct {
    HMODULE hOpenAL;

    // ALC
    LPALCOPENDEVICE alcOpenDevice;
    LPALCCLOSEDEVICE alcCloseDevice;
    LPALCCREATECONTEXT alcCreateContext;
    LPALCMAKECONTEXTCURRENT alcMakeContextCurrent;
    LPALCDESTROYCONTEXT alcDestroyContext;
    LPALCGETCURRENTCONTEXT alcGetCurrentContext;
    LPALCGETSTRING alcGetString;
    LPALCGETERROR alcGetError;

    // AL Source
    LPALGENSOURCES alGenSources;
    LPALDELETESOURCES alDeleteSources;
    LPALSOURCEI alSourcei;
    LPALSOURCEF alSourcef;
    LPALSOURCE3F alSource3f;
    LPALSOURCEFV alSourcefv;
    LPALGETSOURCEI alGetSourcei;
    LPALGETSOURCEF alGetSourcef;
    LPALGETSOURCE3F alGetSource3f;
    LPALSOURCEPLAY alSourcePlay;
    LPALSOURCESTOP alSourceStop;
    LPALSOURCEPAUSE alSourcePause;
    LPALSOURCEREWIND alSourceRewind;
    LPALSOURCEQUEUEBUFFERS alSourceQueueBuffers;
    LPALSOURCEUNQUEUEBUFFERS alSourceUnqueueBuffers;

    // AL Buffer
    LPALGENBUFFERS alGenBuffers;
    LPALDELETEBUFFERS alDeleteBuffers;
    LPALBUFFERDATA alBufferData;
    LPALGETBUFFERI alGetBufferi;

    // AL Listener
    LPALLISTENERF alListenerf;
    LPALLISTENER3F alListener3f;
    LPALLISTENERFV alListenerfv;

    // AL Global
    LPALGETERROR alGetError;
    LPALDISTANCEMODEL alDistanceModel;
    LPALDOPPLERFACTOR alDopplerFactor;
    LPALSPEEDOFSOUND alSpeedOfSound;
} g_AL = {0};

// ============================================================================
// Global State
// ============================================================================

static ALCdevice* g_Device = NULL;
static ALCcontext* g_Context = NULL;
static int g_Initialized = 0;
static char g_LastError[256] = "No error";
static OAL_S32 g_MasterVolume = 127;
static OAL_S32 g_SpeakerType = OAL_SPEAKER_2;

// Maximum samples/streams
#define MAX_SAMPLES     64
#define MAX_3D_SAMPLES  64
#define MAX_STREAMS     8
#define STREAM_BUFFER_SIZE 32768
#define STREAM_BUFFER_COUNT 4
#define MAX_USER_DATA   4

// ============================================================================
// Sample Structure
// ============================================================================

struct OAL_Sample {
    ALuint source;
    ALuint buffer;
    int in_use;
    int status;
    OAL_S32 volume;
    OAL_S32 pan;
    OAL_S32 loop_count;
    OAL_S32 playback_rate;
    OAL_S32 base_rate;
    OAL_S32 total_ms;
    OAL_S32 user_data[MAX_USER_DATA];
};

struct OAL_Sample3D {
    ALuint source;
    ALuint buffer;
    int in_use;
    int status;
    OAL_S32 volume;
    OAL_S32 loop_count;
    OAL_S32 playback_rate;
    OAL_S32 base_rate;
    OAL_F32 pos_x, pos_y, pos_z;
    OAL_F32 vel_x, vel_y, vel_z;
    OAL_F32 min_dist, max_dist;
    OAL_F32 effects_level;
    OAL_U32 data_length;
    OAL_S32 user_data[MAX_USER_DATA];
};

struct OAL_Stream {
    ALuint source;
    ALuint buffers[STREAM_BUFFER_COUNT];
    FILE* file;
    int in_use;
    int status;
    int paused;
    OAL_S32 volume;
    OAL_S32 pan;
    OAL_S32 loop_count;
    int loops_remaining;
    OAL_S32 playback_rate;
    OAL_S32 base_rate;
    OAL_S32 total_ms;
    OAL_S32 current_ms;

    // WAV info
    long data_start;
    long data_size;
    long data_pos;
    int format;
    int channels;
    int bits;
    int rate;

    OAL_S32 user_data[MAX_USER_DATA];
};

// Sample/Stream pools
static OAL_Sample g_Samples[MAX_SAMPLES];
static OAL_Sample3D g_3DSamples[MAX_3D_SAMPLES];
static OAL_Stream g_Streams[MAX_STREAMS];

// ============================================================================
// Helper Functions
// ============================================================================

static void SetError(const char* error) {
    strncpy(g_LastError, error, sizeof(g_LastError) - 1);
    g_LastError[sizeof(g_LastError) - 1] = '\0';
}

static ALenum GetALFormat(int channels, int bits) {
    if (channels == 1) {
        return (bits == 16) ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8;
    } else {
        return (bits == 16) ? AL_FORMAT_STEREO16 : AL_FORMAT_STEREO8;
    }
}

static float VolumeToGain(OAL_S32 volume) {
    // Convert 0-127 to 0.0-1.0, apply master volume
    float gain = (volume / 127.0f) * (g_MasterVolume / 127.0f);
    return gain;
}

static void ApplyPan(ALuint source, OAL_S32 pan) {
    // Pan: 0=left, 64=center, 127=right
    // OpenAL uses position for panning in 2D
    float x = (pan - 64) / 64.0f;  // -1 to +1
    if (g_AL.alSource3f) {
        g_AL.alSource3f(source, AL_POSITION, x, 0.0f, -1.0f);
        g_AL.alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
    }
}

static int UpdateSampleStatus(OAL_Sample* sample) {
    if (!sample || !sample->in_use) return OAL_SMP_FREE;

    ALint state;
    g_AL.alGetSourcei(sample->source, AL_SOURCE_STATE, &state);

    switch (state) {
        case AL_PLAYING: sample->status = OAL_SMP_PLAYING; break;
        case AL_PAUSED:  sample->status = OAL_SMP_STOPPED; break;
        case AL_STOPPED:
        case AL_INITIAL: sample->status = OAL_SMP_DONE; break;
        default:         sample->status = OAL_SMP_DONE; break;
    }

    return sample->status;
}

static int Update3DSampleStatus(OAL_Sample3D* sample) {
    if (!sample || !sample->in_use) return OAL_SMP_FREE;

    ALint state;
    g_AL.alGetSourcei(sample->source, AL_SOURCE_STATE, &state);

    switch (state) {
        case AL_PLAYING: sample->status = OAL_SMP_PLAYING; break;
        case AL_PAUSED:  sample->status = OAL_SMP_STOPPED; break;
        case AL_STOPPED:
        case AL_INITIAL: sample->status = OAL_SMP_DONE; break;
        default:         sample->status = OAL_SMP_DONE; break;
    }

    return sample->status;
}

// ============================================================================
// Core System Functions
// ============================================================================

OAL_S32 OAL_Startup(void) {
    if (g_Initialized) return OAL_OK;

    // Try to load OpenAL Soft
    g_AL.hOpenAL = LoadLibraryA("OpenAL32.dll");
    if (!g_AL.hOpenAL) {
        g_AL.hOpenAL = LoadLibraryA("soft_oal.dll");
    }

    if (!g_AL.hOpenAL) {
        SetError("Failed to load OpenAL32.dll or soft_oal.dll");
        return OAL_ERROR;
    }

    // Load ALC functions
    #define LOAD_ALC(name) g_AL.name = (decltype(g_AL.name))GetProcAddress(g_AL.hOpenAL, #name)
    LOAD_ALC(alcOpenDevice);
    LOAD_ALC(alcCloseDevice);
    LOAD_ALC(alcCreateContext);
    LOAD_ALC(alcMakeContextCurrent);
    LOAD_ALC(alcDestroyContext);
    LOAD_ALC(alcGetCurrentContext);
    LOAD_ALC(alcGetString);
    LOAD_ALC(alcGetError);
    #undef LOAD_ALC

    // Load AL functions
    #define LOAD_AL(name) g_AL.name = (decltype(g_AL.name))GetProcAddress(g_AL.hOpenAL, #name)
    LOAD_AL(alGenSources);
    LOAD_AL(alDeleteSources);
    LOAD_AL(alSourcei);
    LOAD_AL(alSourcef);
    LOAD_AL(alSource3f);
    LOAD_AL(alSourcefv);
    LOAD_AL(alGetSourcei);
    LOAD_AL(alGetSourcef);
    LOAD_AL(alGetSource3f);
    LOAD_AL(alSourcePlay);
    LOAD_AL(alSourceStop);
    LOAD_AL(alSourcePause);
    LOAD_AL(alSourceRewind);
    LOAD_AL(alSourceQueueBuffers);
    LOAD_AL(alSourceUnqueueBuffers);
    LOAD_AL(alGenBuffers);
    LOAD_AL(alDeleteBuffers);
    LOAD_AL(alBufferData);
    LOAD_AL(alGetBufferi);
    LOAD_AL(alListenerf);
    LOAD_AL(alListener3f);
    LOAD_AL(alListenerfv);
    LOAD_AL(alGetError);
    LOAD_AL(alDistanceModel);
    LOAD_AL(alDopplerFactor);
    LOAD_AL(alSpeedOfSound);
    #undef LOAD_AL

    // Verify critical functions loaded
    if (!g_AL.alcOpenDevice || !g_AL.alcCreateContext || !g_AL.alGenSources) {
        SetError("Failed to load required OpenAL functions");
        FreeLibrary(g_AL.hOpenAL);
        g_AL.hOpenAL = NULL;
        return OAL_ERROR;
    }

    // Open default device
    g_Device = g_AL.alcOpenDevice(NULL);
    if (!g_Device) {
        SetError("Failed to open OpenAL device");
        FreeLibrary(g_AL.hOpenAL);
        g_AL.hOpenAL = NULL;
        return OAL_ERROR;
    }

    // Create context
    g_Context = g_AL.alcCreateContext(g_Device, NULL);
    if (!g_Context) {
        SetError("Failed to create OpenAL context");
        g_AL.alcCloseDevice(g_Device);
        g_Device = NULL;
        FreeLibrary(g_AL.hOpenAL);
        g_AL.hOpenAL = NULL;
        return OAL_ERROR;
    }

    g_AL.alcMakeContextCurrent(g_Context);

    // Set default distance model
    if (g_AL.alDistanceModel) {
        g_AL.alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
    }

    // Initialize sample pools
    memset(g_Samples, 0, sizeof(g_Samples));
    memset(g_3DSamples, 0, sizeof(g_3DSamples));
    memset(g_Streams, 0, sizeof(g_Streams));

    g_Initialized = 1;
    SetError("OpenAL initialized successfully");

    return OAL_OK;
}

void OAL_Shutdown(void) {
    if (!g_Initialized) return;

    // Release all samples
    for (int i = 0; i < MAX_SAMPLES; i++) {
        if (g_Samples[i].in_use) {
            OAL_Release_Sample(&g_Samples[i]);
        }
    }

    for (int i = 0; i < MAX_3D_SAMPLES; i++) {
        if (g_3DSamples[i].in_use) {
            OAL_Release_3D_Sample(&g_3DSamples[i]);
        }
    }

    for (int i = 0; i < MAX_STREAMS; i++) {
        if (g_Streams[i].in_use) {
            OAL_Close_Stream(&g_Streams[i]);
        }
    }

    // Destroy context
    if (g_Context) {
        g_AL.alcMakeContextCurrent(NULL);
        g_AL.alcDestroyContext(g_Context);
        g_Context = NULL;
    }

    // Close device
    if (g_Device) {
        g_AL.alcCloseDevice(g_Device);
        g_Device = NULL;
    }

    // Unload library
    if (g_AL.hOpenAL) {
        FreeLibrary(g_AL.hOpenAL);
        memset(&g_AL, 0, sizeof(g_AL));
    }

    g_Initialized = 0;
}

OAL_S32 OAL_Is_Ready(void) {
    return g_Initialized;
}

const char* OAL_Last_Error(void) {
    return g_LastError;
}

void OAL_Update(void) {
    if (!g_Initialized) return;

    // Update all active streams
    for (int i = 0; i < MAX_STREAMS; i++) {
        if (g_Streams[i].in_use && g_Streams[i].status == OAL_SMP_PLAYING) {
            OAL_Service_Stream(&g_Streams[i]);
        }
    }
}

// ============================================================================
// 2D Sample Functions
// ============================================================================

OAL_Sample* OAL_Allocate_Sample(void) {
    if (!g_Initialized) return NULL;

    for (int i = 0; i < MAX_SAMPLES; i++) {
        if (!g_Samples[i].in_use) {
            OAL_Sample* sample = &g_Samples[i];
            memset(sample, 0, sizeof(OAL_Sample));

            g_AL.alGenSources(1, &sample->source);
            if (g_AL.alGetError() != AL_NO_ERROR) {
                return NULL;
            }

            sample->in_use = 1;
            sample->status = OAL_SMP_DONE;
            sample->volume = 127;
            sample->pan = 64;
            sample->loop_count = 1;
            sample->playback_rate = 22050;
            sample->base_rate = 22050;

            return sample;
        }
    }

    return NULL;
}

void OAL_Release_Sample(OAL_Sample* sample) {
    if (!sample || !sample->in_use) return;

    g_AL.alSourceStop(sample->source);
    g_AL.alSourcei(sample->source, AL_BUFFER, 0);

    if (sample->buffer) {
        g_AL.alDeleteBuffers(1, &sample->buffer);
    }
    g_AL.alDeleteSources(1, &sample->source);

    memset(sample, 0, sizeof(OAL_Sample));
}

void OAL_Init_Sample(OAL_Sample* sample) {
    if (!sample || !sample->in_use) return;

    g_AL.alSourceStop(sample->source);
    g_AL.alSourcei(sample->source, AL_BUFFER, 0);

    if (sample->buffer) {
        g_AL.alDeleteBuffers(1, &sample->buffer);
        sample->buffer = 0;
    }

    sample->status = OAL_SMP_DONE;
    sample->volume = 127;
    sample->pan = 64;
    sample->loop_count = 1;
}

OAL_S32 OAL_Set_Sample_File(OAL_Sample* sample, void* file_data, OAL_S32 file_size) {
    if (!sample || !sample->in_use || !file_data) return OAL_ERROR;

    OAL_SoundInfo info;
    if (OAL_WAV_Info(file_data, &info) != OAL_OK) {
        return OAL_ERROR;
    }

    // Create buffer if needed
    if (!sample->buffer) {
        g_AL.alGenBuffers(1, &sample->buffer);
        if (g_AL.alGetError() != AL_NO_ERROR) {
            return OAL_ERROR;
        }
    }

    // Upload data
    ALenum format = GetALFormat(info.channels, info.bits);
    g_AL.alBufferData(sample->buffer, format, info.data_ptr, info.data_len, info.rate);

    if (g_AL.alGetError() != AL_NO_ERROR) {
        return OAL_ERROR;
    }

    // Attach to source
    g_AL.alSourcei(sample->source, AL_BUFFER, sample->buffer);

    sample->base_rate = info.rate;
    sample->playback_rate = info.rate;
    sample->total_ms = (int)((info.samples * 1000) / info.rate);
    sample->status = OAL_SMP_DONE;

    return OAL_OK;
}

void OAL_Start_Sample(OAL_Sample* sample) {
    if (!sample || !sample->in_use) return;

    // Apply settings
    g_AL.alSourcef(sample->source, AL_GAIN, VolumeToGain(sample->volume));
    ApplyPan(sample->source, sample->pan);
    g_AL.alSourcei(sample->source, AL_LOOPING, (sample->loop_count == 0) ? AL_TRUE : AL_FALSE);
    g_AL.alSourcef(sample->source, AL_PITCH, (float)sample->playback_rate / sample->base_rate);

    g_AL.alSourcePlay(sample->source);
    sample->status = OAL_SMP_PLAYING;
}

void OAL_Stop_Sample(OAL_Sample* sample) {
    if (!sample || !sample->in_use) return;
    g_AL.alSourcePause(sample->source);
    sample->status = OAL_SMP_STOPPED;
}

void OAL_Resume_Sample(OAL_Sample* sample) {
    if (!sample || !sample->in_use) return;
    g_AL.alSourcePlay(sample->source);
    sample->status = OAL_SMP_PLAYING;
}

void OAL_End_Sample(OAL_Sample* sample) {
    if (!sample || !sample->in_use) return;
    g_AL.alSourceStop(sample->source);
    g_AL.alSourceRewind(sample->source);
    sample->status = OAL_SMP_DONE;
}

OAL_S32 OAL_Sample_Status(OAL_Sample* sample) {
    return UpdateSampleStatus(sample);
}

void OAL_Set_Sample_Volume(OAL_Sample* sample, OAL_S32 volume) {
    if (!sample || !sample->in_use) return;
    sample->volume = (volume < 0) ? 0 : ((volume > 127) ? 127 : volume);
    g_AL.alSourcef(sample->source, AL_GAIN, VolumeToGain(sample->volume));
}

OAL_S32 OAL_Sample_Volume(OAL_Sample* sample) {
    return sample ? sample->volume : 127;
}

void OAL_Set_Sample_Pan(OAL_Sample* sample, OAL_S32 pan) {
    if (!sample || !sample->in_use) return;
    sample->pan = (pan < 0) ? 0 : ((pan > 127) ? 127 : pan);
    ApplyPan(sample->source, sample->pan);
}

OAL_S32 OAL_Sample_Pan(OAL_Sample* sample) {
    return sample ? sample->pan : 64;
}

void OAL_Set_Sample_Loop_Count(OAL_Sample* sample, OAL_S32 count) {
    if (!sample || !sample->in_use) return;
    sample->loop_count = count;
    g_AL.alSourcei(sample->source, AL_LOOPING, (count == 0) ? AL_TRUE : AL_FALSE);
}

OAL_S32 OAL_Sample_Loop_Count(OAL_Sample* sample) {
    return sample ? sample->loop_count : 1;
}

void OAL_Set_Sample_Playback_Rate(OAL_Sample* sample, OAL_S32 rate) {
    if (!sample || !sample->in_use || rate <= 0) return;
    sample->playback_rate = rate;
    if (sample->base_rate > 0) {
        g_AL.alSourcef(sample->source, AL_PITCH, (float)rate / sample->base_rate);
    }
}

OAL_S32 OAL_Sample_Playback_Rate(OAL_Sample* sample) {
    return sample ? sample->playback_rate : 22050;
}

void OAL_Set_Sample_MS_Position(OAL_Sample* sample, OAL_S32 ms) {
    if (!sample || !sample->in_use) return;
    float seconds = ms / 1000.0f;
    g_AL.alSourcef(sample->source, AL_SEC_OFFSET, seconds);
}

void OAL_Sample_MS_Position(OAL_Sample* sample, OAL_S32* total_ms, OAL_S32* current_ms) {
    if (!sample) {
        if (total_ms) *total_ms = 0;
        if (current_ms) *current_ms = 0;
        return;
    }

    if (total_ms) *total_ms = sample->total_ms;

    if (current_ms && sample->in_use) {
        float seconds = 0;
        g_AL.alGetSourcef(sample->source, AL_SEC_OFFSET, &seconds);
        *current_ms = (OAL_S32)(seconds * 1000);
    } else if (current_ms) {
        *current_ms = 0;
    }
}

void OAL_Set_Sample_User_Data(OAL_Sample* sample, OAL_U32 index, OAL_S32 value) {
    if (!sample || index >= MAX_USER_DATA) return;
    sample->user_data[index] = value;
}

OAL_S32 OAL_Sample_User_Data(OAL_Sample* sample, OAL_U32 index) {
    if (!sample || index >= MAX_USER_DATA) return 0;
    return sample->user_data[index];
}

// ============================================================================
// 3D Sample Functions
// ============================================================================

OAL_Sample3D* OAL_Allocate_3D_Sample(void) {
    if (!g_Initialized) return NULL;

    for (int i = 0; i < MAX_3D_SAMPLES; i++) {
        if (!g_3DSamples[i].in_use) {
            OAL_Sample3D* sample = &g_3DSamples[i];
            memset(sample, 0, sizeof(OAL_Sample3D));

            g_AL.alGenSources(1, &sample->source);
            if (g_AL.alGetError() != AL_NO_ERROR) {
                return NULL;
            }

            sample->in_use = 1;
            sample->status = OAL_SMP_DONE;
            sample->volume = 127;
            sample->loop_count = 1;
            sample->playback_rate = 22050;
            sample->base_rate = 22050;
            sample->min_dist = 1.0f;
            sample->max_dist = 1000.0f;

            // 3D sources should not be relative
            g_AL.alSourcei(sample->source, AL_SOURCE_RELATIVE, AL_FALSE);

            return sample;
        }
    }

    return NULL;
}

void OAL_Release_3D_Sample(OAL_Sample3D* sample) {
    if (!sample || !sample->in_use) return;

    g_AL.alSourceStop(sample->source);
    g_AL.alSourcei(sample->source, AL_BUFFER, 0);

    if (sample->buffer) {
        g_AL.alDeleteBuffers(1, &sample->buffer);
    }
    g_AL.alDeleteSources(1, &sample->source);

    memset(sample, 0, sizeof(OAL_Sample3D));
}

OAL_S32 OAL_Set_3D_Sample_File(OAL_Sample3D* sample, void* file_data, OAL_S32 file_size) {
    if (!sample || !sample->in_use || !file_data) return OAL_ERROR;

    OAL_SoundInfo info;
    if (OAL_WAV_Info(file_data, &info) != OAL_OK) {
        return OAL_ERROR;
    }

    // 3D audio should be mono
    if (info.channels != 1) {
        // Still allow it but warn
    }

    if (!sample->buffer) {
        g_AL.alGenBuffers(1, &sample->buffer);
        if (g_AL.alGetError() != AL_NO_ERROR) {
            return OAL_ERROR;
        }
    }

    ALenum format = GetALFormat(info.channels, info.bits);
    g_AL.alBufferData(sample->buffer, format, info.data_ptr, info.data_len, info.rate);

    if (g_AL.alGetError() != AL_NO_ERROR) {
        return OAL_ERROR;
    }

    g_AL.alSourcei(sample->source, AL_BUFFER, sample->buffer);

    sample->base_rate = info.rate;
    sample->playback_rate = info.rate;
    sample->data_length = info.data_len;
    sample->status = OAL_SMP_DONE;

    return OAL_OK;
}

void OAL_Start_3D_Sample(OAL_Sample3D* sample) {
    if (!sample || !sample->in_use) return;

    g_AL.alSourcef(sample->source, AL_GAIN, VolumeToGain(sample->volume));
    g_AL.alSourcei(sample->source, AL_LOOPING, (sample->loop_count == 0) ? AL_TRUE : AL_FALSE);
    g_AL.alSourcef(sample->source, AL_PITCH, (float)sample->playback_rate / sample->base_rate);
    g_AL.alSourcef(sample->source, AL_REFERENCE_DISTANCE, sample->min_dist);
    g_AL.alSourcef(sample->source, AL_MAX_DISTANCE, sample->max_dist);
    g_AL.alSource3f(sample->source, AL_POSITION, sample->pos_x, sample->pos_y, sample->pos_z);
    g_AL.alSource3f(sample->source, AL_VELOCITY, sample->vel_x, sample->vel_y, sample->vel_z);

    g_AL.alSourcePlay(sample->source);
    sample->status = OAL_SMP_PLAYING;
}

void OAL_Stop_3D_Sample(OAL_Sample3D* sample) {
    if (!sample || !sample->in_use) return;
    g_AL.alSourcePause(sample->source);
    sample->status = OAL_SMP_STOPPED;
}

void OAL_Resume_3D_Sample(OAL_Sample3D* sample) {
    if (!sample || !sample->in_use) return;
    g_AL.alSourcePlay(sample->source);
    sample->status = OAL_SMP_PLAYING;
}

void OAL_End_3D_Sample(OAL_Sample3D* sample) {
    if (!sample || !sample->in_use) return;
    g_AL.alSourceStop(sample->source);
    g_AL.alSourceRewind(sample->source);
    sample->status = OAL_SMP_DONE;
}

OAL_S32 OAL_3D_Sample_Status(OAL_Sample3D* sample) {
    return Update3DSampleStatus(sample);
}

void OAL_Set_3D_Sample_Volume(OAL_Sample3D* sample, OAL_S32 volume) {
    if (!sample || !sample->in_use) return;
    sample->volume = (volume < 0) ? 0 : ((volume > 127) ? 127 : volume);
    g_AL.alSourcef(sample->source, AL_GAIN, VolumeToGain(sample->volume));
}

OAL_S32 OAL_3D_Sample_Volume(OAL_Sample3D* sample) {
    return sample ? sample->volume : 127;
}

void OAL_Set_3D_Sample_Loop_Count(OAL_Sample3D* sample, OAL_S32 count) {
    if (!sample || !sample->in_use) return;
    sample->loop_count = count;
    g_AL.alSourcei(sample->source, AL_LOOPING, (count == 0) ? AL_TRUE : AL_FALSE);
}

OAL_S32 OAL_3D_Sample_Loop_Count(OAL_Sample3D* sample) {
    return sample ? sample->loop_count : 1;
}

void OAL_Set_3D_Sample_Playback_Rate(OAL_Sample3D* sample, OAL_S32 rate) {
    if (!sample || !sample->in_use || rate <= 0) return;
    sample->playback_rate = rate;
    if (sample->base_rate > 0) {
        g_AL.alSourcef(sample->source, AL_PITCH, (float)rate / sample->base_rate);
    }
}

OAL_S32 OAL_3D_Sample_Playback_Rate(OAL_Sample3D* sample) {
    return sample ? sample->playback_rate : 22050;
}

void OAL_Set_3D_Sample_Position(OAL_Sample3D* sample, OAL_F32 x, OAL_F32 y, OAL_F32 z) {
    if (!sample || !sample->in_use) return;
    sample->pos_x = x;
    sample->pos_y = y;
    sample->pos_z = z;
    g_AL.alSource3f(sample->source, AL_POSITION, x, y, z);
}

void OAL_3D_Sample_Position(OAL_Sample3D* sample, OAL_F32* x, OAL_F32* y, OAL_F32* z) {
    if (!sample) {
        if (x) *x = 0;
        if (y) *y = 0;
        if (z) *z = 0;
        return;
    }
    if (x) *x = sample->pos_x;
    if (y) *y = sample->pos_y;
    if (z) *z = sample->pos_z;
}

void OAL_Set_3D_Sample_Velocity(OAL_Sample3D* sample, OAL_F32 dx, OAL_F32 dy, OAL_F32 dz) {
    if (!sample || !sample->in_use) return;
    sample->vel_x = dx;
    sample->vel_y = dy;
    sample->vel_z = dz;
    g_AL.alSource3f(sample->source, AL_VELOCITY, dx, dy, dz);
}

void OAL_Set_3D_Sample_Distances(OAL_Sample3D* sample, OAL_F32 max_dist, OAL_F32 min_dist) {
    if (!sample || !sample->in_use) return;
    sample->min_dist = min_dist;
    sample->max_dist = max_dist;
    g_AL.alSourcef(sample->source, AL_REFERENCE_DISTANCE, min_dist);
    g_AL.alSourcef(sample->source, AL_MAX_DISTANCE, max_dist);
}

void OAL_Set_3D_Sample_Effects_Level(OAL_Sample3D* sample, OAL_F32 level) {
    if (!sample) return;
    sample->effects_level = level;
    // OpenAL EFX would be needed for proper reverb send
}

void OAL_Set_3D_Sample_User_Data(OAL_Sample3D* sample, OAL_U32 index, OAL_S32 value) {
    if (!sample || index >= MAX_USER_DATA) return;
    sample->user_data[index] = value;
}

OAL_S32 OAL_3D_Sample_User_Data(OAL_Sample3D* sample, OAL_U32 index) {
    if (!sample || index >= MAX_USER_DATA) return 0;
    return sample->user_data[index];
}

void OAL_Set_3D_Sample_Offset(OAL_Sample3D* sample, OAL_U32 offset) {
    if (!sample || !sample->in_use) return;
    g_AL.alSourcei(sample->source, AL_BYTE_OFFSET, offset);
}

OAL_U32 OAL_3D_Sample_Offset(OAL_Sample3D* sample) {
    if (!sample || !sample->in_use) return 0;
    ALint offset = 0;
    g_AL.alGetSourcei(sample->source, AL_BYTE_OFFSET, &offset);
    return (OAL_U32)offset;
}

OAL_U32 OAL_3D_Sample_Length(OAL_Sample3D* sample) {
    return sample ? sample->data_length : 0;
}

// ============================================================================
// Listener Functions
// ============================================================================

void OAL_Set_Listener_Position(OAL_F32 x, OAL_F32 y, OAL_F32 z) {
    if (!g_Initialized) return;
    g_AL.alListener3f(AL_POSITION, x, y, z);
}

void OAL_Set_Listener_Orientation(OAL_F32 fx, OAL_F32 fy, OAL_F32 fz,
                                   OAL_F32 ux, OAL_F32 uy, OAL_F32 uz) {
    if (!g_Initialized) return;
    ALfloat orientation[6] = { fx, fy, fz, ux, uy, uz };
    g_AL.alListenerfv(AL_ORIENTATION, orientation);
}

void OAL_Set_Listener_Velocity(OAL_F32 dx, OAL_F32 dy, OAL_F32 dz) {
    if (!g_Initialized) return;
    g_AL.alListener3f(AL_VELOCITY, dx, dy, dz);
}

void OAL_Set_Speaker_Type(OAL_S32 type) {
    g_SpeakerType = type;
    // OpenAL doesn't have direct speaker type control
    // HRTF can be enabled via alcCreateContext attributes if needed
}

OAL_S32 OAL_Speaker_Type(void) {
    return g_SpeakerType;
}

void OAL_Set_Doppler_Factor(OAL_F32 factor) {
    if (!g_Initialized || !g_AL.alDopplerFactor) return;
    g_AL.alDopplerFactor(factor);
}

void OAL_Set_Distance_Factor(OAL_F32 factor) {
    if (!g_Initialized || !g_AL.alSpeedOfSound) return;
    // Distance factor affects speed of sound in OpenAL
    // Default is 343.3 (meters per second)
    g_AL.alSpeedOfSound(343.3f * factor);
}

void OAL_Set_Rolloff_Factor(OAL_F32 factor) {
    // This is set per-source in OpenAL
    // We could store it globally and apply to new sources
}

// ============================================================================
// Streaming Functions
// ============================================================================

OAL_Stream* OAL_Open_Stream(const char* filename) {
    if (!g_Initialized || !filename) return NULL;

    // Find free stream slot
    OAL_Stream* stream = NULL;
    for (int i = 0; i < MAX_STREAMS; i++) {
        if (!g_Streams[i].in_use) {
            stream = &g_Streams[i];
            break;
        }
    }

    if (!stream) return NULL;

    memset(stream, 0, sizeof(OAL_Stream));

    // Open file
    stream->file = fopen(filename, "rb");
    if (!stream->file) {
        return NULL;
    }

    // Parse WAV header
    char riff[4];
    fread(riff, 1, 4, stream->file);
    if (memcmp(riff, "RIFF", 4) != 0) {
        fclose(stream->file);
        return NULL;
    }

    fseek(stream->file, 4, SEEK_CUR); // Skip file size

    char wave[4];
    fread(wave, 1, 4, stream->file);
    if (memcmp(wave, "WAVE", 4) != 0) {
        fclose(stream->file);
        return NULL;
    }

    // Find fmt chunk
    while (!feof(stream->file)) {
        char chunk_id[4];
        unsigned int chunk_size;

        if (fread(chunk_id, 1, 4, stream->file) != 4) break;
        if (fread(&chunk_size, 4, 1, stream->file) != 1) break;

        if (memcmp(chunk_id, "fmt ", 4) == 0) {
            unsigned short format, channels, bits;
            unsigned int rate;

            fread(&format, 2, 1, stream->file);
            fread(&channels, 2, 1, stream->file);
            fread(&rate, 4, 1, stream->file);
            fseek(stream->file, 6, SEEK_CUR); // Skip byte rate and block align
            fread(&bits, 2, 1, stream->file);

            stream->format = format;
            stream->channels = channels;
            stream->rate = rate;
            stream->bits = bits;

            // Skip rest of fmt chunk
            if (chunk_size > 16) {
                fseek(stream->file, chunk_size - 16, SEEK_CUR);
            }
        }
        else if (memcmp(chunk_id, "data", 4) == 0) {
            stream->data_start = ftell(stream->file);
            stream->data_size = chunk_size;
            break;
        }
        else {
            fseek(stream->file, chunk_size, SEEK_CUR);
        }
    }

    if (stream->data_size == 0) {
        fclose(stream->file);
        return NULL;
    }

    // Create source and buffers
    g_AL.alGenSources(1, &stream->source);
    g_AL.alGenBuffers(STREAM_BUFFER_COUNT, stream->buffers);

    stream->in_use = 1;
    stream->status = OAL_SMP_DONE;
    stream->volume = 127;
    stream->pan = 64;
    stream->loop_count = 1;
    stream->loops_remaining = 1;
    stream->playback_rate = stream->rate;
    stream->base_rate = stream->rate;
    stream->data_pos = 0;

    int bytes_per_sample = (stream->bits / 8) * stream->channels;
    int samples = stream->data_size / bytes_per_sample;
    stream->total_ms = (samples * 1000) / stream->rate;

    // Make source relative for 2D playback
    g_AL.alSourcei(stream->source, AL_SOURCE_RELATIVE, AL_TRUE);

    return stream;
}

void OAL_Close_Stream(OAL_Stream* stream) {
    if (!stream || !stream->in_use) return;

    g_AL.alSourceStop(stream->source);

    // Unqueue all buffers
    ALint queued;
    g_AL.alGetSourcei(stream->source, AL_BUFFERS_QUEUED, &queued);
    while (queued > 0) {
        ALuint buffer;
        g_AL.alSourceUnqueueBuffers(stream->source, 1, &buffer);
        queued--;
    }

    g_AL.alDeleteSources(1, &stream->source);
    g_AL.alDeleteBuffers(STREAM_BUFFER_COUNT, stream->buffers);

    if (stream->file) {
        fclose(stream->file);
    }

    memset(stream, 0, sizeof(OAL_Stream));
}

void OAL_Start_Stream(OAL_Stream* stream) {
    if (!stream || !stream->in_use) return;

    // Reset position
    stream->data_pos = 0;
    stream->loops_remaining = stream->loop_count;
    fseek(stream->file, stream->data_start, SEEK_SET);

    // Apply settings
    g_AL.alSourcef(stream->source, AL_GAIN, VolumeToGain(stream->volume));
    ApplyPan(stream->source, stream->pan);
    g_AL.alSourcef(stream->source, AL_PITCH, (float)stream->playback_rate / stream->base_rate);

    // Fill initial buffers
    char buffer_data[STREAM_BUFFER_SIZE];
    ALenum format = GetALFormat(stream->channels, stream->bits);

    for (int i = 0; i < STREAM_BUFFER_COUNT; i++) {
        size_t bytes_to_read = STREAM_BUFFER_SIZE;
        if (stream->data_pos + bytes_to_read > (size_t)stream->data_size) {
            bytes_to_read = stream->data_size - stream->data_pos;
        }

        if (bytes_to_read > 0) {
            size_t bytes_read = fread(buffer_data, 1, bytes_to_read, stream->file);
            stream->data_pos += bytes_read;

            g_AL.alBufferData(stream->buffers[i], format, buffer_data, (ALint)bytes_read, stream->rate);
            g_AL.alSourceQueueBuffers(stream->source, 1, &stream->buffers[i]);
        }
    }

    g_AL.alSourcePlay(stream->source);
    stream->status = OAL_SMP_PLAYING;
    stream->paused = 0;
}

void OAL_Stop_Stream(OAL_Stream* stream) {
    if (!stream || !stream->in_use) return;
    g_AL.alSourceStop(stream->source);
    stream->status = OAL_SMP_STOPPED;
}

void OAL_Pause_Stream(OAL_Stream* stream, OAL_S32 pause) {
    if (!stream || !stream->in_use) return;

    if (pause) {
        g_AL.alSourcePause(stream->source);
        stream->paused = 1;
    } else {
        g_AL.alSourcePlay(stream->source);
        stream->paused = 0;
    }
}

OAL_S32 OAL_Stream_Status(OAL_Stream* stream) {
    if (!stream || !stream->in_use) return OAL_SMP_FREE;

    if (stream->paused) return OAL_SMP_STOPPED;

    ALint state;
    g_AL.alGetSourcei(stream->source, AL_SOURCE_STATE, &state);

    switch (state) {
        case AL_PLAYING: stream->status = OAL_SMP_PLAYING; break;
        case AL_PAUSED:  stream->status = OAL_SMP_STOPPED; break;
        case AL_STOPPED:
        case AL_INITIAL: stream->status = OAL_SMP_DONE; break;
        default:         stream->status = OAL_SMP_DONE; break;
    }

    return stream->status;
}

void OAL_Set_Stream_Volume(OAL_Stream* stream, OAL_S32 volume) {
    if (!stream || !stream->in_use) return;
    stream->volume = (volume < 0) ? 0 : ((volume > 127) ? 127 : volume);
    g_AL.alSourcef(stream->source, AL_GAIN, VolumeToGain(stream->volume));
}

OAL_S32 OAL_Stream_Volume(OAL_Stream* stream) {
    return stream ? stream->volume : 127;
}

void OAL_Set_Stream_Pan(OAL_Stream* stream, OAL_S32 pan) {
    if (!stream || !stream->in_use) return;
    stream->pan = (pan < 0) ? 0 : ((pan > 127) ? 127 : pan);
    ApplyPan(stream->source, stream->pan);
}

OAL_S32 OAL_Stream_Pan(OAL_Stream* stream) {
    return stream ? stream->pan : 64;
}

void OAL_Set_Stream_Loop_Count(OAL_Stream* stream, OAL_S32 count) {
    if (!stream) return;
    stream->loop_count = count;
    stream->loops_remaining = count;
}

OAL_S32 OAL_Stream_Loop_Count(OAL_Stream* stream) {
    return stream ? stream->loop_count : 1;
}

void OAL_Set_Stream_MS_Position(OAL_Stream* stream, OAL_S32 ms) {
    if (!stream || !stream->in_use) return;

    int bytes_per_sample = (stream->bits / 8) * stream->channels;
    long byte_pos = (long)(((float)ms / 1000.0f) * stream->rate * bytes_per_sample);

    if (byte_pos < 0) byte_pos = 0;
    if (byte_pos > stream->data_size) byte_pos = stream->data_size;

    stream->data_pos = byte_pos;
    fseek(stream->file, stream->data_start + byte_pos, SEEK_SET);
}

void OAL_Stream_MS_Position(OAL_Stream* stream, OAL_S32* total, OAL_S32* current) {
    if (!stream) {
        if (total) *total = 0;
        if (current) *current = 0;
        return;
    }

    if (total) *total = stream->total_ms;

    if (current) {
        int bytes_per_sample = (stream->bits / 8) * stream->channels;
        if (bytes_per_sample > 0 && stream->rate > 0) {
            *current = (OAL_S32)((stream->data_pos * 1000) / (stream->rate * bytes_per_sample));
        } else {
            *current = 0;
        }
    }
}

void OAL_Set_Stream_Playback_Rate(OAL_Stream* stream, OAL_S32 rate) {
    if (!stream || !stream->in_use || rate <= 0) return;
    stream->playback_rate = rate;
    if (stream->base_rate > 0) {
        g_AL.alSourcef(stream->source, AL_PITCH, (float)rate / stream->base_rate);
    }
}

OAL_S32 OAL_Stream_Playback_Rate(OAL_Stream* stream) {
    return stream ? stream->playback_rate : 22050;
}

void OAL_Set_Stream_User_Data(OAL_Stream* stream, OAL_U32 index, OAL_S32 value) {
    if (!stream || index >= MAX_USER_DATA) return;
    stream->user_data[index] = value;
}

OAL_S32 OAL_Stream_User_Data(OAL_Stream* stream, OAL_U32 index) {
    if (!stream || index >= MAX_USER_DATA) return 0;
    return stream->user_data[index];
}

void OAL_Service_Stream(OAL_Stream* stream) {
    if (!stream || !stream->in_use || stream->paused) return;
    if (stream->status != OAL_SMP_PLAYING) return;

    ALint processed = 0;
    g_AL.alGetSourcei(stream->source, AL_BUFFERS_PROCESSED, &processed);

    char buffer_data[STREAM_BUFFER_SIZE];
    ALenum format = GetALFormat(stream->channels, stream->bits);

    while (processed > 0) {
        ALuint buffer;
        g_AL.alSourceUnqueueBuffers(stream->source, 1, &buffer);

        size_t bytes_to_read = STREAM_BUFFER_SIZE;
        size_t remaining = stream->data_size - stream->data_pos;

        if (remaining == 0) {
            // End of data
            if (stream->loop_count == 0 || stream->loops_remaining > 1) {
                // Loop
                if (stream->loops_remaining > 0) stream->loops_remaining--;
                stream->data_pos = 0;
                fseek(stream->file, stream->data_start, SEEK_SET);
                remaining = stream->data_size;
            } else {
                // Done
                processed--;
                continue;
            }
        }

        if (bytes_to_read > remaining) {
            bytes_to_read = remaining;
        }

        size_t bytes_read = fread(buffer_data, 1, bytes_to_read, stream->file);
        if (bytes_read > 0) {
            stream->data_pos += bytes_read;
            g_AL.alBufferData(buffer, format, buffer_data, (ALint)bytes_read, stream->rate);
            g_AL.alSourceQueueBuffers(stream->source, 1, &buffer);
        }

        processed--;
    }

    // Check if source stopped (buffer underrun)
    ALint state;
    g_AL.alGetSourcei(stream->source, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING && !stream->paused) {
        // Check if there are queued buffers
        ALint queued;
        g_AL.alGetSourcei(stream->source, AL_BUFFERS_QUEUED, &queued);
        if (queued > 0) {
            g_AL.alSourcePlay(stream->source);
        } else {
            stream->status = OAL_SMP_DONE;
        }
    }
}

// ============================================================================
// Master Volume
// ============================================================================

void OAL_Set_Master_Volume(OAL_S32 volume) {
    g_MasterVolume = (volume < 0) ? 0 : ((volume > 127) ? 127 : volume);

    if (!g_Initialized) return;

    // Update listener gain
    g_AL.alListenerf(AL_GAIN, g_MasterVolume / 127.0f);
}

OAL_S32 OAL_Master_Volume(void) {
    return g_MasterVolume;
}

// ============================================================================
// WAV Parsing
// ============================================================================

OAL_S32 OAL_WAV_Info(void* file_data, OAL_SoundInfo* info) {
    if (!file_data || !info) return OAL_ERROR;

    memset(info, 0, sizeof(OAL_SoundInfo));

    unsigned char* data = (unsigned char*)file_data;

    // Check RIFF header
    if (memcmp(data, "RIFF", 4) != 0) {
        return OAL_ERROR;
    }

    // Check WAVE format
    if (memcmp(data + 8, "WAVE", 4) != 0) {
        return OAL_ERROR;
    }

    // Find fmt and data chunks
    unsigned char* ptr = data + 12;
    int fmt_found = 0;
    int data_found = 0;

    while (!fmt_found || !data_found) {
        char chunk_id[5] = {0};
        memcpy(chunk_id, ptr, 4);
        unsigned int chunk_size = *(unsigned int*)(ptr + 4);
        ptr += 8;

        if (memcmp(chunk_id, "fmt ", 4) == 0) {
            info->format = *(unsigned short*)ptr;
            info->channels = *(unsigned short*)(ptr + 2);
            info->rate = *(unsigned int*)(ptr + 4);
            info->bits = *(unsigned short*)(ptr + 14);
            info->block_size = *(unsigned short*)(ptr + 12);
            fmt_found = 1;
        }
        else if (memcmp(chunk_id, "data", 4) == 0) {
            info->data_ptr = ptr;
            info->data_len = chunk_size;
            info->initial_ptr = ptr;
            data_found = 1;
        }

        ptr += chunk_size;

        // Bounds check
        if (ptr > data + 10000000) break; // Arbitrary limit
    }

    if (!fmt_found || !data_found) {
        return OAL_ERROR;
    }

    // Calculate number of samples
    int bytes_per_sample = (info->bits / 8) * info->channels;
    if (bytes_per_sample > 0) {
        info->samples = info->data_len / bytes_per_sample;
    }

    return OAL_OK;
}
