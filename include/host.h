#pragma once

#include "median_device.h"
#include "median_filter.h"

// Configure the register map, trigger START, then wait for the completion interrupt.
Status run_filter(MedianDevice& device, const std::uint8_t* input,
                  std::uint8_t* output, const ImageParams& params, int radius);
