/*
** Miles Sound System Compatibility Layer
**
** This header provides the Miles Sound System API interface.
** When OpenAL is available, it routes calls to the OpenAL backend.
** When OpenAL is not available, it provides silent stubs.
*/

#ifndef MSS_H
#define MSS_H

#include <windows.h>
#include <mmsystem.h>
#include "openal_audio.h"

// Basic types
typedef int S32;
typedef unsigned int U32;
typedef float F32;
typedef short S16;
typedef unsigned short U16;
typedef char S8;
typedef unsigned char U8;

// MILES_HANDLE - match audiblesound.h definition
#ifndef MILES_HANDLE_DEFINED
#define MILES_HANDLE_DEFINED
typedef unsigned long MILES_HANDLE;
#endif

// Driver structure
typedef struct _DIG_DRIVER {
    S32 emulated_ds;
    S32 initialized;
} DIG_DRIVER;

// Handle types
typedef DIG_DRIVER* HDIGDRIVER;
typedef OAL_Sample* HSAMPLE;
typedef OAL_Sample3D* H3DSAMPLE;
typedef void* H3DPOBJECT;
typedef OAL_Stream* HSTREAM;
typedef void* HPROVIDER;
typedef void* HDISPLAY;
typedef void* HATTRIB;
typedef void* HTIMER;
typedef U32 HPROENUM;

// Boolean constants
#ifndef YES
#define YES 1
#endif
#ifndef NO
#define NO 0
#endif

// Provider enumeration
#define HPROENUM_FIRST 0

// Constants
#define MILES_HANDLE_NONE 0
#define DP_FILTER 1
#define DIG_F_MONO_8 1
#define DIG_F_MONO_16 2
#define DIG_F_STEREO_8 3
#define DIG_F_STEREO_16 4
#define DIG_F_ADPCM_MONO_16 5
#define DIG_F_ADPCM_STEREO_16 6

// Sample status (match OAL constants)
#define SMP_FREE OAL_SMP_FREE
#define SMP_DONE OAL_SMP_DONE
#define SMP_PLAYING OAL_SMP_PLAYING
#define SMP_STOPPED OAL_SMP_STOPPED
#define SMP_PLAYINGBUTRELEASED OAL_SMP_PLAYINGBUTRELEASED

// 3D positioning constants
#define AIL_3D_2_SPEAKER OAL_SPEAKER_2
#define AIL_3D_HEADPHONE OAL_SPEAKER_HEADPHONE
#define AIL_3D_SURROUND OAL_SPEAKER_SURROUND
#define AIL_3D_4_SPEAKER OAL_SPEAKER_4

// Wave format constants
#ifndef WAVE_FORMAT_IMA_ADPCM
#define WAVE_FORMAT_IMA_ADPCM 0x0011
#endif

// Filter preference constants
#define DP_DEFAULT_FILTER_MODE 0
#define DP_FILTER_MODE_LOWPASS 1
#define DP_FILTER_MODE_BANDPASS 2
#define DP_FILTER_MODE_HIGHPASS 3

#define DP_FILTER_CUT_OFF "Filter_CutOff"
#define DP_FILTER_RMS_VOLUME "Filter_RMS_Volume"
#define DP_FILTER_MUTE_AT_MAX "Filter_Mute_At_Max"

// Preference constants
#define DIG_MIXER_CHANNELS 1
#define DIG_DEFAULT_VOLUME 2
#define AIL_LOCK_PROTECTION 3
#define AIL_TIMERS_PER_SECOND 4
#define DIG_USE_WAVEOUT 5

// Error/success codes
#define AIL_NO_ERROR 0
#define M3D_NOERR 0

// File seek constants
#define AIL_FILE_SEEK_BEGIN 0
#define AIL_FILE_SEEK_CURRENT 1
#define AIL_FILE_SEEK_END 2

