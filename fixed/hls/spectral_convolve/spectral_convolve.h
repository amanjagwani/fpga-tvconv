#ifndef SPECTRAL_CONVOLVE_H
#define SPECTRAL_CONVOLVE_H

#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <cstdint>

typedef ap_axis<80, 2, 4, 1> stream_fft_in;

typedef ap_axis<80, 2, 4, 1> stream_fft_out;

// Component bit widths
#define FFT_OUTPUT_WIDTH 35
#define IFFT_INPUT_WIDTH 34

#define MAX_FFT_SIZE 1024
#define MAX_NUM_PARTITIONS 8

#define DEFAULT_FFT_SIZE 1024
#define DEFAULT_NUM_PARTITIONS 1

#define DEFAULT_SCALING_FACTOR 0

void spectral_convolve(hls::stream<stream_fft_in> &inX,
                       hls::stream<stream_fft_in> &inH,
                       hls::stream<stream_fft_out> &out,
                       uint16_t &fft_size,
                       uint16_t &num_partitions,
                       uint16_t &scaling_factor);

#endif
