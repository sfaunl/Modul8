#include <cmath>
#include <ctime>

#include "modulator.h"

void mod_init(unsigned int seed)
{
    srand(seed);
}

// Box muller method
// https://stackoverflow.com/questions/19944111/creating-a-gaussian-random-generator-with-a-mean-and-standard-deviation
double mod_rand_normal(float mean, float stddev)
{
    static double n2 = 0.0;
    static int n2_cached = 0;
    if (!n2_cached)
    {
        double x, y, r;
        do
        {
            x = 2.0*rand()/RAND_MAX - 1;
            y = 2.0*rand()/RAND_MAX - 1;

            r = x*x + y*y;
        }
        while (r == 0.0 || r > 1.0);

        {
            double d = sqrt(-2.0*log(r)/r);
            double n1 = x*d;
            n2 = y*d;
            double result = n1*stddev + mean;
            n2_cached = 1;
            return result;
        }
    }
    else
    {
        n2_cached = 0;
        return n2*stddev + mean;
    }
}

void mod_random_nbits(uint8_t *bits, int length)
{
    for (int i=0; i<length; i++)
    {
        bits[i] = (uint8_t)(rand() > RAND_MAX / 2); 
    }
}

float mod_signal_power(cmplx *signal, int length)
{
    float acc = 0.0;
    for (int i=0; i<length; i++)
    {
        acc += abs(pow(signal[i], 2));
    }
    return acc / length;
}

// bit error rate
float mod_bit_error_rate(uint8_t *txBits, uint8_t *rxBits, int length)
{
    float acc = 0.0;
    for (int i=0; i<length; i++)
    {
        acc += abs((int8_t)txBits[i] - (int8_t)rxBits[i]);
    }
    return acc / length;
}

// symbol error rate
// int bitSize = modList[modType].bitSize;
float mod_symbol_error_rate(uint8_t *txBits, uint8_t *rxBits, int length, int nBits)
{
    float acc = 0.0;
    for (int i=0; i<length; i+=nBits)
    {
        int symbol = 0;
        for(int bit=0; bit<nBits; bit++)
        {
            symbol += abs(txBits[i + bit] - rxBits[i + bit]);
        }
        acc += (symbol > 0) ? 1 : 0;
    }
    return acc / ((float)length / nBits);
}

// modData: input
// output : output
void mod_gaussian_channel(cmplx *symbols, cmplx *output, int length, float snr_db)
{
    float signalPower = mod_signal_power(symbols, length);

    float snr = pow(10, snr_db / 10.0); // snr linear
    float noise_power = signalPower / snr;

    for (int i=0; i<length; i++)
    {
        // generate random noise with normal dist
        float noise_r = sqrt(noise_power / 2) * mod_rand_normal(0.0, 1.0);
        float noise_i = sqrt(noise_power / 2) * mod_rand_normal(0.0, 1.0);

        // add noise to data
        output[i] = symbols[i] + cmplx(noise_r, noise_i);
    }
}

// int bitSize = modList[modType].bitSize;
// nBits should equal to log2(numElements(constelList))
void mod_modulate(uint8_t *inBits, cmplx *out, int inBitsLength, int symbolNBits, cmplx *constellationList)
{
    for (int i=0; i<inBitsLength; i+=symbolNBits)
    {
        int symbol = 0;
        for(int bit=0; bit<symbolNBits; bit++)
        {
            symbol |= inBits[i + bit];
            symbol <<= 1;
        }
        symbol >>= 1;
        out[i / symbolNBits] = constellationList[symbol];
    }
}

void mod_demodulate(cmplx *symbols, uint8_t *outBits, int outBitsLength, int symbolNBits, cmplx *constellationList)
{
    int numSymbols = outBitsLength / symbolNBits;
        int constElements = pow(2,symbolNBits);
    for (int sym = 0; sym < numSymbols; sym++) {
        float minDist = 999.0f; // TODO: replace with float_max
        uint16_t symbol = 0;
        for (int i = 0; i < constElements; i++)
        {
            float dist = abs(constellationList[i] - symbols[sym]);
            if (dist < minDist)
            { 
                minDist = dist;
                symbol = i;
            }
        }

        for(int bit = 0; bit < symbolNBits; bit++)
        {
            symbol <<= 1;
            outBits[sym * symbolNBits + bit] = (symbol & (1 << symbolNBits)) > 0;
        }
    }
}