// EAX Environment constants
#define ENVIRONMENT_GENERIC 0
#define ENVIRONMENT_PADDEDCELL 1
#define ENVIRONMENT_ROOM 2
#define ENVIRONMENT_BATHROOM 3
#define ENVIRONMENT_LIVINGROOM 4
#define ENVIRONMENT_STONEROOM 5
#define ENVIRONMENT_AUDITORIUM 6
#define ENVIRONMENT_CONCERTHALL 7
#define ENVIRONMENT_CAVE 8
#define ENVIRONMENT_ARENA 9
#define ENVIRONMENT_HANGAR 10
#define ENVIRONMENT_CARPETEDHALLWAY 11
#define ENVIRONMENT_HALLWAY 12
#define ENVIRONMENT_STONECORRIDOR 13
#define ENVIRONMENT_ALLEY 14
#define ENVIRONMENT_FOREST 15
#define ENVIRONMENT_CITY 16
#define ENVIRONMENT_MOUNTAINS 17
#define ENVIRONMENT_QUARRY 18
#define ENVIRONMENT_PLAIN 19
#define ENVIRONMENT_PARKINGLOT 20
#define ENVIRONMENT_SEWERPIPE 21
#define ENVIRONMENT_UNDERWATER 22
#define ENVIRONMENT_DRUGGED 23
#define ENVIRONMENT_DIZZY 24
#define ENVIRONMENT_PSYCHOTIC 25
#define ENVIRONMENT_COUNT 26

// Invalid handle constants
#ifndef INVALID_MILES_HANDLE_DEFINED
#define INVALID_MILES_HANDLE_DEFINED
const MILES_HANDLE INVALID_MILES_HANDLE = (MILES_HANDLE)-1;
#endif

#define INVALID_HTIMER ((HTIMER)0)
#define INVALID_HPROVIDER ((HPROVIDER)0)

// Callback convention
#ifndef AILCALLBACK
#define AILCALLBACK __stdcall
#endif

// Callback types
typedef void (AILCALLBACK *AILSAMPLECB)(HSAMPLE);
typedef void (AILCALLBACK *AIL3DSAMPLECB)(H3DSAMPLE);
typedef void (AILCALLBACK *AILSTREAMCB)(HSTREAM);
typedef void (AILCALLBACK *AILTIMERCB)(U32 user);

// File callback types
typedef U32 (AILCALLBACK *AIL_file_open_callback)(const char* filename, U32* file_handle);
typedef void (AILCALLBACK *AIL_file_close_callback)(U32 file_handle);
typedef S32 (AILCALLBACK *AIL_file_seek_callback)(U32 file_handle, S32 offset, U32 type);
typedef U32 (AILCALLBACK *AIL_file_read_callback)(U32 file_handle, void* buffer, U32 bytes);

// Sound info structure
typedef struct {
    S32 format;
    void* data_ptr;
    U32 data_len;
    U32 rate;
    S32 bits;
    S32 channels;
    U32 samples;
    U32 block_size;
    void* initial_ptr;
} AILSOUNDINFO;

// Global driver instance
static DIG_DRIVER g_DigDriver = {0, 0};

// ============================================================================
// Core Functions
// ============================================================================

inline void AIL_lock(void) {}
inline void AIL_unlock(void) {}

inline S32 AIL_startup(void) {
    if (OAL_Startup() == OAL_OK) {
        g_DigDriver.initialized = 1;
        return 1;
    }
    return 0;
}

inline void AIL_shutdown(void) {
    OAL_Shutdown();
    g_DigDriver.initialized = 0;
}

inline char* AIL_last_error(void) {
    return (char*)OAL_Last_Error();
}

inline void AIL_set_error(char* error) { (void)error; }

// ============================================================================
// Digital Driver Functions
// ============================================================================

inline HDIGDRIVER AIL_open_digital_driver(U32 frequency, S32 bits, S32 channel, U32 flags) {
    (void)frequency; (void)bits; (void)channel; (void)flags;
    if (OAL_Is_Ready()) {
        return &g_DigDriver;
    }
    return NULL;
}

