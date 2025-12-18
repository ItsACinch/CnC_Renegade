/*
**  Miles Sound System Compatibility Stub
**
**  This header provides stub definitions for the Miles Sound System (MSS)
**  APIs to allow compilation without the actual MSS SDK.
**
**  The actual audio implementation will need to be replaced with a modern
**  audio library (OpenAL, XAudio2, etc.) in Phase 3 of modernization.
*/

#ifndef _MSS_STUB_H_
#define _MSS_STUB_H_

#include <windows.h>
#include <cstring>

// ============================================================================
// MSS calling conventions and basic types
// ============================================================================

// AILCALLBACK is the calling convention for MSS callbacks
#ifndef AILCALLBACK
#define AILCALLBACK __stdcall
#endif

// MSS integer types
typedef unsigned int U32;
typedef signed int S32;
typedef unsigned short U16;
typedef signed short S16;
typedef unsigned char U8;
typedef signed char S8;
typedef float F32;
typedef double F64;

// ============================================================================
// Basic MSS types
// ============================================================================
typedef void* HDIGDRIVER;
typedef void* HSTREAM;
typedef void* HSAMPLE;
typedef void* H3DSAMPLE;
typedef void* H3DPOBJECT;  // 3D provider object handle
typedef void* H3DPROVIDER;
typedef void* HMDIDRIVER;
typedef void* HSEQUENCE;
typedef void* HDLSDEVICE;
typedef void* HDLSFILEID;
typedef void* HPROVIDER;
typedef void* HATTRIB;

// MSS preferences
typedef struct {
    int dummy;
} MSS_PREFERENCE;

// 3D position structure
typedef struct {
    float X;
    float Y;
    float Z;
} MSS_VECTOR;

// Sample status values
#define SMP_FREE        0
#define SMP_DONE        1
#define SMP_STOPPED     2
#define SMP_PLAYING     3
#define SMP_PLAYINGBUTRELEASED 4

// Stream status values
#define STREAM_FREE     0
#define STREAM_DONE     1
#define STREAM_STOPPED  2
#define STREAM_PLAYING  3

// 3D sample status values
#define H3D_NOENV       0

// Provider types
#define HPROVIDER_NONE  0

// Audio format types
#define DIG_F_MONO_8    0
#define DIG_F_MONO_16   1
#define DIG_F_STEREO_8  2
#define DIG_F_STEREO_16 3

// Loop types
#define DIG_RL_NONE     0
#define DIG_RL_LOOP     1

// Reverb types
#define EAX_ENVIRONMENT_GENERIC 0

// MSS initialization/shutdown stubs
inline int AIL_startup() { return 0; }
inline void AIL_shutdown() {}
inline int AIL_set_redist_directory(const char*) { return 0; }
inline int AIL_set_preference(unsigned int, int) { return 0; }
inline const char* AIL_last_error() { return "MSS not available"; }
inline const char* AIL_set_error(const char* msg) { return msg; }

// Digital audio stubs
inline int AIL_waveOutOpen(HDIGDRIVER*, void*, int, void*) { return 0; }
inline void AIL_waveOutClose(HDIGDRIVER) {}
inline int AIL_digital_handle_release(HDIGDRIVER) { return 0; }
inline int AIL_digital_handle_reacquire(HDIGDRIVER) { return 0; }

