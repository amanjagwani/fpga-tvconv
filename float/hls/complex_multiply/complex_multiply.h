#ifndef COMPLEX_MULTIPLY_H
#define COMPLEX_MULTIPLY_H

#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <cstdint>

typedef ap_axis<64, 2, 4, 1> stream_fft_float;

void complex_multiply(hls::stream<stream_fft_float> &inX,
                      hls::stream<stream_fft_float> &inH,
                      hls::stream<stream_fft_float> &out,
                      uint16_t &fft_size);

#endif
