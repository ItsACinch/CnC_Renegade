# Phase 3: Audio System Modernization

## Objective

Replace the proprietary Miles Sound System 6 with open-source alternatives while preserving the existing WWAudio abstraction layer.

## Current Miles Integration

### API Usage Summary
The codebase calls 60+ Miles API functions across these categories:

| Category | Functions | Count |
|----------|-----------|-------|
| Initialization | `AIL_startup`, `AIL_waveOutOpen`, `AIL_open_3D_provider` | 8 |
| 2D Samples | `AIL_allocate_sample_handle`, `AIL_start_sample`, etc. | 18 |
| 3D Samples | `AIL_3D_*` functions | 22 |
| Streaming | `AIL_open_stream`, `AIL_start_stream`, etc. | 12 |
| Filtering | `AIL_set_sample_processor`, `AIL_set_filter_*` | 4 |

### Abstraction Layer (WWAudio)
The existing abstraction is well-designed:

```
WWAudioClass (singleton)
├── Sound2DHandleClass   → HSAMPLE (Miles)
├── Sound3DHandleClass   → H3DSAMPLE (Miles)
├── SoundStreamHandleClass → HSTREAM (Miles)
├── SoundBufferClass     → In-memory audio data
└── Listener3DClass      → 3D listener position
```

## Replacement Strategy

### Primary: OpenAL Soft + libsndfile

| Miles Component | Replacement | Library |
|----------------|-------------|---------|
| 2D audio | OpenAL sources | OpenAL Soft |
| 3D positional audio | OpenAL 3D sources | OpenAL Soft |
| Streaming | OpenAL buffer queuing | OpenAL Soft |
| File loading | libsndfile | libsndfile |
| MP3 support | dr_mp3 or minimp3 | Single-header |
| Reverb/effects | OpenAL EFX | OpenAL Soft |

### Libraries

| Library | Version | License | Purpose |
|---------|---------|---------|---------|
| OpenAL Soft | 1.23+ | LGPL-2.1 | 3D audio API |
| libsndfile | 1.2+ | LGPL-2.1 | WAV/AIFF loading |
| dr_mp3 | latest | Public Domain | MP3 decoding |
| dr_wav | latest | Public Domain | WAV loading (alternative) |

## Implementation Plan

### 3.1 OpenAL Initialization (Replace AIL_startup)

```cpp
// Code/WWAudio/OpenALDevice.cpp

class OpenALDevice {
    ALCdevice* device = nullptr;
    ALCcontext* context = nullptr;

public:
    bool Initialize() {
        device = alcOpenDevice(nullptr);  // Default device
        if (!device) return false;

        context = alcCreateContext(device, nullptr);
        if (!context) {
            alcCloseDevice(device);
            return false;
        }

        alcMakeContextCurrent(context);
        return true;
    }

    void Shutdown() {
        alcMakeContextCurrent(nullptr);
        if (context) alcDestroyContext(context);
        if (device) alcCloseDevice(device);
    }
};
```

### 3.2 2D Sound Handle (Replace HSAMPLE)

```cpp
// Code/WWAudio/OpenAL2DHandle.cpp

class OpenAL2DHandle : public SoundHandleClass {
    ALuint source = 0;
    ALuint buffer = 0;

public:
    bool Initialize(SoundBufferClass* soundBuffer) override {
        alGenSources(1, &source);
        alGenBuffers(1, &buffer);

        // Upload audio data
        ALenum format = (soundBuffer->Get_Channels() == 1) ?
            AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

        alBufferData(buffer, format,
            soundBuffer->Get_Data(),
            soundBuffer->Get_Size(),
            soundBuffer->Get_Sample_Rate());

        alSourcei(source, AL_BUFFER, buffer);
        return true;
    }

    void Play() override {
        alSourcePlay(source);
    }

    void Stop() override {
        alSourceStop(source);
    }

    void Set_Volume(float volume) override {
        alSourcef(source, AL_GAIN, volume);
    }

    void Set_Pan(float pan) override {
        // Pan via position: left = -1, center = 0, right = 1
        alSource3f(source, AL_POSITION, pan, 0.0f, 0.0f);
    }

    void Set_Loop_Count(int count) override {
        alSourcei(source, AL_LOOPING, (count == 0) ? AL_TRUE : AL_FALSE);
        // For finite loops, need to handle in update
    }
};
```