// Sample stubs
inline HSAMPLE AIL_allocate_sample_handle(HDIGDRIVER) { return nullptr; }
inline void AIL_release_sample_handle(HSAMPLE) {}
inline int AIL_set_sample_file(HSAMPLE, const void*, int) { return 0; }
inline void AIL_set_sample_address(HSAMPLE, const void*, unsigned int) {}
inline void AIL_set_sample_type(HSAMPLE, int, unsigned int) {}
inline void AIL_set_sample_playback_rate(HSAMPLE, int) {}
inline void AIL_set_sample_volume_levels(HSAMPLE, float, float, float) {}
inline void AIL_set_sample_volume_pan(HSAMPLE, float, float) {}
inline void AIL_set_sample_loop_count(HSAMPLE, int) {}
inline void AIL_start_sample(HSAMPLE) {}
inline void AIL_stop_sample(HSAMPLE) {}
inline void AIL_resume_sample(HSAMPLE) {}
inline void AIL_end_sample(HSAMPLE) {}
inline int AIL_sample_status(HSAMPLE) { return SMP_FREE; }
inline unsigned int AIL_sample_position(HSAMPLE) { return 0; }
inline void AIL_set_sample_position(HSAMPLE, unsigned int) {}
inline int AIL_sample_playback_rate(HSAMPLE) { return 0; }
inline float AIL_sample_volume(HSAMPLE) { return 0.0f; }
inline float AIL_sample_pan(HSAMPLE) { return 0.0f; }
inline int AIL_sample_loop_count(HSAMPLE) { return 0; }

// Stream stubs
inline HSTREAM AIL_open_stream(HDIGDRIVER, const char*, int) { return nullptr; }
inline void AIL_close_stream(HSTREAM) {}
inline void AIL_start_stream(HSTREAM) {}
inline void AIL_stop_stream(HSTREAM) {}
inline void AIL_pause_stream(HSTREAM, int) {}
inline void AIL_set_stream_loop_count(HSTREAM, int) {}
inline void AIL_set_stream_volume_levels(HSTREAM, float, float, float) {}
inline void AIL_set_stream_volume_pan(HSTREAM, float, float) {}
inline int AIL_stream_status(HSTREAM) { return STREAM_FREE; }
inline int AIL_stream_position(HSTREAM) { return 0; }
inline void AIL_set_stream_position(HSTREAM, int) {}
inline void AIL_set_stream_ms_position(HSTREAM, int) {}
inline int AIL_stream_ms_position(HSTREAM) { return 0; }
inline int AIL_stream_loop_count(HSTREAM) { return 0; }

// 3D audio stubs
inline HPROVIDER AIL_open_3D_provider(HPROVIDER) { return nullptr; }
inline void AIL_close_3D_provider(HPROVIDER) {}
inline H3DSAMPLE AIL_allocate_3D_sample_handle(HPROVIDER) { return nullptr; }
inline void AIL_release_3D_sample_handle(H3DSAMPLE) {}
inline void AIL_set_3D_position(H3DSAMPLE, float, float, float) {}
inline void AIL_set_3D_velocity(H3DSAMPLE, float, float, float) {}
inline void AIL_set_3D_velocity_vector(H3DSAMPLE, float, float, float) {}
inline void AIL_set_3D_orientation(H3DSAMPLE, float, float, float, float, float, float) {}
inline void AIL_set_3D_sample_volume(H3DSAMPLE, float) {}
inline void AIL_set_3D_sample_distances(H3DSAMPLE, float, float) {}
inline void AIL_set_3D_sample_loop_count(H3DSAMPLE, int) {}
inline void AIL_set_3D_sample_file(H3DSAMPLE, const void*) {}
inline void AIL_start_3D_sample(H3DSAMPLE) {}
inline void AIL_stop_3D_sample(H3DSAMPLE) {}
inline void AIL_resume_3D_sample(H3DSAMPLE) {}
inline int AIL_3D_sample_status(H3DSAMPLE) { return SMP_FREE; }
inline void AIL_set_3D_speaker_type(HPROVIDER, int) {}
inline void AIL_set_3D_rolloff_factor(HPROVIDER, float) {}
inline void AIL_set_3D_doppler_factor(HPROVIDER, float) {}
inline void AIL_set_3D_distance_factor(HPROVIDER, float) {}
inline int AIL_3D_sample_loop_count(H3DSAMPLE) { return 0; }
inline unsigned int AIL_3D_sample_length(H3DSAMPLE) { return 0; }
inline unsigned int AIL_3D_sample_playback_rate(H3DSAMPLE) { return 0; }
inline void AIL_set_3D_sample_playback_rate(H3DSAMPLE, unsigned int) {}
inline float AIL_3D_sample_volume(H3DSAMPLE) { return 0.0f; }