inline void AIL_close_digital_driver(HDIGDRIVER dig) { (void)dig; }
inline void AIL_digital_handle_release(HDIGDRIVER dig) { (void)dig; }
inline void AIL_digital_handle_reacquire(HDIGDRIVER dig) { (void)dig; }
inline S32 AIL_digital_CPU_percent(HDIGDRIVER dig) { (void)dig; return 0; }

inline S32 AIL_set_digital_master_volume(HDIGDRIVER dig, S32 volume) {
    (void)dig;
    OAL_Set_Master_Volume(volume);
    return volume;
}

inline S32 AIL_digital_master_volume(HDIGDRIVER dig) {
    (void)dig;
    return OAL_Master_Volume();
}

inline S32 AIL_waveOutOpen(HDIGDRIVER* dig, void* reserved, S32 driver_index, LPWAVEFORMAT format) {
    (void)reserved; (void)driver_index; (void)format;
    if (OAL_Is_Ready()) {
        if (dig) *dig = &g_DigDriver;
        return 0;
    }
    if (dig) *dig = NULL;
    return -1;
}

inline void AIL_waveOutClose(HDIGDRIVER dig) { (void)dig; }

// ============================================================================
// 2D Sample Functions
// ============================================================================

inline HSAMPLE AIL_allocate_sample_handle(HDIGDRIVER dig) {
    (void)dig;
    return OAL_Allocate_Sample();
}

inline void AIL_release_sample_handle(HSAMPLE S) {
    OAL_Release_Sample(S);
}

inline void AIL_init_sample(HSAMPLE S) {
    OAL_Init_Sample(S);
}

inline S32 AIL_set_sample_file(HSAMPLE S, void* file_image, S32 block) {
    (void)block;
    return (OAL_Set_Sample_File(S, file_image, 0) == OAL_OK) ? 1 : 0;
}

inline S32 AIL_set_named_sample_file(HSAMPLE S, char* filename, void* file_image, S32 file_size, S32 block) {
    (void)filename; (void)block;
    return (OAL_Set_Sample_File(S, file_image, file_size) == OAL_OK) ? 1 : 0;
}

inline void AIL_start_sample(HSAMPLE S) { OAL_Start_Sample(S); }
inline void AIL_stop_sample(HSAMPLE S) { OAL_Stop_Sample(S); }
inline void AIL_resume_sample(HSAMPLE S) { OAL_Resume_Sample(S); }
inline void AIL_end_sample(HSAMPLE S) { OAL_End_Sample(S); }
inline S32 AIL_sample_status(HSAMPLE S) { return OAL_Sample_Status(S); }

inline void AIL_set_sample_pan(HSAMPLE S, S32 pan) { OAL_Set_Sample_Pan(S, pan); }
inline S32 AIL_sample_pan(HSAMPLE S) { return OAL_Sample_Pan(S); }

inline void AIL_set_sample_volume(HSAMPLE S, S32 volume) { OAL_Set_Sample_Volume(S, volume); }
inline S32 AIL_sample_volume(HSAMPLE S) { return OAL_Sample_Volume(S); }

inline void AIL_set_sample_loop_count(HSAMPLE S, S32 count) { OAL_Set_Sample_Loop_Count(S, count); }
inline S32 AIL_sample_loop_count(HSAMPLE S) { return OAL_Sample_Loop_Count(S); }

inline void AIL_set_sample_ms_position(HSAMPLE S, S32 ms) { OAL_Set_Sample_MS_Position(S, ms); }
inline void AIL_sample_ms_position(HSAMPLE S, S32* total_ms, S32* current_ms) {
    OAL_Sample_MS_Position(S, total_ms, current_ms);
}

