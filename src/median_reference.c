#include "median_filter.h"
#include "reference_image.h"

#include <stddef.h>
#include <stdlib.h>

static int compare_pixels(const void* left, const void* right)
{
    return (int)*(const uint8_t*)left - (int)*(const uint8_t*)right;
}

void median_reference(const uint8_t* input, uint8_t* output,
                      const ImageParams* params, int radius)
{
    uint8_t samples[(2 * MEDIAN_MAX_RADIUS + 1) *
                    (2 * MEDIAN_MAX_RADIUS + 1)];
    const ReferenceImage image = make_reference_image(input, params->width,
                                                      params->height,
                                                      params->input_stride, radius);
    const int diameter = 2 * radius + 1;
    int y;
    int x;

    for (y = 0; y < params->height; ++y) {
        for (x = 0; x < params->width; ++x) {
            /* Top-left of the neighborhood centered on the original pixel. */
            const uint8_t* window = image.pixels + (size_t)y * image.stride + x;
            int count = 0;
            int dy;
            int dx;

            for (dy = 0; dy < diameter; ++dy) {
                const uint8_t* row = window + (size_t)dy * image.stride;
                for (dx = 0; dx < diameter; ++dx) {
                    samples[count++] = row[dx];
                }
            }

            qsort(samples, (size_t)count, sizeof(samples[0]), compare_pixels);
            output[(size_t)y * params->output_stride + x] = samples[count / 2];
        }
    }
    free(image.pixels);
}
