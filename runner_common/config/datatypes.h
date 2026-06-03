#ifndef DATATYPES_H
#define DATATYPES_H

#include <stdint.h>

typedef int16_t q15_t;

/* Complex float 32-bit */
struct cf32 {
    float real;
    float imag;
};

/* Stereo float 32-bit */
struct sf32 {
    float left;
    float right;
};

/* Complex Q15 */
struct cq15 {
    int16_t real;
    int16_t imag;
};

/* Stereo Q15 */
struct sq15 {
    int16_t left;
    int16_t right;
};

#endif // DATATYPES_H
