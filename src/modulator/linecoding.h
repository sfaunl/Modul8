
#ifndef _LINECODING_H_
#define _LINECODING_H_

#include <stdint.h>

// TODO: NOT TESTED! Test all the coding schemes

void linecoding_miller(int8_t *bitsIn, int size, int8_t *bitsOut, int oversampling);
void linecoding_manchester_diff(int8_t *bitsIn, int size, int8_t *bitsOut, int oversampling);
void linecoding_manchester(int8_t *bitsIn, int size, int8_t *bitsOut, int oversampling);
void linecoding_rz(int8_t *bitsIn, int size, int8_t *bitsOut, int oversampling);
void linecoding_nrz(int8_t *bitsIn, int size, int8_t *bitsOut, int oversampling);

#endif //_LINECODING_H_