// Listener stubs
inline void AIL_set_3D_listener_position(HPROVIDER, float, float, float) {}
inline void AIL_set_3D_listener_velocity(HPROVIDER, float, float, float) {}
inline void AIL_set_3D_listener_orientation(HPROVIDER, float, float, float, float, float, float) {}

// Room type / EAX stubs
inline int AIL_set_3D_room_type(HPROVIDER, int) { return 0; }

// MIDI stubs
inline int AIL_midiOutOpen(HMDIDRIVER*, void*, int) { return 0; }
inline void AIL_midiOutClose(HMDIDRIVER) {}
inline int AIL_MIDI_handle_release(HMDIDRIVER) { return 0; }
inline int AIL_MIDI_handle_reacquire(HMDIDRIVER) { return 0; }

// Sequence stubs
inline HSEQUENCE AIL_allocate_sequence_handle(HMDIDRIVER) { return nullptr; }
inline void AIL_release_sequence_handle(HSEQUENCE) {}
inline int AIL_init_sequence(HSEQUENCE, const void*, int) { return 0; }
inline void AIL_start_sequence(HSEQUENCE) {}
inline void AIL_stop_sequence(HSEQUENCE) {}
inline void AIL_resume_sequence(HSEQUENCE) {}
inline void AIL_end_sequence(HSEQUENCE) {}
inline int AIL_sequence_status(HSEQUENCE) { return 0; }
inline void AIL_set_sequence_volume(HSEQUENCE, int, int) {}
inline void AIL_set_sequence_loop_count(HSEQUENCE, int) {}
inline int AIL_sequence_loop_count(HSEQUENCE) { return 0; }

// DLS stubs
inline HDLSDEVICE AIL_DLS_open(HMDIDRIVER, HDIGDRIVER, void*, int, void*, int, void*, int) { return nullptr; }
inline void AIL_DLS_close(HDLSDEVICE, int) {}
inline HDLSFILEID AIL_DLS_load_file(HDLSDEVICE, const char*, int) { return nullptr; }
inline void AIL_DLS_unload(HDLSDEVICE, HDLSFILEID) {}

// Timer stubs
typedef void (*AILTIMERCB)(void*);
typedef void* HTIMER;
inline HTIMER AIL_register_timer(AILTIMERCB) { return nullptr; }
inline void AIL_release_timer_handle(HTIMER) {}
inline void AIL_set_timer_frequency(HTIMER, unsigned int) {}
inline void AIL_start_timer(HTIMER) {}
inline void AIL_stop_timer(HTIMER) {}
inline void AIL_set_timer_user(HTIMER, void*) {}

// Filter stubs
inline int AIL_enumerate_filters(HPROVIDER*, HPROVIDER*, int*) { return 0; }
inline void AIL_close_filter(HPROVIDER) {}
inline int AIL_open_filter(HPROVIDER, HDIGDRIVER) { return 0; }
inline int AIL_filter_stream_attribute(HSTREAM, const char*, void*) { return 0; }
inline int AIL_set_filter_stream_preference(HSTREAM, const char*, void*) { return 0; }
inline int AIL_filter_sample_attribute(HSAMPLE, const char*, void*) { return 0; }
inline int AIL_set_filter_sample_preference(HSAMPLE, const char*, void*) { return 0; }

// File I/O stubs
inline int AIL_file_size(const char*) { return 0; }
inline void* AIL_file_read(const char*, void*) { return nullptr; }
inline void AIL_mem_free_lock(void*) {}

// WAV info
typedef struct {
    int format;
    void* data_ptr;
    unsigned int data_len;
    unsigned int rate;
    int bits;
    int channels;
    unsigned int samples;
    unsigned int block_size;
} AILSOUNDINFO;

inline int AIL_WAV_info(const void*, AILSOUNDINFO*) { return 0; }

// String helpers
inline char* AIL_get_string(char*, int, const char*, ...) { return nullptr; }

// Additional defines that might be used
#define M3D_NOPOINT     0

#endif // _MSS_STUB_H_