### 3.3 3D Sound Handle (Replace H3DSAMPLE)

```cpp
// Code/WWAudio/OpenAL3DHandle.cpp

class OpenAL3DHandle : public SoundHandleClass {
    ALuint source = 0;
    ALuint buffer = 0;

public:
    void Set_Position(const Vector3& pos) override {
        alSource3f(source, AL_POSITION, pos.X, pos.Y, pos.Z);
    }

    void Set_Velocity(const Vector3& vel) override {
        alSource3f(source, AL_VELOCITY, vel.X, vel.Y, vel.Z);
    }

    void Set_Min_Max_Distance(float minDist, float maxDist) override {
        alSourcef(source, AL_REFERENCE_DISTANCE, minDist);
        alSourcef(source, AL_MAX_DISTANCE, maxDist);
    }

    void Set_Cone(float innerAngle, float outerAngle, float outerGain) {
        alSourcef(source, AL_CONE_INNER_ANGLE, innerAngle);
        alSourcef(source, AL_CONE_OUTER_ANGLE, outerAngle);
        alSourcef(source, AL_CONE_OUTER_GAIN, outerGain);
    }
};
```

### 3.4 Streaming Audio (Replace HSTREAM)

```cpp
// Code/WWAudio/OpenALStreamHandle.cpp

class OpenALStreamHandle : public SoundHandleClass {
    static const int NUM_BUFFERS = 4;
    static const int BUFFER_SIZE = 65536;

    ALuint source = 0;
    ALuint buffers[NUM_BUFFERS];
    std::unique_ptr<AudioDecoder> decoder;
    std::vector<char> decodeBuffer;

public:
    bool Initialize(const char* filename) {
        alGenSources(1, &source);
        alGenBuffers(NUM_BUFFERS, buffers);

        decoder = CreateDecoder(filename);  // WAV, MP3, etc.
        decodeBuffer.resize(BUFFER_SIZE);

        // Pre-fill buffers
        for (int i = 0; i < NUM_BUFFERS; i++) {
            FillBuffer(buffers[i]);
        }

        alSourceQueueBuffers(source, NUM_BUFFERS, buffers);
        return true;
    }

    void Update() override {
        ALint processed = 0;
        alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);

        while (processed--) {
            ALuint buffer;
            alSourceUnqueueBuffers(source, 1, &buffer);

            if (FillBuffer(buffer)) {
                alSourceQueueBuffers(source, 1, &buffer);
            }
        }

        // Restart if stopped due to buffer underrun
        ALint state;
        alGetSourcei(source, AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING) {
            alSourcePlay(source);
        }
    }

private:
    bool FillBuffer(ALuint buffer) {
        int bytesRead = decoder->Read(decodeBuffer.data(), BUFFER_SIZE);
        if (bytesRead <= 0) return false;

        alBufferData(buffer, decoder->GetFormat(),
            decodeBuffer.data(), bytesRead, decoder->GetSampleRate());
        return true;
    }
};
```

### 3.5 Listener Management

```cpp
// Code/WWAudio/OpenALListener.cpp

class OpenALListener : public Listener3DClass {
public:
    void Set_Position(const Vector3& pos) override {
        alListener3f(AL_POSITION, pos.X, pos.Y, pos.Z);
    }

    void Set_Orientation(const Vector3& forward, const Vector3& up) override {
        float orientation[6] = {
            forward.X, forward.Y, forward.Z,
            up.X, up.Y, up.Z
        };
        alListenerfv(AL_ORIENTATION, orientation);
    }

    void Set_Velocity(const Vector3& vel) override {
        alListener3f(AL_VELOCITY, vel.X, vel.Y, vel.Z);
    }
};
```

### 3.6 Reverb Effects (Replace AIL_set_filter_*)

