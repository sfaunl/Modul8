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
static const int MAX_SYMBOLSIZE = 16;

Mod *modulation_init()
{
    Mod *mod = new Mod();

    mod->running    = true;
    mod->numSymbols = 1000;
    mod->data       = new uint8_t[mod->numSymbols * MAX_SYMBOLSIZE];
    mod->modData    = new cmplx[mod->numSymbols];
    mod->rxData     = new cmplx[mod->numSymbols];
    mod->demodData  = new uint8_t[mod->numSymbols * MAX_SYMBOLSIZE];
    mod->modType    = MOD_16QAM;

    mod->noiseSNRdB         = 14.0f;
    mod->bitErrorRate       = 0.0f;
    mod->symbolErrorRate    = 0.0f;

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

int modulation_run(void *userArg)
{
    App *app = (App*)userArg;
    Mod *mod = app->mod;
    if(app->keepRunning)
    {
        if (app->mod->running)
        {
            int nBits = modList[mod->modType].bitSize;
            int symbolSize = mod->numSymbols;
            int dataSize = symbolSize * nBits;
            cmplx *constelList = modList[mod->modType].constel;
            
            // mod_random_nbits(mod->data, dataSize);
            mod_nbits(mod->data, dataSize);

            mod_modulate(mod->data, mod->modData, dataSize, nBits, constelList);

            mod_gaussian_channel(mod->modData, mod->rxData, symbolSize, mod->noiseSNRdB);

            mod_demodulate(mod->rxData, mod->demodData, dataSize, nBits, constelList);

            float SER = mod_symbol_error_rate(mod->data, mod->demodData, dataSize, nBits);
            float BER = mod_bit_error_rate(mod->data, mod->demodData, dataSize);

            mod->symbolErrorRate = (mod->symbolErrorRate * 0.8 + SER * 0.2);
            mod->bitErrorRate = (mod->bitErrorRate * 0.8 + BER * 0.2);
        }
    }
    else
    {
        return 0;
    }
    
    return 1;
}