#ifndef SOBEL_REFERENCE_H
#define SOBEL_REFERENCE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * Floating-point reference for the Sobel-magnitude filter.
 */
void sobel_reference(const uint8_t* input, int width, int height,
                     int input_stride, double gain, double* output);

#ifdef __cplusplus
}
#endif

#endif