```cpp
// Code/WWAudio/OpenALEffects.cpp

class OpenALEffects {
    ALuint effectSlot = 0;
    ALuint reverbEffect = 0;

public:
    bool Initialize() {
        if (!alcIsExtensionPresent(nullptr, "ALC_EXT_EFX"))
            return false;

        alGenAuxiliaryEffectSlots(1, &effectSlot);
        alGenEffects(1, &reverbEffect);

        alEffecti(reverbEffect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
        return true;
    }

    void SetReverbParams(float decay, float reflectTime, float level) {
        alEffectf(reverbEffect, AL_REVERB_DECAY_TIME, decay);
        alEffectf(reverbEffect, AL_REVERB_REFLECTIONS_DELAY, reflectTime);
        alEffectf(reverbEffect, AL_REVERB_GAIN, level);

        alAuxiliaryEffectSloti(effectSlot, AL_EFFECTSLOT_EFFECT, reverbEffect);
    }

    void AttachToSource(ALuint source) {
        alSource3i(source, AL_AUXILIARY_SEND_FILTER, effectSlot, 0, AL_FILTER_NULL);
    }
};
```

### 3.7 Audio File Loading

```cpp
// Code/WWAudio/AudioLoader.cpp

class AudioLoader {
public:
    static SoundBufferClass* LoadWAV(const char* filename) {
        SF_INFO info;
        SNDFILE* file = sf_open(filename, SFM_READ, &info);
        if (!file) return nullptr;

        std::vector<short> data(info.frames * info.channels);
        sf_read_short(file, data.data(), data.size());
        sf_close(file);

        return new SoundBufferClass(
            data.data(), data.size() * sizeof(short),
            info.samplerate, info.channels, 16
        );
    }

    static SoundBufferClass* LoadMP3(const char* filename) {
        drmp3 mp3;
        if (!drmp3_init_file(&mp3, filename, nullptr))
            return nullptr;

        drmp3_uint64 frameCount;
        drmp3_int16* data = drmp3_open_file_and_read_pcm_frames_s16(
            filename, nullptr, &mp3.channels, &mp3.sampleRate, &frameCount, nullptr
        );

        auto buffer = new SoundBufferClass(
            data, frameCount * mp3.channels * sizeof(short),
            mp3.sampleRate, mp3.channels, 16
        );

        drmp3_free(data, nullptr);
        return buffer;
    }
};
```

## API Mapping Reference

| Miles Function | OpenAL Equivalent |
|---------------|-------------------|
| `AIL_startup()` | `alcOpenDevice()` + `alcCreateContext()` |
| `AIL_allocate_sample_handle()` | `alGenSources()` |
| `AIL_set_named_sample_file()` | `alBufferData()` + `alSourcei(AL_BUFFER)` |
| `AIL_start_sample()` | `alSourcePlay()` |
| `AIL_stop_sample()` | `alSourceStop()` |
| `AIL_set_sample_volume()` | `alSourcef(AL_GAIN)` |
| `AIL_set_sample_pan()` | `alSource3f(AL_POSITION)` |
| `AIL_set_3D_position()` | `alSource3f(AL_POSITION)` |
| `AIL_set_3D_orientation()` | `alSource3f(AL_DIRECTION)` |
| `AIL_set_3D_sample_distances()` | `alSourcef(AL_REFERENCE_DISTANCE/MAX_DISTANCE)` |
| `AIL_3D_open_listener()` | (implicit - one listener in OpenAL) |
| `AIL_open_stream()` | Custom streaming with buffer queue |

## File Changes

| Original File | Action |
|--------------|--------|
| WWAudio.cpp | Modify initialization |
| sound2dhandle.cpp | Replace with OpenAL2DHandle |
| sound3dhandle.cpp | Replace with OpenAL3DHandle |
| soundstreamhandle.cpp | Replace with OpenALStreamHandle |
| Utils.h | Remove MMSLockClass (OpenAL is thread-safe) |
| SoundBuffer.cpp | Update format handling |

## Verification

1. All game sounds play correctly
2. 3D positional audio works with listener movement
3. Streaming music plays without gaps
4. Reverb effects in appropriate areas
5. Volume controls function properly
6. No audio glitches or pops

## Estimated Effort

- OpenAL wrapper implementation: 4-5 days
- Streaming system: 2-3 days
- Effects system: 1-2 days
- File loading: 1-2 days
- Testing/debugging: 3-4 days
- **Total: 11-16 days**

## Risks

1. **3D Provider differences**: OpenAL may spatialize differently than Miles
2. **Streaming sync**: Buffer underruns during high CPU load
3. **Format support**: Some edge-case audio formats may fail
4. **HRTF differences**: Headphone audio may sound different
