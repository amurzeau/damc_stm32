# TinyDenoiser

The denoiser uses the model from https://github.com/GreenWaves-Technologies/tiny_denoiser_v2

The folder contains the model as C code.

The model was converted to C using onnx2c (https://github.com/kraiskil/onnx2c) and
then adjusted to optimize its execution on Cortex-M55:

- Remove useless operations like Reshape (the actual memory don't need to be changed, a cast is sufficient).
- Replace LSTM implementations:
  - Maximize hardware cache prefetcher for maximum SRAM bandwidth (the original C code from onnx2c was bandwidth-limited).
  - Maximize SIMD performance by using FP16 functions from CMSIS-DSP.
