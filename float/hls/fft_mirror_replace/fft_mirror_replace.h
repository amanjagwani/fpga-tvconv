#ifndef FFT_MIRROR_REPLACE_H
#define FFT_MIRROR_REPLACE_H

#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <cstdint>

typedef ap_axis<64, 2, 4, 1> stream_fft_float;

#define MAX_FFT_SIZE 1024

void fft_mirror_replace(hls::stream<stream_fft_float> &in,
                        hls::stream<stream_fft_float> &out,
                        uint16_t &fft_size);

#endif
