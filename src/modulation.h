
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

typedef enum{
    MODINPUT_RANDOM,
    MODINPUT_AUDIO
} ModInput;

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

    ModInput input;      // inputType
} Mod;

int modulation_run(void *userArg);

Mod *modulation_init();

int modulation_get_data_size(ModType mod, int numSymbols);
int modulation_get_symbol_size(ModType mod);
cmplx *modulation_get_constellation_data(ModType mod);
int modulation_get_symbol_nbits(ModType mod);
int modulation_get_symbol_element_size(ModType mod);
void mod_u8_to_bitstream(uint8_t *dataIn, int size, uint8_t *bitsOut);
void mod_bitstream_to_u8(uint8_t *bitsIn, uint8_t *dataOut, int size);

#endif /* MODULATION_H_ */