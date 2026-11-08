#ifndef SPECTRAL_CONVOLVE_DDR_H
#define SPECTRAL_CONVOLVE_DDR_H

#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <cstdint>

typedef ap_axis<64, 2, 4, 1> stream_fft_float;

#define MAX_FFT_SIZE 1024
#ifndef MAX_NUM_PARTITIONS
#define MAX_NUM_PARTITIONS 1024

#endif
#define FDL_DEPTH (MAX_NUM_PARTITIONS * MAX_FFT_SIZE)

void spectral_convolve_ddr(hls::stream<stream_fft_float> &inX,
                           hls::stream<stream_fft_float> &inH,
                           hls::stream<stream_fft_float> &out,
                           ap_uint<64> *fdl_x,
                           ap_uint<64> *fdl_h,
                           uint16_t &fft_size,
                           uint16_t &num_partitions);

#endif
