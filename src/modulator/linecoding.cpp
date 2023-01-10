#include "linecoding.h"

// TODO: NOT TESTED! Test all the coding schemes

// NRZ encoding (unipolar)
// function r = nrz(bits, oversampling)
//     os = oversampling;
//     for i = 1:numel(bits)
//         r((i-1)*os+1:i*os) = bits(i);
//     end
// end
void linecoding_nrz(int8_t *bitsIn, int size, int8_t *bitsOut, int oversampling)
{
    for (int i=0; i<size; i++)
    {
        for (int j=0; j<oversampling; j++)
        {
            bitsOut[i * oversampling + j] = bitsIn[i];
        }
    }
}
// RZ encoding (unipolar)
// function r = rz(bits, oversampling)
//     os = oversampling;
//     for i = 1:numel(bits)
//         % first half is the bit itself
//         r((i-1)*os+1:(i-0.5)*os) = bits(i);

//         % first half is 0
//         r((i-0.5)*os+1:i*os) = 0;
//     end
// end
void linecoding_rz(int8_t *bitsIn, int size, int8_t *bitsOut, int oversampling)
{
    for (int i=0; i<size; i++)
    {
        for (int j=0; j<oversampling; j++)
        {
            if (j < oversampling/2){
                // first half is the bit itself
                bitsOut[i * oversampling + j] = bitsIn[i];
            }
            else{
                // second half is always 0
                bitsOut[i * oversampling + j] = 0;
            }
        }
    }
}
// Manchester encoding (polar)
// function r = manchester(bits, oversampling)
//     os = oversampling;
//     for i = 1:numel(bits)
//         % first half is the bit itself
//         r((i-1)*os+1:(i-0.5)*os) = bits(i) * 2 - 1; % scale to -1,+1

//         % second half is inverse of the bit
//         r((i-0.5)*os+1:i*os) = (~bits(i)) * 2 - 1;  % scale to -1,+1
//     end
// end
void linecoding_manchester(int8_t *bitsIn, int size, int8_t *bitsOut, int oversampling)
{
    for (int i=0; i<size; i++)
    {
        for (int j=0; j<oversampling; j++)
        {
            if (j < oversampling/2){
                // first half is the bit itself
                bitsOut[i * oversampling + j] = bitsIn[i] * 2 - 1;  // scale to -1,+1
            }
            else{
                // second half is inverse of the bit
                bitsOut[i * oversampling + j] = (bitsIn[i] ^ 0x01) * 2 - 1; // scale to -1,+1
            }
        }
    }
}
// Differential Manchester encoding (polar)
// function r = manchester_diff(bits, oversampling)
//     os = oversampling;
//     % set state to 1
//     state = 0;
//     for i = 1:numel(bits)
//         % first half is the bit itself
//         % if current bit is '1' change the state
//         state = xor(state, bits(i));
//         r((i-1)*os+1:(i-0.5)*os) = state * 2 - 1; % scale to -1,+1

//         % second half is inverse of the bit
//         r((i-0.5)*os+1:i*os) = (~state) * 2 - 1;  % scale to -1,+1
//     end
// end
void linecoding_manchester_diff(int8_t *bitsIn, int size, int8_t *bitsOut, int oversampling)
{
    int state = 0;
    for (int i=0; i<size; i++)
    {
        for (int j=0; j<oversampling; j++)
        {
            // if current bit is '1' change the state
            state ^= bitsIn[i];

            if (j < oversampling/2){
                // first half is the bit itself
                bitsOut[i * oversampling + j] = state * 2 - 1;  // scale to -1,+1
            }
            else{
                // second half is inverse of the bit
                bitsOut[i * oversampling + j] = (state ^ 0x01) * 2 - 1; // scale to -1,+1
            }
        }
    }
}
// Miller encoding (polar)
// function r = miller(bits, oversampling)
//     os = oversampling;
//     % set last bit to 1
//     state = 1;
//     last_bit = 1;
//     for i = 1:numel(bits)
//         % first half is same as last bit's second half unless
//         % two zeros come in sequence, then state changes
//         change_state = ~(last_bit | bits(i)); % nor(state, bits(i))
//         state = xor(state, change_state);
//         r((i-1)*os+1:(i-0.5)*os) = state * 2 - 1; % scale to -1,+1

//         % second half changes if the bit is '1'
//         state = xor(bits(i), state);
//         r((i-0.5)*os+1:i*os) = state * 2 - 1;  % scale to -1,+1

//         % store current bit in last_bit
//         last_bit = bits(i);
//     end
// end
void linecoding_miller(int8_t *bitsIn, int size, int8_t *bitsOut, int oversampling)
{
    int state = 1;
    int last_bit = 1;
    for (int i=0; i<size; i++)
    {
        for (int j=0; j<oversampling; j++)
        {
            // first half is same as last bit's second half unless
            // two zeros come in sequence, then state changes
            int change_state = (last_bit | bitsIn[i]) ^ 0x01; // nor(state, bits(i))
            state = state ^ change_state;
            if (j < oversampling/2){
                bitsOut[i * oversampling + j] = state * 2 - 1;  // scale to -1,+1
            }
            else{
                // second half changes if the bit is '1'
                state = bitsIn[i] ^ state;
                bitsOut[i * oversampling + j] = state * 2 - 1; // scale to -1,+1
            }
            // store current bit in last_bit
            last_bit = bitsIn[i];
        }
    }
}