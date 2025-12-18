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
** OpenAL Audio Backend
**
** This module provides OpenAL-based audio functionality to replace the
** proprietary Miles Sound System. It implements the same API as the Miles
** stubs in mss.h but with actual audio playback via OpenAL Soft.
*/

#ifndef OPENAL_AUDIO_H
#define OPENAL_AUDIO_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Types and Constants
// ============================================================================

// Basic types
typedef int OAL_S32;
typedef unsigned int OAL_U32;
typedef float OAL_F32;

// Maximum handles
#define OAL_MAX_2D_SAMPLES 64
#define OAL_MAX_3D_SAMPLES 64
#define OAL_MAX_STREAMS 8

// Sample states (compatible with Miles)
#define OAL_SMP_FREE 0
#define OAL_SMP_DONE 1
#define OAL_SMP_PLAYING 2
#define OAL_SMP_STOPPED 3
#define OAL_SMP_PLAYINGBUTRELEASED 4

// Error codes
#define OAL_NO_ERROR 0
#define OAL_ERROR -1

// ============================================================================
// Initialization
// ============================================================================

OAL_S32 OAL_Startup(void);
void OAL_Shutdown(void);
const char* OAL_Get_Last_Error(void);
int OAL_Is_Available(void);

// ============================================================================
// 2D Sample API
// ============================================================================

// Handle is an index into our sample array
typedef OAL_U32 OAL_HSAMPLE;
#define OAL_INVALID_SAMPLE 0xFFFFFFFF

OAL_HSAMPLE OAL_Allocate_Sample(void);
void OAL_Release_Sample(OAL_HSAMPLE sample);
void OAL_Init_Sample(OAL_HSAMPLE sample);

// File loading - parses WAV format from buffer
OAL_S32 OAL_Set_Sample_File(OAL_HSAMPLE sample, const void* file_image, OAL_U32 file_size, const char* filename);

// Playback
void OAL_Start_Sample(OAL_HSAMPLE sample);
void OAL_Stop_Sample(OAL_HSAMPLE sample);
void OAL_Resume_Sample(OAL_HSAMPLE sample);
void OAL_End_Sample(OAL_HSAMPLE sample);
OAL_S32 OAL_Sample_Status(OAL_HSAMPLE sample);

// Properties
void OAL_Set_Sample_Volume(OAL_HSAMPLE sample, OAL_S32 volume);  // 0-127
OAL_S32 OAL_Get_Sample_Volume(OAL_HSAMPLE sample);
void OAL_Set_Sample_Pan(OAL_HSAMPLE sample, OAL_S32 pan);  // 0=left, 64=center, 127=right
OAL_S32 OAL_Get_Sample_Pan(OAL_HSAMPLE sample);
void OAL_Set_Sample_Loop_Count(OAL_HSAMPLE sample, OAL_S32 count);  // 0=infinite, 1=once, etc.
OAL_S32 OAL_Get_Sample_Loop_Count(OAL_HSAMPLE sample);
void OAL_Set_Sample_Playback_Rate(OAL_HSAMPLE sample, OAL_S32 rate);
OAL_S32 OAL_Get_Sample_Playback_Rate(OAL_HSAMPLE sample);

// Position (in milliseconds)
void OAL_Set_Sample_MS_Position(OAL_HSAMPLE sample, OAL_S32 ms);
void OAL_Get_Sample_MS_Position(OAL_HSAMPLE sample, OAL_S32* total_ms, OAL_S32* current_ms);

// User data storage (4 slots per sample)
void OAL_Set_Sample_User_Data(OAL_HSAMPLE sample, OAL_U32 index, OAL_S32 value);
OAL_S32 OAL_Get_Sample_User_Data(OAL_HSAMPLE sample, OAL_U32 index);

// ============================================================================
// 3D Sample API
// ============================================================================

typedef OAL_U32 OAL_H3DSAMPLE;
#define OAL_INVALID_3D_SAMPLE 0xFFFFFFFF

