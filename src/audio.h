#ifndef _AUDIO_H
#define _AUDIO_H

#include <SDL2/SDL.h>

typedef struct
{
    SDL_AudioDeviceID deviceId;
    SDL_AudioSpec wavSpec;
    uint8_t *wavBuffer;
    uint8_t *wavBuffer2;
    Uint32 wavLength;
} Audio;

Audio *audio_init();

void audio_append(Audio* audio, uint8_t *data, int size);

#endif // _AUDIO_H
