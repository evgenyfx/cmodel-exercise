#ifndef REFERENCE_IMAGE_H
#define REFERENCE_IMAGE_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ReferenceImage {
    uint8_t* pixels;
    size_t stride;
} ReferenceImage;

/* Build a private reflect-101 padded copy for the reference filters.
 * The caller supplies valid image dimensions, stride, and border size.
 */
static ReferenceImage make_reference_image(const uint8_t* input, int width,
                                           int height, int input_stride, int border)
{
    ReferenceImage image;
    int y;
    int distance;
    image.stride = (size_t)width + 2 * border;
    image.pixels = (uint8_t*)malloc(((size_t)height + 2 * border) * image.stride);
    if (!image.pixels) {
        fputs("Cannot allocate padded reference image\n", stderr);
        abort();
    }

    for (y = 0; y < height; ++y) {
        uint8_t* row = image.pixels + ((size_t)y + border) * image.stride;
        memcpy(row + border, input + (size_t)y * input_stride, (size_t)width);

        /* Fill outward. On narrow images, earlier layers on the opposite
         * side supply the pixels needed for repeated reflection. */
        for (distance = 1; distance <= border; ++distance) {
            row[border - distance] = row[width == 1 ? border : border + distance];
            row[border + width - 1 + distance] =
                row[width == 1 ? border : border + width - 1 - distance];
        }
    }

    /* Reflect complete rows, including their already-filled corner pixels. */
    for (distance = 1; distance <= border; ++distance) {
        const int top_source = height == 1 ? border : border + distance;
        const int bottom_source = height == 1 ? border : border + height - 1 - distance;
        memcpy(image.pixels + (size_t)(border - distance) * image.stride,
               image.pixels + (size_t)top_source * image.stride, image.stride);
        memcpy(image.pixels + (size_t)(border + height - 1 + distance) * image.stride,
               image.pixels + (size_t)bottom_source * image.stride, image.stride);
    }
    return image;
}

#endif
