#include "modulator/modulator.h"
#include "modulation.h"
#include "app.h"


// TODO: Should it be normalized?
cmplx bpsk_constel[2] = {{-1,0}, {1,0}};
cmplx qpsk_constel[4] = {{1,1}, {-1,1}, {-1,-1}, {1,-1}};
cmplx qam8_constel[8] = {{-3,1}, {-1,1}, {1,1}, {3,1}, 
                         {-3,-1}, {-1,-1}, {1,-1}, {3,-1}};
cmplx qam16_constel[16] = {{-3,3}, {-1,3}, {1,3}, {3,3},
                           {-3,1}, {-1,1}, {1,1}, {3,1},
                           {-3,-1}, {-1,-1}, {1,-1}, {3,-1},
                           {-3,-3}, {-1,-3}, {1,-3}, {3,-3}};

typedef struct{
    ModType type;
    int     bitSize;
    cmplx   *constel;
} Modulation;

Modulation modList[] = {
    {MOD_BPSK,  1,  bpsk_constel},
    {MOD_QPSK,  2,  qpsk_constel},
    {MOD_8QAM,  3,  qam8_constel},
    {MOD_16QAM, 4,  qam16_constel}
};


Mod *modulation_init()
{
    Mod *mod = (Mod*)malloc(sizeof(Mod));

    mod->dataLength = 100000;
    mod->data = (uint8_t*)malloc(mod->dataLength);
    mod->modData = (cmplx*)malloc(mod->dataLength * sizeof(cmplx));
    mod->rxData = (cmplx*)malloc(mod->dataLength * sizeof(cmplx));
    mod->demodData = (uint8_t*)malloc(mod->dataLength);
    mod->modType = MOD_16QAM;

    mod->noiseSNRdB = 14.0f;
    mod->bitErrorRate = 0.0f;
    mod->symbolErrorRate = 0.0f;

    return mod;
}

int modulation_run(void *userArg)
{
    App *app = (App*)userArg;
    Mod *mod = app->mod;
    if(app->keepRunning)
    {
        int dataSize = mod->dataLength;
        int nBits = modList[mod->modType].bitSize;
        cmplx *constelList = modList[mod->modType].constel;
        
        mod_random_nbits(mod->data, dataSize);

        mod_modulate(mod->data, mod->modData, dataSize, nBits, constelList);

        mod_gaussian_channel(mod->modData, mod->rxData, dataSize, mod->noiseSNRdB);

        mod_demodulate(mod->rxData, mod->demodData, dataSize, nBits, constelList);

        float SER = mod_symbol_error_rate(mod->data, mod->demodData, mod->dataLength, nBits);
        float BER = mod_bit_error_rate(mod->data, mod->demodData, mod->dataLength);

        mod->symbolErrorRate = (mod->symbolErrorRate * 0.8 + SER * 0.2);
        mod->bitErrorRate = (mod->bitErrorRate * 0.8 + BER * 0.2);
    }
    else
    {
        return 0;
    }
    
    return 1;
}