#include "modulator/modulator.h"
#include "modulation.h"
#include "app.h"

// TODO: Should it be normalized?
cmplx bpsk_constel[2] = {{-1,0}, {1,0}};
cmplx qpsk_constel[4] = {{1,1},  {-1,1}, {-1,-1}, {1,-1}};
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
    {MOD_BPSK,  1,  bpsk_constel},
    {MOD_QPSK,  2,  qpsk_constel},
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

int modulation_get_data_size(Mod *mod)
{
    int nBits = modList[mod->modType].bitSize;
    int symbolSize = mod->numSymbols;
    return symbolSize * nBits;
}

int modulation_get_symbol_size(Mod *mod)
{
    return mod->numSymbols;
}

int modulation_get_symbol_nbits(Mod *mod)
{
    return modList[mod->modType].bitSize;
}

int modulation_get_symbol_element_size(Mod *mod)
{
    return pow(2, modList[mod->modType].bitSize);
}

cmplx *modulation_get_constellation_data(Mod *mod)
{
    return modList[mod->modType].constel;
}

Mod *modulation_init()
{
    Mod *mod = new Mod();

    mod->running    = true;
    mod->numSymbols = 250;
    mod->data       = new uint8_t[MAX_SYMBOL_LENGTH * MAX_SYMBOL_ELEMENTS];
    mod->modData    = new cmplx[MAX_SYMBOL_LENGTH];
    mod->rxData     = new cmplx[MAX_SYMBOL_LENGTH];
    mod->demodData  = new uint8_t[MAX_SYMBOL_LENGTH * MAX_SYMBOL_ELEMENTS];
    mod->modType    = MOD_16QAM;

    mod->noiseSNRdB         = 14.0f;
    mod->bitErrorRate       = 0.0f;
    mod->symbolErrorRate    = 0.0f;

    // Calculate constellations
    modulation_calculate_qam(qam16_constel, 4);
    modulation_calculate_qam(qam64_constel, 6);
    modulation_calculate_qam(qam256_constel, 8);
    modulation_calculate_qam(qam1024_constel, 10);
    modulation_calculate_qam(qam4096_constel, 12);
    // Normalize constellations
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
            
            mod_random_nbits(mod->data, dataSize);
            // mod_nbits(mod->data, dataSize);

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