inline void AIL_set_sample_user_data(HSAMPLE S, U32 index, S32 value) { OAL_Set_Sample_User_Data(S, index, value); }
inline S32 AIL_sample_user_data(HSAMPLE S, U32 index) { return OAL_Sample_User_Data(S, index); }

inline S32 AIL_sample_playback_rate(HSAMPLE S) { return OAL_Sample_Playback_Rate(S); }
inline void AIL_set_sample_playback_rate(HSAMPLE S, S32 rate) { OAL_Set_Sample_Playback_Rate(S, rate); }

inline void AIL_set_sample_processor(HSAMPLE S, S32 pipeline, HPROVIDER provider) {
    (void)S; (void)pipeline; (void)provider;
}

// ============================================================================
// 3D Provider Functions
// ============================================================================

inline S32 AIL_open_3D_provider(HPROVIDER lib) {
    (void)lib;
    return OAL_Is_Ready() ? 0 : -1;
}

inline void AIL_close_3D_provider(HPROVIDER lib) { (void)lib; }

inline S32 AIL_enumerate_3D_providers(HPROENUM* next, HPROVIDER* provider, char** name) {
    static int enumerated = 0;
    if (*next == HPROENUM_FIRST) {
        enumerated = 0;
    }
    if (enumerated == 0 && OAL_Is_Ready()) {
        enumerated = 1;
        if (provider) *provider = (HPROVIDER)1;
        if (name) *name = (char*)"OpenAL Soft";
        *next = 1;
        return 1;
    }
    return 0;
}

inline H3DPOBJECT AIL_3D_open_listener(HPROVIDER lib) { (void)lib; return (H3DPOBJECT)1; }
inline void AIL_close_3D_listener(HPROVIDER lib) { (void)lib; }
inline char* AIL_3D_provider_attribute(HPROVIDER lib, char* name) { (void)lib; (void)name; return (char*)""; }

// ============================================================================
// 3D Sample Functions
// ============================================================================

inline H3DSAMPLE AIL_allocate_3D_sample_handle(HPROVIDER lib) {
    (void)lib;
    return OAL_Allocate_3D_Sample();
}

inline void AIL_release_3D_sample_handle(H3DSAMPLE S) { OAL_Release_3D_Sample(S); }

inline S32 AIL_set_3D_sample_file(H3DSAMPLE S, void* file_image) {
    return (OAL_Set_3D_Sample_File(S, file_image, 0) == OAL_OK) ? 1 : 0;
}

inline void AIL_start_3D_sample(H3DSAMPLE S) { OAL_Start_3D_Sample(S); }
inline void AIL_stop_3D_sample(H3DSAMPLE S) { OAL_Stop_3D_Sample(S); }
inline void AIL_resume_3D_sample(H3DSAMPLE S) { OAL_Resume_3D_Sample(S); }
inline void AIL_end_3D_sample(H3DSAMPLE S) { OAL_End_3D_Sample(S); }
inline S32 AIL_3D_sample_status(H3DSAMPLE S) { return OAL_3D_Sample_Status(S); }

inline void AIL_set_3D_sample_volume(H3DSAMPLE S, S32 volume) { OAL_Set_3D_Sample_Volume(S, volume); }
inline S32 AIL_3D_sample_volume(H3DSAMPLE S) { return OAL_3D_Sample_Volume(S); }

inline void AIL_set_3D_sample_loop_count(H3DSAMPLE S, S32 count) { OAL_Set_3D_Sample_Loop_Count(S, count); }
inline S32 AIL_3D_sample_loop_count(H3DSAMPLE S) { return OAL_3D_Sample_Loop_Count(S); }

inline void AIL_set_3D_sample_offset(H3DSAMPLE S, U32 offset) { OAL_Set_3D_Sample_Offset(S, offset); }
inline U32 AIL_3D_sample_offset(H3DSAMPLE S) { return OAL_3D_Sample_Offset(S); }
inline U32 AIL_3D_sample_length(H3DSAMPLE S) { return OAL_3D_Sample_Length(S); }

