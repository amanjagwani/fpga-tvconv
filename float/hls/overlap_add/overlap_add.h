#ifndef OVERLAP_ADD_H
#define OVERLAP_ADD_H

#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <cstdint>

typedef ap_axis<64, 2, 4, 1> stream_fft_float;

typedef ap_axis<48, 2, 4, 1> stream_audio;

#define MAX_PARTITION_SIZE 512

void overlap_add(hls::stream<stream_fft_float> &in,
                 hls::stream<stream_audio> &out,
                 uint16_t &fft_size,
                 volatile float &output_gain);

#endif
