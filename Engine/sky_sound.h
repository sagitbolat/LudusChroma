#pragma once

struct AudioContext;
struct SoundClip;
struct ALSource;
struct AudioSource {
    bool is_looping;
    float volume;
    ALSource* al_source; // handle to the OpenAL source
    bool IsPlaying();
};

AudioContext* InitAudioContext();
SoundClip* LoadSoundClip(const char* sound_file_path);
AudioSource* LoadAudioSource(bool source_looping = false, float volume = 1.0f); 
void PlaySoundFromSource(const AudioSource* source, const SoundClip* clip);
void FreeAudioSource(AudioSource* source);
void FreeSoundClip(SoundClip* sound_clip);
void FreeAudioContext(AudioContext* ac);


