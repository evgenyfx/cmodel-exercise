# Candidate exercises

Build/run instructions are in [README.md](README.md); the current device interface is in [REGISTER_MAP.md](REGISTER_MAP.md). Keep changes focused and reference implementations independent. We will discuss your decisions and evidence during the interview.
You may take all or any subset of exercises.

## Analyze host/device communication

Read the host, device, and event implementations. Explain how a job moves through configuration, processing, and completion; how the threads synchronize; and the assumptions about register access and buffer lifetimes.

## Exercise 1: run the sample and fix the general bug

Run `./build/median_demo` with the supplied `data/lena_mono.y4m`. The job completes, but the model output differs from the reference. Find and fix the cause, keeping the histogram-based algorithm and register/event interface. Explain the fix and show that the sample now matches.

## Exercise 2: reproduce and fix the small-image bug

After exercise 1, a second defect remains on small images with filter windows larger than the image. Create an artificial in-memory image to expose it; choose the pixel values, dimensions, and radius yourself. Find and fix the cause, explain the expected output, and add a regression test.

Show that the test fails with only the exercise 1 fix and passes after exercise 2. Recheck the Lena sample. Add your regression to `tests/median_tests.cpp` and run `./build/median_tests` without arguments.

## Exercise 3: implement fixed-point Sobel and estimate error

**The target device has no floating-point support. The model must use integer arithmetic only, including all helpers and per-job setup.**

Add Sobel edge magnitude to the existing model thread, with host/demo support. Keep existing median calls and command-line usage working, including optional Y4M saving.

Choose and explain the fixed-point representation, intermediate widths/ranges, magnitude approximation, rounding, and clipping. The host converts the requested real-valued gain into a register code; document its rounding. Floating point is allowed in host gain conversion, reference, and evaluation code; image processing and per-job approximation calculations belong on the device.

Run your implementation and the floating-point reference on the same image with the same requested gain. Compare the device's integer pixel values with the reference's floating-point values, without rounding the reference first. Choose how to measure the differences, explain how much error you consider acceptable, and report whether your implementation meets that limit. Exact agreement is not required.

Support successive jobs using either filter on the same device. Define and document parameter validation and error handling.

## Specifications

Both model filters use 8-bit monochrome input/output, with unchanged dimensions from 1 to 4096 on each axis. Use the existing image descriptor and separate input/output byte strides, each at least the width. The host provides valid, non-overlapping buffers of at least `(height - 1) * stride + width` bytes. Write only visible output pixels; preserve input, padding, and trailing bytes. Allocation lengths and arbitrary pointer validity are outside the model's checks.

### Median filter

- Radius: 0–8. At each pixel, collect the centered `(2 * radius + 1)^2` samples using reflect-101 borders on both axes. Repeated reflected samples count separately.
- Select the sorted sample at zero-based index `sample_count / 2`; the histogram must give exactly this value. Radius zero copies visible pixels.
- Preserve existing validation: invalid checked arguments return `InvalidArgument`, signal completion, and leave output unchanged.

Reflect-101 reflects without repeating the edge pixel. Repeat reflection as needed for windows larger than the image; a one-pixel axis always maps to its only sample:

```text
coordinate:  -4 -3 -2 -1 | 0 1 2 3 | 4 5 6 7
sample:       c  d  c  b | a b c d | c b a b
```

### Sobel filter

Apply these kernels using the same reflect-101 borders:

```text
Gx                 Gy
-1  0  1           -1 -2 -1
-2  0  2            0  0  0
-1  0  1            1  2  1

reference = clamp(gain * sqrt(Gx * Gx + Gy * Gy) / 4.0, 0.0, 255.0)
```

The kernels and division by four are fixed. Gain scales the output, ranges from 0.0 to 1.0, and defaults to 1.0.

The supplied [C reference](src/sobel_reference.c), declared in [include/sobel_reference.h](include/sobel_reference.h), accepts input stride in bytes and writes `width * height` packed `double` samples, clipped but unrounded, without row padding. The device writes 8-bit pixels to the existing output buffer.

### Register-bank extension

Reuse the existing input, output, and image-descriptor registers. Add two read/write registers while preserving existing offsets and command/status values:

| Offset | Register | Initial value | Meaning |
| --- | --- | ---: | --- |
| `0x30` | `FilterSelect` | 0 | 0 = median, 1 = Sobel. Other values are invalid. |
| `0x38` | `EdgeGain` | 0 | Represents real-valued gain in [0.0, 1.0] using an unsigned integer code. Choose and document the mapping, precision, and valid code range. Code 0 represents zero gain. |

## Submission

- Source changes for each task, easy to review separately; exclude build products. Update `README` and `REGISTER_MAP.md` for the new operation, gain encoding, parameter rules, and usage.
- Host/device analysis, explanations of both bug fixes, and the task 2 image/regression with its expected output and before/after results.
- Runnable sample covering Sobel filter run with quality metrics result.
