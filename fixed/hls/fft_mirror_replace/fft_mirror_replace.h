#ifndef FFT_MIRROR_REPLACE_H
#define FFT_MIRROR_REPLACE_H

#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <cstdint>

typedef ap_axis<80, 2, 4, 1> stream_fft_wide;

#define FFT_OUTPUT_WIDTH 35
#define MAX_FFT_SIZE 1024

void fft_mirror_replace(hls::stream<stream_fft_wide> &in,
                        hls::stream<stream_fft_wide> &out,
                        uint16_t &fft_size);

#endif
