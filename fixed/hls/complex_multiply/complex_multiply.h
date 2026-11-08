#ifndef COMPLEX_MULTIPLY_H
#define COMPLEX_MULTIPLY_H

#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <cstdint>

typedef ap_axis<80, 2, 4, 1> stream_fft_wide;

#define FFT_OUTPUT_WIDTH 35
#define IFFT_INPUT_WIDTH 34

void complex_multiply(hls::stream<stream_fft_wide> &inX,
                      hls::stream<stream_fft_wide> &inH,
                      hls::stream<stream_fft_wide> &out,
                      uint16_t &fft_size,
                      uint16_t &scaling_factor);

#endif
