#include "modulator/modulator.h"
#include "modulation.h"
#include "app.h"

// TODO: Should it be normalized?
cmplx ask_constel[2] = {{1,0}, {0,0}};
cmplx bpsk_constel[2] = {{-1,0}, {1,0}};
cmplx qpsk_constel[4] = {{1,1},  {-1,1}, {-1,-1}, {1,-1}};
cmplx psk8_constel[8] = {{M_SQRT1_2, 0}, {0.5, 0.5}, {-0, M_SQRT1_2}, {-0.5,-0.5},
                         {-M_SQRT1_2,0}, {-0.5,0.5}, {-0,-M_SQRT1_2}, {0.5, -0.5}};
cmplx qam8_constel[8] = {{-3,1},  {-1,1},  {1,1},  {3,1}, 
                         {-3,-1}, {-1,-1}, {1,-1}, {3,-1}};
cmplx qam16_constel[16] = {{-3,3},  {-1,3},  {1,3},  {3,3},
                           {-3,1},  {-1,1},  {1,1},  {3,1},
                           {-3,-1}, {-1,-1}, {1,-1}, {3,-1},
                           {-3,-3}, {-1,-3}, {1,-3}, {3,-3}};
cmplx qam64_constel[64] = { {-7,7},  {-5,7},  {-3,7},  {-1,7},  {1,7},  {3,7},  {5,7},  {7,7}, 
                            {-7,5},  {-5,5},  {-3,5},  {-1,5},  {1,5},  {3,5},  {5,5},  {7,5}, 
                            {-7,3},  {-5,3},  {-3,3},  {-1,3},  {1,3},  {3,3},  {5,3},  {7,3}, 
                            {-7,1},  {-5,1},  {-3,1},  {-1,1},  {1,1},  {3,1},  {5,1},  {7,1}, 
                            {-7,-1}, {-5,-1}, {-3,-1}, {-1,-1}, {1,-1}, {3,-1}, {5,-1}, {7,-1}, 
                            {-7,-3}, {-5,-3}, {-3,-3}, {-1,-3}, {1,-3}, {3,-3}, {5,-3}, {7,-3}, 
                            {-7,-5}, {-5,-5}, {-3,-5}, {-1,-5}, {1,-5}, {3,-5}, {5,-5}, {7,-5},
                            {-7,-7}, {-5,-7}, {-3,-7}, {-1,-7}, {1,-7}, {3,-7}, {5,-7}, {7,-7}};
cmplx qam256_constel[256] ={{-15,15},  }; 
cmplx qam1024_constel[1024] ={{-31,31}, }; 
cmplx qam4096_constel[4096] ={{-63,63}, }; 


typedef struct{
    ModType type;
    int     bitSize;
    cmplx   *constel;
} Modulation;

Modulation modList[] = {
    {MOD_ASK,   1,  ask_constel},
    {MOD_BPSK,  1,  bpsk_constel},
    {MOD_QPSK,  2,  qpsk_constel},
    {MOD_8PSK,  3,  psk8_constel},
    {MOD_8QAM,  3,  qam8_constel},
    {MOD_16QAM, 4,  qam16_constel},
    {MOD_64QAM, 6,  qam64_constel},
    {MOD_256QAM, 8,  qam256_constel},
    {MOD_1024QAM, 10,  qam1024_constel},
    {MOD_4096QAM, 12,  qam4096_constel}
};
static const int MAX_SYMBOL_ELEMENTS = 4096;
static const int MAX_SYMBOL_LENGTH = 100000;

void modulation_calculate_qam(cmplx *constel, int nBits)
{
    int k = sqrt(pow(2,nBits));
    for (int j=0; j<k; j++)
    {
        for (int i=0; i<k; i++)
        {
            constel[j * k + i] =  cmplx(-pow(2, nBits / 2) + i * 2 + 1,
                                            -pow(2, nBits / 2) + j * 2 + 1);
        }
    }
}


int modulation_get_data_size(ModType mod, int numSymbols)
{
    int nBits = modList[mod].bitSize;
    return numSymbols * nBits;
}

int modulation_get_symbol_nbits(ModType mod)
{
    return modList[mod].bitSize;
}

int modulation_get_symbol_element_size(ModType mod)
{
    return pow(2, modList[mod].bitSize);
}

cmplx *modulation_get_constellation_data(ModType mod)
{
    return modList[mod].constel;
}

