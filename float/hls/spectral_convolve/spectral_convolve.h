#ifndef SPECTRAL_CONVOLVE_H
#define SPECTRAL_CONVOLVE_H

#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <cstdint>

typedef ap_axis<64, 2, 4, 1> stream_fft_float;

#define MAX_FFT_SIZE 1024
#define MAX_NUM_PARTITIONS 8

#define DEFAULT_FFT_SIZE 1024
#define DEFAULT_NUM_PARTITIONS 1

void spectral_convolve(hls::stream<stream_fft_float> &inX,
                       hls::stream<stream_fft_float> &inH,
                       hls::stream<stream_fft_float> &out,
                       uint16_t &fft_size,
                       uint16_t &num_partitions);

#endif
