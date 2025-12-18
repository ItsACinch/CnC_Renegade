/*
** Miles Sound System Stub Header
**
** This stub provides minimal definitions to allow compilation
** when the Miles Sound System SDK is not available.
** Audio functionality will be disabled.
*/

#ifndef MSS_H
#define MSS_H

#include <windows.h>
#include <mmsystem.h> // For LPWAVEFORMAT, PCMWAVEFORMAT

// Always provide stubs when this file is included
// The real Miles SDK would have different headers

// Basic types
typedef int S32;
typedef unsigned int U32;
typedef float F32;
typedef short S16;
typedef unsigned short U16;
typedef char S8;
typedef unsigned char U8;

// MILES_HANDLE - match audiblesound.h definition to avoid conflicts
#ifndef MILES_HANDLE_DEFINED
#define MILES_HANDLE_DEFINED
typedef unsigned long MILES_HANDLE;
#endif

// Driver structure (needs emulated_ds field for code compatibility)
typedef struct _DIG_DRIVER {
    S32 emulated_ds;
    // Other fields not used by stub
} DIG_DRIVER;

// Handle types (pointers for stub)
typedef DIG_DRIVER* HDIGDRIVER;
typedef void* HSAMPLE;
typedef void* H3DSAMPLE;
typedef void* H3DPOBJECT;
typedef void* HSTREAM;
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

// Sample status
#define SMP_FREE 0
#define SMP_DONE 1
#define SMP_PLAYING 2
#define SMP_STOPPED 3
#define SMP_PLAYINGBUTRELEASED 4

// 3D positioning constants
#define AIL_3D_2_SPEAKER 0
#define AIL_3D_HEADPHONE 1
#define AIL_3D_SURROUND 2
#define AIL_3D_4_SPEAKER 3

// Wave format constants (may be missing from older Windows SDK)
#ifndef WAVE_FORMAT_IMA_ADPCM
#define WAVE_FORMAT_IMA_ADPCM 0x0011
#endif

// Filter preference constants
#define DP_DEFAULT_FILTER_MODE 0
#define DP_FILTER_MODE_LOWPASS 1
#define DP_FILTER_MODE_BANDPASS 2
#define DP_FILTER_MODE_HIGHPASS 3

// Filter preference names
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

// EAX Environment constants (reverb room types)
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

// Filter constants (for FilteredSound)
// Match audiblesound.h definition to avoid conflicts
#ifndef INVALID_MILES_HANDLE_DEFINED
#define INVALID_MILES_HANDLE_DEFINED
const MILES_HANDLE INVALID_MILES_HANDLE = (MILES_HANDLE)-1;
#endif

// Null constants for handle types (used for initialization)
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

// Sound info structure (for AIL_WAV_info)
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

// ============================================================================
// Stub function implementations
// All functions return appropriate default values or do nothing
// ============================================================================

// Lock/Unlock (used by Utils.h)
inline void AIL_lock(void) {}
inline void AIL_unlock(void) {}

// Startup/Shutdown
inline S32 AIL_startup(void) { return 0; }
inline void AIL_shutdown(void) {}
inline char* AIL_last_error(void) { return "Audio disabled (Miles SDK not available)"; }
inline void AIL_set_error(char* error) { (void)error; }

// Digital driver functions
inline HDIGDRIVER AIL_open_digital_driver(U32 frequency, S32 bits, S32 channel, U32 flags) { return NULL; }
inline void AIL_close_digital_driver(HDIGDRIVER dig) {}
inline void AIL_digital_handle_release(HDIGDRIVER dig) {}
inline void AIL_digital_handle_reacquire(HDIGDRIVER dig) {}
inline S32 AIL_digital_CPU_percent(HDIGDRIVER dig) { return 0; }
inline S32 AIL_set_digital_master_volume(HDIGDRIVER dig, S32 volume) { return 0; }
inline S32 AIL_digital_master_volume(HDIGDRIVER dig) { return 0; }

// WaveOut driver functions
inline S32 AIL_waveOutOpen(HDIGDRIVER* dig, void* reserved, S32 driver_index, LPWAVEFORMAT format) {
    if (dig) *dig = NULL;
    return -1; // Return error - no audio available
}
inline void AIL_waveOutClose(HDIGDRIVER dig) {}

