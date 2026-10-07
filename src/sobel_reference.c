#include "sobel_reference.h"
#include "reference_image.h"

#include <math.h>
#include <stddef.h>
#include <stdlib.h>

void sobel_reference(const uint8_t* input, int width, int height,
                     int input_stride, double gain, double* output) {
    static const int kernel_x[3][3] = {
        {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}
    };
    static const int kernel_y[3][3] = {
        {-1, -2, -1}, {0, 0, 0}, {1, 2, 1}
    };
    const ReferenceImage image = make_reference_image(input, width, height, input_stride, 1);
    int y;
    for (y = 0; y < height; ++y) {
        int x;
        for (x = 0; x < width; ++x) {
            /* The reflected border makes every 3x3 neighborhood available. */
            const uint8_t* window = image.pixels + (size_t)y * image.stride + x;
            int gx = 0;
            int gy = 0;
            int ky;
            double value;
            for (ky = 0; ky < 3; ++ky) {
                const uint8_t* row = window + (size_t)ky * image.stride;
                int kx;
                for (kx = 0; kx < 3; ++kx) {
                    const int pixel = row[kx];
                    gx += kernel_x[ky][kx] * pixel;
                    gy += kernel_y[ky][kx] * pixel;
                }
            }
            value = gain * sqrt((double)gx * gx + (double)gy * gy) / 4.0;
            output[(size_t)y * width + x] = value > 255.0 ? 255.0 : value;
        }
    }
    free(image.pixels);
}