inline void AIL_set_3D_object_user_data(H3DSAMPLE S, U32 index, S32 value) { OAL_Set_3D_Sample_User_Data(S, index, value); }
inline S32 AIL_3D_object_user_data(H3DSAMPLE S, U32 index) { return OAL_3D_Sample_User_Data(S, index); }

inline S32 AIL_3D_sample_playback_rate(H3DSAMPLE S) { return OAL_3D_Sample_Playback_Rate(S); }
inline void AIL_set_3D_sample_playback_rate(H3DSAMPLE S, S32 rate) { OAL_Set_3D_Sample_Playback_Rate(S, rate); }

inline void AIL_set_3D_sample_distances(H3DSAMPLE S, F32 max_dist, F32 min_dist) {
    OAL_Set_3D_Sample_Distances(S, max_dist, min_dist);
}

inline void AIL_set_3D_sample_effects_level(H3DSAMPLE S, F32 level) {
    OAL_Set_3D_Sample_Effects_Level(S, level);
}

// ============================================================================
// 3D Position/Orientation
// ============================================================================

inline void AIL_set_3D_position(H3DSAMPLE S, F32 x, F32 y, F32 z) {
    OAL_Set_3D_Sample_Position(S, x, y, z);
}

inline void AIL_3D_position(H3DSAMPLE S, F32* x, F32* y, F32* z) {
    OAL_3D_Sample_Position(S, x, y, z);
}

inline void AIL_set_3D_orientation(H3DSAMPLE S, F32 fx, F32 fy, F32 fz, F32 ux, F32 uy, F32 uz) {
    (void)S; (void)fx; (void)fy; (void)fz; (void)ux; (void)uy; (void)uz;
    // Samples don't have orientation in OpenAL, only listeners
}

inline void AIL_3D_orientation(H3DSAMPLE S, F32* fx, F32* fy, F32* fz, F32* ux, F32* uy, F32* uz) {
    (void)S;
    if(fx) *fx = 0; if(fy) *fy = 0; if(fz) *fz = -1;
    if(ux) *ux = 0; if(uy) *uy = 1; if(uz) *uz = 0;
}

inline void AIL_set_3D_velocity_vector(H3DSAMPLE S, F32 dx, F32 dy, F32 dz) {
    OAL_Set_3D_Sample_Velocity(S, dx, dy, dz);
}

inline void AIL_set_3D_velocity(H3DSAMPLE S, F32 dx, F32 dy, F32 dz, F32 dt) {
    (void)dt;
    OAL_Set_3D_Sample_Velocity(S, dx, dy, dz);
}

// ============================================================================
// 3D Listener
// ============================================================================

inline void AIL_set_3D_speaker_type(HPROVIDER lib, S32 type) {
    (void)lib;
    OAL_Set_Speaker_Type(type);
}

inline S32 AIL_3D_speaker_type(HPROVIDER lib) {
    (void)lib;
    return OAL_Speaker_Type();
}

inline void AIL_set_listener_3D_position(HPROVIDER lib, F32 x, F32 y, F32 z) {
    (void)lib;
    OAL_Set_Listener_Position(x, y, z);
}

inline void AIL_listener_3D_position(HPROVIDER lib, F32* x, F32* y, F32* z) {
    (void)lib;
    if(x) *x = 0; if(y) *y = 0; if(z) *z = 0;
}

inline void AIL_set_listener_3D_orientation(HPROVIDER lib, F32 fx, F32 fy, F32 fz, F32 ux, F32 uy, F32 uz) {
    (void)lib;
    OAL_Set_Listener_Orientation(fx, fy, fz, ux, uy, uz);
}

inline void AIL_set_listener_3D_velocity_vector(HPROVIDER lib, F32 dx, F32 dy, F32 dz) {
    (void)lib;
    OAL_Set_Listener_Velocity(dx, dy, dz);
}

