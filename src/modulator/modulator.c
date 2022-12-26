#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include "../app.h"
#include "statistics.h"

#include "modulator.h"

#define SCALE_LINEAR(input, inputMin, inputMax, outputMin, outputMax) (((input)-(inputMin))*((outputMax)-(outputMin))/((inputMax)-(inputMin))+(outputMin))

// TODO: check if these constellations are correct
double complex bpsk_constel[2] = {-1,1};
double complex qpsk_constel[4] = {1+1i, -1+1i, -1-1i, 1-1i};
double complex qam8_constel[8] = {1+1i, -1+1i, -1-1i, 1-1i};
double complex qam16_constel[16] = {1+1i, -1+1i, -1-1i, 1-1i};

typedef enum{
    MOD_BPSK,
    MOD_QPSK,
    MOD_8QAM,
    MOD_16QAM
} ModType;

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

double calculate_signal_power(cmplx *signal, int length)
{
    double acc = 0.0;
    for (int i=0; i<length; i++)
    {
        acc += cabs(cpow(signal[i], 2));
    }
    return acc / length;
}

// bit error rate
double calculate_ber(uint8_t *txBits, uint8_t *rxBits, int length)
{
    double acc = 0.0;
    for (int i=0; i<length; i++)
    {
        acc += abs(txBits[i] - rxBits[i]);
    }
    return acc / length;
}

// symbol error rate
double calculate_ser(uint8_t *txBits, uint8_t *rxBits, int length, ModType modType)
{
    int bitSize = modList[modType].bitSize;

    double acc = 0.0;
    for (int i=0; i<length; i+=bitSize)
    {
        int symbol = 0;
        for(int bit=0; bit<bitSize; bit++)
        {
            symbol += abs(txBits[i + bit] - rxBits[i + bit]);
        }
        acc += (symbol > 0) ? 1 : 0;
    }
    return acc / ((double)length / bitSize);
}

// modData: input
// rxData : output
void channel(cmplx *symbols, cmplx *rxData, int length, double snr_db)
{
    double signalPower = calculate_signal_power(symbols, length);

    double snr = pow(10, snr_db / 10.0); // snr linear
    double noise_power = signalPower / snr;

    // generate random noise with normal dist
    double *rand_n = malloc(length * sizeof(double));
    statistics_dist_normal(rand_n, length, 0.0, 1.0);
    
    cmplx noise[length];
    for (int i=0; i<length; i++)
    {
        noise[i] = sqrt(noise_power / 2) * (1 + 1i) * rand_n[i];
    }

    // add noise to data
    for (int i=0; i<length; i++)
    {
        rxData[i] = symbols[i] + noise[i];
    }
    free(rand_n);
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
        outBits[i] = (creal(symbols[i]) < 0) ? 0 : 1;
    }
}

void mod_demo()
{
    #define NUM_SNR 20

    // srand(time(0));
    srand(0);
    Mod mod;
    mod.dataLength = 100000;
    mod.data = malloc(mod.dataLength);
    mod.modData = malloc(mod.dataLength * sizeof(cmplx));
    mod.rxData = malloc(mod.dataLength * sizeof(cmplx));
    mod.demodData = malloc(mod.dataLength);

    generate_random_bits(mod.data, mod.dataLength);
    modulate(mod.data, mod.modData, mod.dataLength, MOD_BPSK);

    double ber[NUM_SNR];
    for (int SNR_DB = 0; SNR_DB < NUM_SNR; SNR_DB++)
    {
        channel(mod.modData, mod.rxData, mod.dataLength, SNR_DB);

        demodulate(mod.rxData, mod.demodData, mod.dataLength, MOD_BPSK);

        ber[SNR_DB] = calculate_ber(mod.data, mod.demodData, mod.dataLength);
    }

    for (int i=0; i<NUM_SNR; i++)
    {
        printf("%.6f ", ber[i] );
    }
    printf("\n");
}


Mod *modulator_init()
{
    Mod *mod = malloc(sizeof(Mod));

    mod->dataLength = 100000;
    mod->data = malloc(mod->dataLength);
    mod->modData = malloc(mod->dataLength * sizeof(cmplx));
    mod->rxData = malloc(mod->dataLength * sizeof(cmplx));
    mod->demodData = malloc(mod->dataLength);

    mod->noiseSNRdB = 2;

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

        modulate(mod->data, mod->modData, bits, MOD_BPSK);

        channel(mod->modData, mod->rxData, bits, mod->noiseSNRdB);

        demodulate(mod->rxData, mod->demodData, bits, MOD_BPSK);

        static double SER = 0;  // symbol error rate
        static double BER = 0;  // bit error rate
        
        double _SER = calculate_ser(mod->data, mod->demodData, mod->dataLength, MOD_BPSK);
        double _BER = calculate_ber(mod->data, mod->demodData, mod->dataLength);

        SER = (SER * 0.98 + _SER * 0.02);
        BER = (BER * 0.98 + _BER * 0.02);

        for (int i=0; i<NUM_SNR; i++)
        {
            printf("%.6f ", BER );
        }
        printf("\n");
    }
    else
    {
        return 0;
    }
    
    return 1;
}