// 2D Sample functions
inline HSAMPLE AIL_allocate_sample_handle(HDIGDRIVER dig) { return NULL; }
inline void AIL_release_sample_handle(HSAMPLE S) {}
inline void AIL_init_sample(HSAMPLE S) {}
inline S32 AIL_set_sample_file(HSAMPLE S, void* file_image, S32 block) { return 0; }
inline S32 AIL_set_named_sample_file(HSAMPLE S, char* filename, void* file_image, S32 file_size, S32 block) { return 0; }
inline void AIL_start_sample(HSAMPLE S) {}
inline void AIL_stop_sample(HSAMPLE S) {}
inline void AIL_resume_sample(HSAMPLE S) {}
inline void AIL_end_sample(HSAMPLE S) {}
inline S32 AIL_sample_status(HSAMPLE S) { return SMP_DONE; }
inline void AIL_set_sample_pan(HSAMPLE S, S32 pan) {}
inline S32 AIL_sample_pan(HSAMPLE S) { return 64; }
inline void AIL_set_sample_volume(HSAMPLE S, S32 volume) {}
inline S32 AIL_sample_volume(HSAMPLE S) { return 127; }
inline void AIL_set_sample_loop_count(HSAMPLE S, S32 count) {}
inline S32 AIL_sample_loop_count(HSAMPLE S) { return 1; }
inline void AIL_set_sample_ms_position(HSAMPLE S, S32 ms) {}
inline void AIL_sample_ms_position(HSAMPLE S, S32* total_ms, S32* current_ms) { if(total_ms) *total_ms = 0; if(current_ms) *current_ms = 0; }
inline void AIL_set_sample_user_data(HSAMPLE S, U32 index, S32 value) {}
inline S32 AIL_sample_user_data(HSAMPLE S, U32 index) { return 0; }
inline S32 AIL_sample_playback_rate(HSAMPLE S) { return 22050; }
inline void AIL_set_sample_playback_rate(HSAMPLE S, S32 rate) {}
inline void AIL_set_sample_processor(HSAMPLE S, S32 pipeline, HPROVIDER provider) {}

// 3D Provider functions
inline S32 AIL_open_3D_provider(HPROVIDER lib) { return -1; }
inline void AIL_close_3D_provider(HPROVIDER lib) {}
inline S32 AIL_enumerate_3D_providers(HPROENUM* next, HPROVIDER* provider, char** name) { return 0; }
inline H3DPOBJECT AIL_3D_open_listener(HPROVIDER lib) { return NULL; }
inline void AIL_close_3D_listener(HPROVIDER lib) {}
inline char* AIL_3D_provider_attribute(HPROVIDER lib, char* name) { return ""; }

// 3D Sample functions
inline H3DSAMPLE AIL_allocate_3D_sample_handle(HPROVIDER lib) { return NULL; }
inline void AIL_release_3D_sample_handle(H3DSAMPLE S) {}
inline S32 AIL_set_3D_sample_file(H3DSAMPLE S, void* file_image) { return 0; }
inline void AIL_start_3D_sample(H3DSAMPLE S) {}
inline void AIL_stop_3D_sample(H3DSAMPLE S) {}
inline void AIL_resume_3D_sample(H3DSAMPLE S) {}
inline void AIL_end_3D_sample(H3DSAMPLE S) {}
inline S32 AIL_3D_sample_status(H3DSAMPLE S) { return SMP_DONE; }
inline void AIL_set_3D_sample_volume(H3DSAMPLE S, S32 volume) {}
inline S32 AIL_3D_sample_volume(H3DSAMPLE S) { return 127; }
inline void AIL_set_3D_sample_loop_count(H3DSAMPLE S, S32 count) {}
inline S32 AIL_3D_sample_loop_count(H3DSAMPLE S) { return 1; }
inline void AIL_set_3D_sample_offset(H3DSAMPLE S, U32 offset) {}
inline U32 AIL_3D_sample_offset(H3DSAMPLE S) { return 0; }
inline U32 AIL_3D_sample_length(H3DSAMPLE S) { return 0; }
inline void AIL_set_3D_object_user_data(H3DSAMPLE S, U32 index, S32 value) {}
inline S32 AIL_3D_object_user_data(H3DSAMPLE S, U32 index) { return 0; }
inline S32 AIL_3D_sample_playback_rate(H3DSAMPLE S) { return 22050; }
inline void AIL_set_3D_sample_playback_rate(H3DSAMPLE S, S32 rate) {}
inline void AIL_set_3D_sample_distances(H3DSAMPLE S, F32 max_dist, F32 min_dist) {}
inline void AIL_set_3D_sample_effects_level(H3DSAMPLE S, F32 level) {}

// 3D Position/Orientation
inline void AIL_set_3D_position(H3DSAMPLE S, F32 x, F32 y, F32 z) {}
inline void AIL_3D_position(H3DSAMPLE S, F32* x, F32* y, F32* z) { if(x) *x = 0; if(y) *y = 0; if(z) *z = 0; }
inline void AIL_set_3D_orientation(H3DSAMPLE S, F32 fx, F32 fy, F32 fz, F32 ux, F32 uy, F32 uz) {}
inline void AIL_3D_orientation(H3DSAMPLE S, F32* fx, F32* fy, F32* fz, F32* ux, F32* uy, F32* uz) {}
inline void AIL_set_3D_velocity_vector(H3DSAMPLE S, F32 dx, F32 dy, F32 dz) {}
inline void AIL_set_3D_velocity(H3DSAMPLE S, F32 dx, F32 dy, F32 dz, F32 dt) {}