inline void AIL_set_3D_doppler_factor(HPROVIDER lib, F32 factor) {
    (void)lib;
    OAL_Set_Doppler_Factor(factor);
}

inline void AIL_set_3D_distance_factor(HPROVIDER lib, F32 factor) {
    (void)lib;
    OAL_Set_Distance_Factor(factor);
}

inline void AIL_set_3D_rolloff_factor(HPROVIDER lib, F32 factor) {
    (void)lib;
    OAL_Set_Rolloff_Factor(factor);
}

// ============================================================================
// Stream Functions
// ============================================================================

inline HSTREAM AIL_open_stream(HDIGDRIVER dig, char* filename, S32 stream_mem) {
    (void)dig; (void)stream_mem;
    return OAL_Open_Stream(filename);
}

inline HSTREAM AIL_open_stream_by_sample(HDIGDRIVER dig, HSAMPLE sample, const char* filename, S32 stream_mem) {
    (void)dig; (void)sample; (void)stream_mem;
    return OAL_Open_Stream(filename);
}

inline void AIL_close_stream(HSTREAM stream) { OAL_Close_Stream(stream); }
inline void AIL_start_stream(HSTREAM stream) { OAL_Start_Stream(stream); }
inline void AIL_stop_stream(HSTREAM stream) { OAL_Stop_Stream(stream); }
inline void AIL_pause_stream(HSTREAM stream, S32 onoff) { OAL_Pause_Stream(stream, onoff); }

inline void AIL_set_stream_volume(HSTREAM stream, S32 volume) { OAL_Set_Stream_Volume(stream, volume); }
inline S32 AIL_stream_volume(HSTREAM stream) { return OAL_Stream_Volume(stream); }

inline void AIL_set_stream_pan(HSTREAM stream, S32 pan) { OAL_Set_Stream_Pan(stream, pan); }
inline S32 AIL_stream_pan(HSTREAM stream) { return OAL_Stream_Pan(stream); }

inline void AIL_set_stream_loop_count(HSTREAM stream, S32 count) { OAL_Set_Stream_Loop_Count(stream, count); }
inline S32 AIL_stream_loop_count(HSTREAM stream) { return OAL_Stream_Loop_Count(stream); }

inline void AIL_set_stream_loop_block(HSTREAM stream, S32 start, S32 end) {
    (void)stream; (void)start; (void)end;
}

inline void AIL_set_stream_ms_position(HSTREAM stream, S32 ms) { OAL_Set_Stream_MS_Position(stream, ms); }
inline void AIL_stream_ms_position(HSTREAM stream, S32* total, S32* current) {
    OAL_Stream_MS_Position(stream, total, current);
}

inline S32 AIL_stream_status(HSTREAM stream) { return OAL_Stream_Status(stream); }

inline void AIL_set_stream_user_data(HSTREAM stream, U32 index, S32 value) {
    OAL_Set_Stream_User_Data(stream, index, value);
}
inline S32 AIL_stream_user_data(HSTREAM stream, U32 index) {
    return OAL_Stream_User_Data(stream, index);
}

inline void AIL_set_stream_playback_rate(HSTREAM stream, S32 rate) { OAL_Set_Stream_Playback_Rate(stream, rate); }
inline S32 AIL_stream_playback_rate(HSTREAM stream) { return OAL_Stream_Playback_Rate(stream); }

inline void AIL_service_stream(HSTREAM stream, S32 fillup) {
    (void)fillup;
    OAL_Service_Stream(stream);
}

// ============================================================================
// Callbacks (stored but not invoked by OpenAL implementation)
// ============================================================================

inline void AIL_register_EOS_callback(HSAMPLE S, AILSAMPLECB cb) { (void)S; (void)cb; }
inline void AIL_register_3D_EOS_callback(H3DSAMPLE S, AIL3DSAMPLECB cb) { (void)S; (void)cb; }
inline void AIL_register_stream_callback(HSTREAM stream, AILSTREAMCB cb) { (void)stream; (void)cb; }