OAL_H3DSAMPLE OAL_Allocate_3D_Sample(void);
void OAL_Release_3D_Sample(OAL_H3DSAMPLE sample);
OAL_S32 OAL_Set_3D_Sample_File(OAL_H3DSAMPLE sample, const void* file_image, OAL_U32 file_size);

// Playback
void OAL_Start_3D_Sample(OAL_H3DSAMPLE sample);
void OAL_Stop_3D_Sample(OAL_H3DSAMPLE sample);
void OAL_Resume_3D_Sample(OAL_H3DSAMPLE sample);
void OAL_End_3D_Sample(OAL_H3DSAMPLE sample);
OAL_S32 OAL_3D_Sample_Status(OAL_H3DSAMPLE sample);

// Properties
void OAL_Set_3D_Sample_Volume(OAL_H3DSAMPLE sample, OAL_S32 volume);
OAL_S32 OAL_Get_3D_Sample_Volume(OAL_H3DSAMPLE sample);
void OAL_Set_3D_Sample_Loop_Count(OAL_H3DSAMPLE sample, OAL_S32 count);
OAL_S32 OAL_Get_3D_Sample_Loop_Count(OAL_H3DSAMPLE sample);
void OAL_Set_3D_Sample_Playback_Rate(OAL_H3DSAMPLE sample, OAL_S32 rate);
OAL_S32 OAL_Get_3D_Sample_Playback_Rate(OAL_H3DSAMPLE sample);

// 3D Position/Orientation
void OAL_Set_3D_Sample_Position(OAL_H3DSAMPLE sample, OAL_F32 x, OAL_F32 y, OAL_F32 z);
void OAL_Get_3D_Sample_Position(OAL_H3DSAMPLE sample, OAL_F32* x, OAL_F32* y, OAL_F32* z);
void OAL_Set_3D_Sample_Velocity(OAL_H3DSAMPLE sample, OAL_F32 dx, OAL_F32 dy, OAL_F32 dz);
void OAL_Set_3D_Sample_Distances(OAL_H3DSAMPLE sample, OAL_F32 max_dist, OAL_F32 min_dist);

// User data
void OAL_Set_3D_Sample_User_Data(OAL_H3DSAMPLE sample, OAL_U32 index, OAL_S32 value);
OAL_S32 OAL_Get_3D_Sample_User_Data(OAL_H3DSAMPLE sample, OAL_U32 index);

// Offset in samples
void OAL_Set_3D_Sample_Offset(OAL_H3DSAMPLE sample, OAL_U32 offset);
OAL_U32 OAL_Get_3D_Sample_Offset(OAL_H3DSAMPLE sample);
OAL_U32 OAL_Get_3D_Sample_Length(OAL_H3DSAMPLE sample);

// ============================================================================
// Listener API (for 3D audio)
// ============================================================================

void OAL_Set_Listener_Position(OAL_F32 x, OAL_F32 y, OAL_F32 z);
void OAL_Get_Listener_Position(OAL_F32* x, OAL_F32* y, OAL_F32* z);
void OAL_Set_Listener_Orientation(OAL_F32 fx, OAL_F32 fy, OAL_F32 fz, OAL_F32 ux, OAL_F32 uy, OAL_F32 uz);
void OAL_Set_Listener_Velocity(OAL_F32 dx, OAL_F32 dy, OAL_F32 dz);

// Global 3D settings
void OAL_Set_Doppler_Factor(OAL_F32 factor);
void OAL_Set_Distance_Factor(OAL_F32 factor);
void OAL_Set_Rolloff_Factor(OAL_F32 factor);

// ============================================================================
// Streaming API
// ============================================================================

typedef OAL_U32 OAL_HSTREAM;
#define OAL_INVALID_STREAM 0xFFFFFFFF