// 3D Listener
inline void AIL_set_3D_speaker_type(HPROVIDER lib, S32 type) {}
inline S32 AIL_3D_speaker_type(HPROVIDER lib) { return AIL_3D_2_SPEAKER; }
inline void AIL_set_listener_3D_position(HPROVIDER lib, F32 x, F32 y, F32 z) {}
inline void AIL_listener_3D_position(HPROVIDER lib, F32* x, F32* y, F32* z) {}
inline void AIL_set_listener_3D_orientation(HPROVIDER lib, F32 fx, F32 fy, F32 fz, F32 ux, F32 uy, F32 uz) {}
inline void AIL_set_listener_3D_velocity_vector(HPROVIDER lib, F32 dx, F32 dy, F32 dz) {}
inline void AIL_set_3D_doppler_factor(HPROVIDER lib, F32 factor) {}
inline void AIL_set_3D_distance_factor(HPROVIDER lib, F32 factor) {}
inline void AIL_set_3D_rolloff_factor(HPROVIDER lib, F32 factor) {}

// Stream functions
inline HSTREAM AIL_open_stream(HDIGDRIVER dig, char* filename, S32 stream_mem) { return NULL; }
inline HSTREAM AIL_open_stream_by_sample(HDIGDRIVER dig, HSAMPLE sample, const char* filename, S32 stream_mem) { return NULL; }
inline void AIL_close_stream(HSTREAM stream) {}
inline void AIL_start_stream(HSTREAM stream) {}
inline void AIL_stop_stream(HSTREAM stream) {}
inline void AIL_pause_stream(HSTREAM stream, S32 onoff) {}
inline void AIL_set_stream_volume(HSTREAM stream, S32 volume) {}
inline S32 AIL_stream_volume(HSTREAM stream) { return 127; }
inline void AIL_set_stream_pan(HSTREAM stream, S32 pan) {}
inline S32 AIL_stream_pan(HSTREAM stream) { return 64; }
inline void AIL_set_stream_loop_count(HSTREAM stream, S32 count) {}
inline S32 AIL_stream_loop_count(HSTREAM stream) { return 1; }
inline void AIL_set_stream_loop_block(HSTREAM stream, S32 start, S32 end) {}
inline void AIL_set_stream_ms_position(HSTREAM stream, S32 ms) {}
inline void AIL_stream_ms_position(HSTREAM stream, S32* total, S32* current) { if(total) *total = 0; if(current) *current = 0; }
inline S32 AIL_stream_status(HSTREAM stream) { return SMP_DONE; }
inline void AIL_set_stream_user_data(HSTREAM stream, U32 index, S32 value) {}
inline S32 AIL_stream_user_data(HSTREAM stream, U32 index) { return 0; }
inline void AIL_set_stream_playback_rate(HSTREAM stream, S32 rate) {}
inline S32 AIL_stream_playback_rate(HSTREAM stream) { return 22050; }
inline void AIL_service_stream(HSTREAM stream, S32 fillup) {}

// Callbacks
inline void AIL_register_EOS_callback(HSAMPLE S, AILSAMPLECB cb) {}
inline void AIL_register_3D_EOS_callback(H3DSAMPLE S, AIL3DSAMPLECB cb) {}
inline void AIL_register_stream_callback(HSTREAM stream, AILSTREAMCB cb) {}

// Filter functions
inline HPROVIDER AIL_find_filter(HDIGDRIVER dig, char* name, char* suffix) { return NULL; }
inline S32 AIL_enumerate_filters(HPROENUM* next, HPROVIDER* filter, char** name) { return 0; }
inline void AIL_set_filter_sample_preference(HSAMPLE S, char* name, void* val) {}
inline void AIL_filter_sample_attribute(HSAMPLE S, char* name, void* val) {}

// Timer functions
inline HTIMER AIL_register_timer(AILTIMERCB cb) { return NULL; }
inline void AIL_release_timer_handle(HTIMER timer) {}
inline void AIL_set_timer_period(HTIMER timer, U32 microseconds) {}
inline void AIL_set_timer_frequency(HTIMER timer, U32 frequency) {}
inline void AIL_set_timer_user(HTIMER timer, U32 user) {}
inline void AIL_start_timer(HTIMER timer) {}
inline void AIL_stop_timer(HTIMER timer) {}

// Misc functions
inline S32 AIL_set_preference(U32 pref, S32 value) { return 0; }
inline S32 AIL_get_preference(U32 pref) { return 0; }
inline void AIL_serve(void) {}
inline void AIL_set_timer_divisor(S32 divisor) {}
inline S32 AIL_timer_count(void) { return 0; }
inline U32 AIL_us_count(void) { return 0; }
inline void AIL_delay(S32 ms) { Sleep(ms); }

// WAV info function
inline S32 AIL_WAV_info(void* file_image, AILSOUNDINFO* info) {
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
    return 0; // Return 0 to indicate failure/no info available
}

// File I/O functions
inline void* AIL_file_read(char* filename, void* dest) { return NULL; }
inline S32 AIL_file_size(char* filename) { return 0; }
inline void AIL_mem_free_lock(void* ptr) {}

// File callback registration
inline void AIL_set_file_callbacks(
    AIL_file_open_callback open_cb,
    AIL_file_close_callback close_cb,
    AIL_file_seek_callback seek_cb,
    AIL_file_read_callback read_cb) {}

#endif // MSS_H