Mod *modulation_init()
{
    Mod *mod = new Mod();

    mod->running    = true;
    mod->numSymbols = 500;
    mod->data       = new uint8_t[MAX_SYMBOL_LENGTH * MAX_SYMBOL_ELEMENTS];
    mod->modData    = new cmplx[MAX_SYMBOL_LENGTH];
    mod->rxData     = new cmplx[MAX_SYMBOL_LENGTH];
    mod->demodData  = new uint8_t[MAX_SYMBOL_LENGTH * MAX_SYMBOL_ELEMENTS];
    mod->modType    = MOD_256QAM;

    mod->noiseSNRdB         = 30.0f;
    mod->bitErrorRate       = 0.0f;
    mod->symbolErrorRate    = 0.0f;

    // Calculate constellations
    modulation_calculate_qam(qam16_constel, 4);
    modulation_calculate_qam(qam64_constel, 6);
    modulation_calculate_qam(qam256_constel, 8);
    modulation_calculate_qam(qam1024_constel, 10);
    modulation_calculate_qam(qam4096_constel, 12);
    // Normalize constellations
    for (int i=0; i<8; i++) psk8_constel[i] /= M_SQRT1_2;
    for (int i=0; i<8; i++) qam8_constel[i] /= 3.0f;
    for (int i=0; i<16; i++) qam16_constel[i] /= 3.0f;
    for (int i=0; i<64; i++) qam64_constel[i] /= 7.0f;
    for (int i=0; i<256; i++) qam256_constel[i] /= 15.0f;
    for (int i=0; i<1024; i++) qam1024_constel[i] /= 31.0f;
    for (int i=0; i<4096; i++) qam4096_constel[i] /= 63.0f;

    return mod;
}

void mod_nbits(uint8_t *bits, int length)
{   
    int bit = 1;
    for (int i=0; i<length; i++)
    {
        bits[i] = bit; 
        bit ^= 1;
    }
}

void mod_u8_to_bitstream(uint8_t *dataIn, int size, uint8_t *bitsOut)
{   
    for (int i=0; i<size; i++)
    {
        uint8_t buf = dataIn[i];
        for (int j=0; j<8; j++)
        {
            bitsOut[i*8+j] = buf & 0x01;
            buf >>= 1;
        }
    }
}

void mod_bitstream_to_u8(uint8_t *bitsIn, uint8_t *dataOut, int size)
{   
    for (int i=0; i<size; i++)
    {
        uint8_t buf = 0;
        for (int j=0; j<8; j++)
        {
            buf |= (bitsIn[i*8+j] << j);
        }
        dataOut[i] = buf;
    }
}

void modulate_bits(void *arg, uint8_t *bitsIn, uint8_t *bitsOut, int size)
{
    Mod *mod = (Mod*)arg;
    int nBits = modulation_get_symbol_nbits(mod->modType);
    cmplx *constelList = modulation_get_constellation_data(mod->modType);
    int symbolSize = size / nBits;
    mod->numSymbols = symbolSize;

    mod_modulate(bitsIn, mod->modData, size, nBits, constelList);
    mod_gaussian_channel(mod->modData, mod->rxData, symbolSize, mod->noiseSNRdB);
    mod_demodulate(mod->rxData, bitsOut, size, nBits, constelList);

    float SER = mod_symbol_error_rate(bitsIn, bitsOut, size, nBits);
    float BER = mod_bit_error_rate(bitsIn, bitsOut, size);

    mod->symbolErrorRate = (mod->symbolErrorRate * 0.8 + SER * 0.2);
    mod->bitErrorRate = (mod->bitErrorRate * 0.8 + BER * 0.2);
}

void modulate_bytes(Mod *mod, uint8_t *dataIn, uint8_t *dataOut, int sizeBytes)
{
    int size = sizeBytes * 8;

    mod_u8_to_bitstream(dataIn, sizeBytes, mod->data);
    modulate_bits(mod, mod->data, mod->demodData, size);
    mod_bitstream_to_u8(mod->demodData, dataOut, sizeBytes);
}

void modulate_audio(void *arg, uint8_t *dataIn, uint8_t *dataOut, int size)
{
    App* app = (App*)arg;
    static int index = 0;
    static int chunkSize = 512; // bytes

    // Wait for the audio buffer to get under a threshold to synchronize the audio
    // TODO: Find a smart way to set the threshold
    if (SDL_GetQueuedAudioSize(app->audio->deviceId) < (Uint32)chunkSize * 10)
    {
        modulate_bytes(app->mod, &dataIn[index], &dataOut[index], chunkSize);

        audio_append(app->audio, &dataOut[index], chunkSize);
        index += chunkSize;

        if (index + chunkSize  > size)
        {
            index = 0;
        }
    }
}

void modulate_random(void *arg, int size)
{
    App* app = (App*)arg;
    Mod *mod = app->mod;

    mod_random_nbits(mod->data, size);
    modulate_bits(mod, mod->data, mod->demodData, size);
}

int modulation_run(void *userArg)
{
    App *app = (App*)userArg;

    if(app->keepRunning)
    {
        if (app->mod->running)
        {
            switch(app->mod->input)
            {
                case MODINPUT_RANDOM:
                    modulate_random(app, modulation_get_data_size(app->mod->modType, app->mod->numSymbols));
                    break;
                case MODINPUT_AUDIO:
                    modulate_audio(app, app->audio->wavBuffer, app->audio->wavBuffer2, app->audio->wavLength);
                    break;
            }
        }
    }
    else
    {
        return 0;
    }
    
    return 1;
}