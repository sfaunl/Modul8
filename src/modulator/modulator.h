
#ifndef MODULATOR_H_
#define MODULATOR_H_

#include <complex>
#include <stdint.h>

typedef std::complex<float> cmplx;

// Initializes random seed
void mod_init(unsigned int seed);

void mod_random_nbits(uint8_t *bits, int length);
float mod_signal_power(cmplx *signal, int length);
float mod_bit_error_rate(uint8_t *txBits, uint8_t *rxBits, int length);
float mod_symbol_error_rate(uint8_t *txBits, uint8_t *rxBits, int length, int nBits);
void mod_gaussian_channel(cmplx *symbols, cmplx *output, int length, float snr_db);
void mod_modulate(uint8_t *inBits, cmplx *out, int length, int nBits, cmplx *constellationList);
void mod_demodulate(cmplx *symbols, uint8_t *outBits, int length, int nBits, cmplx *constellationList);


#endif /* MODULATOR_H_ */