OAL_HSTREAM OAL_Open_Stream(const char* filename);
void OAL_Close_Stream(OAL_HSTREAM stream);
void OAL_Start_Stream(OAL_HSTREAM stream);
void OAL_Stop_Stream(OAL_HSTREAM stream);
void OAL_Pause_Stream(OAL_HSTREAM stream, OAL_S32 pause);
OAL_S32 OAL_Stream_Status(OAL_HSTREAM stream);
void OAL_Set_Stream_Volume(OAL_HSTREAM stream, OAL_S32 volume);
OAL_S32 OAL_Get_Stream_Volume(OAL_HSTREAM stream);
void OAL_Set_Stream_Pan(OAL_HSTREAM stream, OAL_S32 pan);
OAL_S32 OAL_Get_Stream_Pan(OAL_HSTREAM stream);
void OAL_Set_Stream_Loop_Count(OAL_HSTREAM stream, OAL_S32 count);
OAL_S32 OAL_Get_Stream_Loop_Count(OAL_HSTREAM stream);
void OAL_Set_Stream_MS_Position(OAL_HSTREAM stream, OAL_S32 ms);
void OAL_Get_Stream_MS_Position(OAL_HSTREAM stream, OAL_S32* total, OAL_S32* current);
void OAL_Service_Stream(OAL_HSTREAM stream);

// ============================================================================
// File Callbacks (for loading from MIX archives)
// ============================================================================

typedef OAL_U32 (*OAL_FileOpenCallback)(const char* filename, OAL_U32* file_handle);
typedef void (*OAL_FileCloseCallback)(OAL_U32 file_handle);
typedef OAL_S32 (*OAL_FileSeekCallback)(OAL_U32 file_handle, OAL_S32 offset, OAL_U32 type);
typedef OAL_U32 (*OAL_FileReadCallback)(OAL_U32 file_handle, void* buffer, OAL_U32 bytes);

void OAL_Set_File_Callbacks(
    OAL_FileOpenCallback open_cb,
    OAL_FileCloseCallback close_cb,
    OAL_FileSeekCallback seek_cb,
    OAL_FileReadCallback read_cb
);

// ============================================================================
// WAV Parsing
// ============================================================================

typedef struct {
    OAL_S32 format;      // WAVE_FORMAT_PCM or WAVE_FORMAT_IMA_ADPCM
    void* data_ptr;      // Pointer to audio data
    OAL_U32 data_len;    // Length of audio data in bytes
    OAL_U32 rate;        // Sample rate (e.g., 22050, 44100)
    OAL_S32 bits;        // Bits per sample (8 or 16)
    OAL_S32 channels;    // 1 = mono, 2 = stereo
    OAL_U32 samples;     // Total number of samples
    OAL_U32 block_size;  // Block alignment
    void* initial_ptr;   // Original buffer pointer
} OAL_SoundInfo;

OAL_S32 OAL_Parse_WAV(const void* file_image, OAL_U32 file_size, OAL_SoundInfo* info);

// ============================================================================
// Timer API
// ============================================================================

typedef void (*OAL_TimerCallback)(OAL_U32 user_data);
typedef OAL_U32 OAL_HTIMER;
#define OAL_INVALID_TIMER 0xFFFFFFFF

OAL_HTIMER OAL_Register_Timer(OAL_TimerCallback callback);
void OAL_Release_Timer(OAL_HTIMER timer);
void OAL_Set_Timer_Period(OAL_HTIMER timer, OAL_U32 microseconds);
void OAL_Set_Timer_User(OAL_HTIMER timer, OAL_U32 user_data);
void OAL_Start_Timer(OAL_HTIMER timer);
void OAL_Stop_Timer(OAL_HTIMER timer);

// ============================================================================
// Utility
// ============================================================================

void OAL_Delay(OAL_S32 milliseconds);
OAL_U32 OAL_Microseconds(void);

#ifdef __cplusplus
}
#endif

#endif // OPENAL_AUDIO_H
