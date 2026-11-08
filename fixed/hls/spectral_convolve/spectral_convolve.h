#ifndef SPECTRAL_CONVOLVE_H
#define SPECTRAL_CONVOLVE_H

#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <cstdint>

typedef ap_axis<80, 2, 4, 1> stream_fft_wide;

// Component bit widths
#define FFT_OUTPUT_WIDTH 35
#define IFFT_INPUT_WIDTH 34

#define MAX_FFT_SIZE 1024
#define MAX_NUM_PARTITIONS 8

#define SCALING_FACTOR 34

void spectral_convolve(hls::stream<stream_fft_wide> &inX,
                       hls::stream<stream_fft_wide> &inH,
                       hls::stream<stream_fft_wide> &out,
                       uint16_t &fft_size,
                       uint16_t &num_partitions);

#endif
