#ifndef PARTITION_H
#define PARTITION_H

#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <cstdint>

typedef ap_axis<48, 2, 4, 1> stream_audio;

typedef ap_axis<48, 2, 4, 1> stream_fft_fixed;

#define MAX_PARTITION_SIZE 512

void partition_buffer(hls::stream<stream_audio> &in,
                      hls::stream<stream_fft_fixed> &out,
                      uint16_t &partition_size);

#endif
