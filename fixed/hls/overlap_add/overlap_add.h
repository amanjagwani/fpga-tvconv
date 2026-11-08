#ifndef OVERLAP_ADD_H
#define OVERLAP_ADD_H

#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <cstdint>

typedef ap_axis<80, 2, 4, 1> stream_fft_wide;

typedef ap_axis<48, 2, 4, 1> stream_audio;

#define IFFT_OUTPUT_WIDTH 34

#define MAX_PARTITION_SIZE 512

void overlap_add(hls::stream<stream_fft_wide> &in,
                 hls::stream<stream_audio> &out,
                 uint16_t &fft_size,
                 uint16_t &output_gain);

#endif
