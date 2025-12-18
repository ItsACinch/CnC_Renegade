/*
** OpenAL Audio Backend
**
** Provides audio functionality using OpenAL Soft as a replacement
** for the proprietary Miles Sound System.
*/

#ifndef OPENAL_AUDIO_H
#define OPENAL_AUDIO_H

#include <windows.h>

// OpenAL types (defined here to avoid requiring OpenAL headers everywhere)
typedef struct ALCdevice_struct ALCdevice;
typedef struct ALCcontext_struct ALCcontext;
typedef unsigned int ALuint;
typedef int ALint;
typedef float ALfloat;
typedef int ALenum;
typedef char ALboolean;

// Error codes
#define OAL_OK          0
#define OAL_ERROR      -1

// Sample status (matches Miles SMP_* constants)
#define OAL_SMP_FREE            0
#define OAL_SMP_DONE            1
#define OAL_SMP_PLAYING         2
#define OAL_SMP_STOPPED         3
#define OAL_SMP_PLAYINGBUTRELEASED 4

// Speaker types (matches Miles AIL_3D_* constants)
#define OAL_SPEAKER_2           0
#define OAL_SPEAKER_HEADPHONE   1
#define OAL_SPEAKER_SURROUND    2
#define OAL_SPEAKER_4           3

// Forward declarations
typedef struct OAL_Sample OAL_Sample;
typedef struct OAL_Sample3D OAL_Sample3D;
typedef struct OAL_Stream OAL_Stream;

// OpenAL types for external use
typedef int OAL_S32;
typedef unsigned int OAL_U32;
typedef float OAL_F32;

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Core System Functions
// ============================================================================

// Initialize OpenAL audio system
// Returns OAL_OK on success, OAL_ERROR on failure
OAL_S32 OAL_Startup(void);

// Shutdown OpenAL audio system
void OAL_Shutdown(void);

// Check if OpenAL is initialized and ready
OAL_S32 OAL_Is_Ready(void);

// Get last error message
const char* OAL_Last_Error(void);

// Process audio (call periodically for streaming updates)
void OAL_Update(void);

// ============================================================================
// 2D Sample Functions
// ============================================================================

// Allocate a 2D sample handle
OAL_Sample* OAL_Allocate_Sample(void);

// Release a 2D sample handle
void OAL_Release_Sample(OAL_Sample* sample);

// Initialize/reset a sample
void OAL_Init_Sample(OAL_Sample* sample);

// Load sample data from memory (WAV format)
// Returns OAL_OK on success
OAL_S32 OAL_Set_Sample_File(OAL_Sample* sample, void* file_data, OAL_S32 file_size);

// Playback control
void OAL_Start_Sample(OAL_Sample* sample);
void OAL_Stop_Sample(OAL_Sample* sample);
void OAL_Resume_Sample(OAL_Sample* sample);
void OAL_End_Sample(OAL_Sample* sample);

// Get sample status
OAL_S32 OAL_Sample_Status(OAL_Sample* sample);

// Volume (0-127)
void OAL_Set_Sample_Volume(OAL_Sample* sample, OAL_S32 volume);
OAL_S32 OAL_Sample_Volume(OAL_Sample* sample);

// Pan (0=left, 64=center, 127=right)
void OAL_Set_Sample_Pan(OAL_Sample* sample, OAL_S32 pan);
OAL_S32 OAL_Sample_Pan(OAL_Sample* sample);

// Loop count (0=infinite, 1=play once, etc.)
void OAL_Set_Sample_Loop_Count(OAL_Sample* sample, OAL_S32 count);
OAL_S32 OAL_Sample_Loop_Count(OAL_Sample* sample);

// Playback rate (Hz)
void OAL_Set_Sample_Playback_Rate(OAL_Sample* sample, OAL_S32 rate);
OAL_S32 OAL_Sample_Playback_Rate(OAL_Sample* sample);

// Position in milliseconds
void OAL_Set_Sample_MS_Position(OAL_Sample* sample, OAL_S32 ms);
void OAL_Sample_MS_Position(OAL_Sample* sample, OAL_S32* total_ms, OAL_S32* current_ms);

// User data storage
void OAL_Set_Sample_User_Data(OAL_Sample* sample, OAL_U32 index, OAL_S32 value);
OAL_S32 OAL_Sample_User_Data(OAL_Sample* sample, OAL_U32 index);

// ============================================================================
// 3D Sample Functions
// ============================================================================

// Allocate a 3D sample handle
OAL_Sample3D* OAL_Allocate_3D_Sample(void);

// Release a 3D sample handle
void OAL_Release_3D_Sample(OAL_Sample3D* sample);

// Load 3D sample data from memory (WAV format, must be mono)
OAL_S32 OAL_Set_3D_Sample_File(OAL_Sample3D* sample, void* file_data, OAL_S32 file_size);

// Playback control
void OAL_Start_3D_Sample(OAL_Sample3D* sample);
void OAL_Stop_3D_Sample(OAL_Sample3D* sample);
void OAL_Resume_3D_Sample(OAL_Sample3D* sample);
void OAL_End_3D_Sample(OAL_Sample3D* sample);

// Get sample status
OAL_S32 OAL_3D_Sample_Status(OAL_Sample3D* sample);

// Volume (0-127)
void OAL_Set_3D_Sample_Volume(OAL_Sample3D* sample, OAL_S32 volume);
OAL_S32 OAL_3D_Sample_Volume(OAL_Sample3D* sample);

// Loop count
void OAL_Set_3D_Sample_Loop_Count(OAL_Sample3D* sample, OAL_S32 count);
OAL_S32 OAL_3D_Sample_Loop_Count(OAL_Sample3D* sample);

