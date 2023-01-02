
#ifndef MODULATION_H_
#define MODULATION_H_

#include <complex>

typedef std::complex<float> cmplx;

typedef enum{
    MOD_ASK,
    MOD_BPSK,
    MOD_QPSK,
    MOD_8PSK,
    MOD_8QAM,
    MOD_16QAM,
    MOD_64QAM,
    MOD_256QAM,
    MOD_1024QAM,
    MOD_4096QAM
} ModType;

typedef struct{
    bool    running;
    int     numSymbols;
    uint8_t *data;      // data [0 1]
    cmplx   *modData;   // modulated data (complex)
    uint8_t *demodData; // demodulated data [0 1]
    cmplx   *rxData;    // modulated data after channel
    
    float   noiseSNRdB; // channel noise SNR in dB
    float   bitErrorRate;
    float   symbolErrorRate;
    ModType modType;
    // char    *modTypeStr;

} Mod;

int modulation_run(void *userArg);

Mod *modulation_init();

int modulation_get_data_size(Mod *mod);
int modulation_get_symbol_size(Mod *mod);
cmplx *modulation_get_constellation_data(Mod *mod);
int modulation_get_symbol_nbits(Mod *mod);
int modulation_get_symbol_element_size(Mod *mod);

#endif /* MODULATION_H_ */