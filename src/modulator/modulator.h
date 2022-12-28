
#ifndef MODULATOR_H_
#define MODULATOR_H_

#include <complex>

typedef std::complex<double> cmplx;

typedef enum{
    MOD_BPSK,
    MOD_QPSK,
    MOD_8QAM,
    MOD_16QAM
} ModType;

typedef struct{
    int     dataLength;
    uint8_t *data;      // data [0 1]
    cmplx   *modData;   // modulated data (complex)
    uint8_t *demodData; // demodulated data [0 1]
    cmplx   *rxData;    // modulated data after channel
    
    double  noiseSNRdB; // channel noise SNR in dB

    ModType modType;
} Mod;

int modulator_run(void *userArg);

Mod *modulator_init();

void mod_demo();


#endif /* MODULATOR_H_ */