// Playback rate
void OAL_Set_3D_Sample_Playback_Rate(OAL_Sample3D* sample, OAL_S32 rate);
OAL_S32 OAL_3D_Sample_Playback_Rate(OAL_Sample3D* sample);

// 3D Position
void OAL_Set_3D_Sample_Position(OAL_Sample3D* sample, OAL_F32 x, OAL_F32 y, OAL_F32 z);
void OAL_3D_Sample_Position(OAL_Sample3D* sample, OAL_F32* x, OAL_F32* y, OAL_F32* z);

// 3D Velocity (for Doppler effect)
void OAL_Set_3D_Sample_Velocity(OAL_Sample3D* sample, OAL_F32 dx, OAL_F32 dy, OAL_F32 dz);

// Distance model parameters
void OAL_Set_3D_Sample_Distances(OAL_Sample3D* sample, OAL_F32 max_dist, OAL_F32 min_dist);

// Effects level (reverb send)
void OAL_Set_3D_Sample_Effects_Level(OAL_Sample3D* sample, OAL_F32 level);

// User data
void OAL_Set_3D_Sample_User_Data(OAL_Sample3D* sample, OAL_U32 index, OAL_S32 value);
OAL_S32 OAL_3D_Sample_User_Data(OAL_Sample3D* sample, OAL_U32 index);

// Offset (sample position in bytes)
void OAL_Set_3D_Sample_Offset(OAL_Sample3D* sample, OAL_U32 offset);
OAL_U32 OAL_3D_Sample_Offset(OAL_Sample3D* sample);
OAL_U32 OAL_3D_Sample_Length(OAL_Sample3D* sample);

// ============================================================================
// Listener Functions (3D audio receiver)
// ============================================================================

// Set listener position
void OAL_Set_Listener_Position(OAL_F32 x, OAL_F32 y, OAL_F32 z);

// Set listener orientation (forward and up vectors)
void OAL_Set_Listener_Orientation(OAL_F32 fx, OAL_F32 fy, OAL_F32 fz,
                                   OAL_F32 ux, OAL_F32 uy, OAL_F32 uz);

// Set listener velocity
void OAL_Set_Listener_Velocity(OAL_F32 dx, OAL_F32 dy, OAL_F32 dz);

// Speaker configuration
void OAL_Set_Speaker_Type(OAL_S32 type);
OAL_S32 OAL_Speaker_Type(void);

// Distance/Doppler/Rolloff factors
void OAL_Set_Doppler_Factor(OAL_F32 factor);
void OAL_Set_Distance_Factor(OAL_F32 factor);
void OAL_Set_Rolloff_Factor(OAL_F32 factor);

// ============================================================================
// Streaming Functions
// ============================================================================

// Open a stream from file
OAL_Stream* OAL_Open_Stream(const char* filename);

// Close a stream
void OAL_Close_Stream(OAL_Stream* stream);

// Playback control
void OAL_Start_Stream(OAL_Stream* stream);
void OAL_Stop_Stream(OAL_Stream* stream);
void OAL_Pause_Stream(OAL_Stream* stream, OAL_S32 pause);

// Get stream status
OAL_S32 OAL_Stream_Status(OAL_Stream* stream);

// Volume
void OAL_Set_Stream_Volume(OAL_Stream* stream, OAL_S32 volume);
OAL_S32 OAL_Stream_Volume(OAL_Stream* stream);

// Pan
void OAL_Set_Stream_Pan(OAL_Stream* stream, OAL_S32 pan);
OAL_S32 OAL_Stream_Pan(OAL_Stream* stream);

// Loop
void OAL_Set_Stream_Loop_Count(OAL_Stream* stream, OAL_S32 count);
OAL_S32 OAL_Stream_Loop_Count(OAL_Stream* stream);

// Position
void OAL_Set_Stream_MS_Position(OAL_Stream* stream, OAL_S32 ms);
void OAL_Stream_MS_Position(OAL_Stream* stream, OAL_S32* total, OAL_S32* current);

// Playback rate
void OAL_Set_Stream_Playback_Rate(OAL_Stream* stream, OAL_S32 rate);
OAL_S32 OAL_Stream_Playback_Rate(OAL_Stream* stream);

// User data
void OAL_Set_Stream_User_Data(OAL_Stream* stream, OAL_U32 index, OAL_S32 value);
OAL_S32 OAL_Stream_User_Data(OAL_Stream* stream, OAL_U32 index);

// Service stream (call periodically to refill buffers)
void OAL_Service_Stream(OAL_Stream* stream);

// ============================================================================
// Master Volume
// ============================================================================

void OAL_Set_Master_Volume(OAL_S32 volume);
OAL_S32 OAL_Master_Volume(void);

// ============================================================================
// WAV Parsing Helper
// ============================================================================

typedef struct {
    OAL_S32 format;      // WAVE_FORMAT_PCM, etc.
    void* data_ptr;       // Pointer to raw PCM data
    OAL_U32 data_len;    // Length of PCM data in bytes
    OAL_U32 rate;        // Sample rate (Hz)
    OAL_S32 bits;        // Bits per sample
    OAL_S32 channels;    // Number of channels
    OAL_U32 samples;     // Number of samples
    OAL_U32 block_size;  // Block alignment
    void* initial_ptr;    // Original file data pointer (for Miles compatibility)
} OAL_SoundInfo;

// Parse WAV header and get sound info
OAL_S32 OAL_WAV_Info(void* file_data, OAL_SoundInfo* info);

#ifdef __cplusplus
}
#endif

#endif // OPENAL_AUDIO_H
