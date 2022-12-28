#include <cmath>
#include <ctime>

#include "../app.h"
#include "statistics.h"

#include "modulator.h"

#define SCALE_LINEAR(input, inputMin, inputMax, outputMin, outputMax) (((input)-(inputMin))*((outputMax)-(outputMin))/((inputMax)-(inputMin))+(outputMin))

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


void generate_random_bits(uint8_t *bits, int length)
{
    for (int i=0; i<length; i++)
    {
        bits[i] = rand() > RAND_MAX / 2; 
    }
}

float calculate_signal_power(cmplx *signal, int length)
{
    float acc = 0.0;
    for (int i=0; i<length; i++)
    {
        acc += abs(pow(signal[i], 2));
    }
    return acc / length;
}

// bit error rate
float calculate_ber(uint8_t *txBits, uint8_t *rxBits, int length)
{
    float acc = 0.0;
    for (int i=0; i<length; i++)
    {
        acc += abs(txBits[i] - rxBits[i]);
    }
    return acc / length;
}

// symbol error rate
float calculate_ser(uint8_t *txBits, uint8_t *rxBits, int length, ModType modType)
{
    int bitSize = modList[modType].bitSize;

    float acc = 0.0;
    for (int i=0; i<length; i+=bitSize)
    {
        int symbol = 0;
        for(int bit=0; bit<bitSize; bit++)
        {
            symbol += abs(txBits[i + bit] - rxBits[i + bit]);
        }
        acc += (symbol > 0) ? 1 : 0;
    }
    return acc / ((float)length / bitSize);
}

// modData: input
// rxData : output
void channel(cmplx *symbols, cmplx *rxData, int length, float snr_db)
{
    float signalPower = calculate_signal_power(symbols, length);

    float snr = pow(10, snr_db / 10.0); // snr linear
    float noise_power = signalPower / snr;

    // generate random noise with normal dist
    cmplx *noise = (cmplx*)malloc(length * sizeof(cmplx));;
    for (int i=0; i<length; i++)
    {
        float noise_r = sqrt(noise_power / 2) * rand_normal(0.0, 1.0);
        float noise_i = sqrt(noise_power / 2) * rand_normal(0.0, 1.0);
        noise[i] = cmplx(noise_r, noise_i);
    }

    // add noise to data
    for (int i=0; i<length; i++)
    {
        rxData[i] = symbols[i] + noise[i];
    }
    free(noise);
}

void modulate(uint8_t *inBits, cmplx *out, int length, ModType modType)
{
    int bitSize = modList[modType].bitSize;
    for (int i=0; i<length; i+=bitSize)
    {
        int symbol = 0;
        for(int bit=0; bit<bitSize; bit++)
        {
            symbol |= inBits[i + bit];
            symbol <<= 1;
        }
        symbol >>= 1;
        out[i / bitSize] = modList[modType].constel[symbol];
    }
}

// TODO only does BPSK currently
void demodulate(cmplx *symbols, uint8_t *outBits, int length, ModType modType)
{
    (void)modType;
    for (int i = 0; i < length; i++) {
        outBits[i] = (real(symbols[i]) < 0) ? 0 : 1;
    }
}

Mod *modulator_init()
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

int modulator_run(void *userArg)
{
    App *app = (App*)userArg;
    Mod *mod = app->mod;
    if(app->keepRunning)
    {
        int bits = mod->dataLength;
        generate_random_bits(mod->data, bits);

        modulate(mod->data, mod->modData, bits, mod->modType);

        channel(mod->modData, mod->rxData, bits, mod->noiseSNRdB);

        demodulate(mod->rxData, mod->demodData, bits, mod->modType);

        float SER = calculate_ser(mod->data, mod->demodData, mod->dataLength, mod->modType);
        float BER = calculate_ber(mod->data, mod->demodData, mod->dataLength);

        mod->symbolErrorRate = (mod->symbolErrorRate * 0.8 + SER * 0.2);
        mod->bitErrorRate = (mod->bitErrorRate * 0.8 + BER * 0.2);

    }
    else
    {
        return 0;
    }
    
    return 1;
}