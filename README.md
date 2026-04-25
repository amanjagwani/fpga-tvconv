# FPGA Time-Varying Convolution

Real-time convolution of two live audio signals on a Zybo Z7-020 (Xilinx Zynq-7000) FPGA board. Two channels of live audio input are convolved with each other in the frequency domain using multi-partition overlap-add FFT convolution with Frequency-Domain Delay Lines.

All signal processing runs in programmable logic (PL) using custom HLS IPs connected via AXI4-Stream. The ARM PS only configures I2S and HLS IP parameters at startup.

## Hardware

- **Board**: Digilent Zybo Z7-020 (XC7Z020-1CLG400C)
- **Audio codec**: SSM2603 (I2S, 48 kHz, 24-bit)
- **PL clock**: 122.88 MHz
- **Vivado/Vitis**: 2022.2

## Signal Flow

```
I2S RX (interleaved L/R 48kHz)
  → i2s_2_fft (deinterleave, pad imag=0, TLAST every P samples)
    → L: partition_buffer → FIFO → FFT_L → fft_mirror_replace ──→ spectral_convolve → FIFO → IFFT
    → R: partition_buffer → FIFO → FFT_R → fft_mirror_replace ──↗         ↓
                                                                    overlap_add
                                                                        ↓
                                                              fft_2_i2s (mono→stereo)
                                                                        ↓
                                                                     I2S TX
```

Both L and R channels received via I2S are treated as independent live audio signals to be convolved with each other.

## Key Parameters

| Parameter                    | Value                           |
| ---------------------------- | ------------------------------- |
| FFT size (N)                 | variable                        |
| Partition size (P)           | N/2                             |
| Sample rate                  | 48 kHz                          |
| Audio bit depth              | 24-bit signed                   |
| Partitions (K)               | 1–8 (configurable via AXI-Lite) |
| Effective convolution length | K × P samples                   |

## Two Builds

### Float

### Fixed

Unscaled forward FFTs (S_fwd=1) with scaled IFFT (S_inv=N) for exact 1/N normalization. 80-bit wide datapaths (35-bit components).

## HLS IPs

| IP                   | Description                                                                                                                                                                                                                                                  |
| -------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `i2s_2_fft`          | Deinterleaves I2S L/R, reformats 32-bit I2S → 48-bit FFT (24-bit real, imag=0), generates TLAST every P samples                                                                                                                                              |
| `fft_2_i2s`          | Extracts real part from 48-bit FFT format, reinterleaves L/R → 32-bit I2S                                                                                                                                                                                    |
| `partition_buffer`   | Collects P samples at audio rate, outputs 2P samples (P data + P zeros) at PL rate for zero-padded FFT                                                                                                                                                       |
| `spectral_convolve`  | Multi-partition frequency dimain delay line convolution. Maintains K spectral frames for both inputs in circular BRAM buffers. Each frame: ingest, multiply-accumulate K products with index reversal, emit                                                  |
| `overlap_add`        | Overlap-add reconstruction. First P samples overlap-added with previous tail, last P saved.                                                                                                                                                                  |
| `fft_mirror_replace` | Replaces lower-half FFT bins with conjugates of upper half, forcing perfect conjugate symmetry for real-valued input. Placed after each forward FFT. Based on Xilinx PG109 recommendation (see below). Most audible in the fixed build; negligible in float. |
| `complex_multiply`   | single-partition convolution                                                                                                                                                                                                                                 |

All IPs use `ap_ctrl_none` (free-running), AXI4-Stream interfaces, and AXI-Lite parameters configurable from PS at startup.

### Why fft_mirror_replace

From [Xilinx PG109 — FFT LogiCORE IP Product Guide v9.1](https://www.xilinx.com/support/documents/ip_documentation/xfft/v9_1/pg109-xfft.pdf), section on real-valued input data:

> _The FFT core accepts complex data samples, but can perform a transform on real-valued data by setting all imaginary input samples to zero. Due to the finite wordlength effects described previously, noise is introduced during the transform, resulting in the output data not being perfectly symmetric. The DIT and DIF FFT algorithms have different noise effects due to the different calculation order. The asymmetry between the two halves of the result is more noticeable at larger point sizes. In addition, the noise is more prominent in the lower frequency bins. Therefore, Xilinx recommends that the upper half (N/2+1 to N points) of the output data is used when performing a real-valued FFT._

`fft_mirror_replace` implements this recommendation.

## Repository Structure

```
fpga-tvconv/
  i2s_2_fft/                  # I2S → FFT format converter (shared)
  fft_2_i2s/                  # FFT format → I2S converter (shared)
  float/
    hls/                      # HLS IP source
      partition_buffer/
      complex_multiply/       # Legacy single-partition
      spectral_convolve/      # Multi-partition FDL (recommended)
      overlap_add/
      fft_mirror_replace/
    vivado/                   # Vivado project (block design, constraints, XSAs)
    vitis/src/                # PS application source
  fixed/
    hls/                      # Same IPs, 80-bit fixed-point variant
      partition_buffer/
      complex_multiply/
      spectral_convolve/
      overlap_add/
      fft_mirror_replace/
    vivado/                   # Vivado project (unscaled build)
    vitis/src/                # PS application source
```

## Building

### HLS IPs

1. Open Vitis HLS 2022.2
2. Create a project for each IP, set the top function, add source
3. Run C Synthesis → Export RTL (IP Catalog format)

### Vivado

1. Open the `.xpr` project in `float/vivado/` or `fixed/vivado/`
2. Refresh IP Catalog (picks up exported HLS IPs)
3. Generate Bitstream
4. Export Hardware (include bitstream) → produces `.xsa`

### Vitis PS Application

The `vitis/src/` folders contain the PS application source files. To create a Vitis project:

1. Open Vitis 2022.2
2. **File → New → Application Project**
3. Select **Create a new platform from hardware (XSA)** → browse to the `.xsa` from the Vivado export
4. Name the application, select **Empty Application (C++)** template
5. Copy `main.cpp`, `audio.h`, and `audio.c` from the corresponding `vitis/src/` folder into the project's `src/` directory
6. Build the project
7. **Run As → Launch on Hardware** to program the FPGA and start the application