// ============================================================================
// Filter Functions (not implemented in OpenAL backend)
// ============================================================================

inline HPROVIDER AIL_find_filter(HDIGDRIVER dig, char* name, char* suffix) {
    (void)dig; (void)name; (void)suffix;
    return NULL;
}

inline S32 AIL_enumerate_filters(HPROENUM* next, HPROVIDER* filter, char** name) {
    (void)next; (void)filter; (void)name;
    return 0;
}

inline void AIL_set_filter_sample_preference(HSAMPLE S, char* name, void* val) {
    (void)S; (void)name; (void)val;
}

inline void AIL_filter_sample_attribute(HSAMPLE S, char* name, void* val) {
    (void)S; (void)name; (void)val;
}

// ============================================================================
// Timer Functions (minimal implementation)
// ============================================================================

inline HTIMER AIL_register_timer(AILTIMERCB cb) { (void)cb; return NULL; }
inline void AIL_release_timer_handle(HTIMER timer) { (void)timer; }
inline void AIL_set_timer_period(HTIMER timer, U32 microseconds) { (void)timer; (void)microseconds; }
inline void AIL_set_timer_frequency(HTIMER timer, U32 frequency) { (void)timer; (void)frequency; }
inline void AIL_set_timer_user(HTIMER timer, U32 user) { (void)timer; (void)user; }
inline void AIL_start_timer(HTIMER timer) { (void)timer; }
inline void AIL_stop_timer(HTIMER timer) { (void)timer; }

// ============================================================================
// Misc Functions
// ============================================================================

inline S32 AIL_set_preference(U32 pref, S32 value) { (void)pref; (void)value; return 0; }
inline S32 AIL_get_preference(U32 pref) { (void)pref; return 0; }

inline void AIL_serve(void) {
    OAL_Update();
}

inline void AIL_set_timer_divisor(S32 divisor) { (void)divisor; }
inline S32 AIL_timer_count(void) { return 0; }
inline U32 AIL_us_count(void) { return GetTickCount() * 1000; }
inline void AIL_delay(S32 ms) { Sleep(ms); }

// ============================================================================
// WAV Info
// ============================================================================

inline S32 AIL_WAV_info(void* file_image, AILSOUNDINFO* info) {
    OAL_SoundInfo oal_info;
    if (OAL_WAV_Info(file_image, &oal_info) != OAL_OK) {
        if (info) {
            info->format = WAVE_FORMAT_IMA_ADPCM;
            info->data_ptr = NULL;
            info->data_len = 0;
            info->rate = 22050;
            info->bits = 16;
            info->channels = 1;
            info->samples = 0;
            info->block_size = 0;
            info->initial_ptr = NULL;
        }
        return 0;
    }

    if (info) {
        info->format = oal_info.format;
        info->data_ptr = oal_info.data_ptr;
        info->data_len = oal_info.data_len;
        info->rate = oal_info.rate;
        info->bits = oal_info.bits;
        info->channels = oal_info.channels;
        info->samples = oal_info.samples;
        info->block_size = oal_info.block_size;
        info->initial_ptr = oal_info.initial_ptr;
    }

    return 1;
}

// ============================================================================
// File I/O (not implemented)
// ============================================================================

inline void* AIL_file_read(char* filename, void* dest) { (void)filename; (void)dest; return NULL; }
inline S32 AIL_file_size(char* filename) { (void)filename; return 0; }
inline void AIL_mem_free_lock(void* ptr) { (void)ptr; }

inline void AIL_set_file_callbacks(
    AIL_file_open_callback open_cb,
    AIL_file_close_callback close_cb,
    AIL_file_seek_callback seek_cb,
    AIL_file_read_callback read_cb) {
    (void)open_cb; (void)close_cb; (void)seek_cb; (void)read_cb;
}

#endif // MSS_H
