#include "audio.h"

Audio *audio_init()
{
    if(SDL_Init(SDL_INIT_AUDIO) != 0)
    {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return NULL;
    } 

    Audio *audio = new Audio();

#ifdef __EMSCRIPTEN__
    const char *wavFile = "assets/wav/taunt.wav";
#else
    const char *wavFile = "taunt.wav";
#endif
    if (SDL_LoadWAV(wavFile, &audio->wavSpec, &audio->wavBuffer, &audio->wavLength) == NULL) 
    {
        SDL_Log("Error: Failed to load WAV file: %s\n", SDL_GetError());
        return NULL;
    }
    // Load second time for demodulated audio
    if (SDL_LoadWAV(wavFile, &audio->wavSpec, &audio->wavBuffer2, &audio->wavLength) == NULL) 
    {
        SDL_Log("Error: Failed to load WAV file: %s\n", SDL_GetError());
        return NULL;
    }
    memset(audio->wavBuffer2, 0, audio->wavLength);

    SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(NULL, 0, &audio->wavSpec, NULL, 0);
    if (deviceId == 0) 
    {
        SDL_Log("Error: Failed to open audio device: %s\n", SDL_GetError());
        return NULL;
    }

    audio->deviceId = deviceId;
    SDL_PauseAudioDevice(audio->deviceId, 0);

    audio->volume = 0.33f;
    for(int i=0; i<audio->wavLength; i++)
    {
        audio->wavBuffer[i] *= audio->volume;
    }

    return audio;
}

void audio_append(Audio* audio, uint8_t *data, int size)
{
    SDL_QueueAudio(audio->deviceId, data, size);
}
