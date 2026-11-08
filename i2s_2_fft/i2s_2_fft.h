#ifndef I2S_2_FFT_H
#define I2S_2_FFT_H

#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <cstdint>

typedef ap_axis<32, 2, 4, 1> stream_i2s;
typedef ap_axis<48, 2, 4, 1> stream_audio;

#define PARTITION_SIZE 256
#define FFT_SIZE 512

#define NUM_CHANNELS 2;

void i2s_2_fft(hls::stream<stream_i2s> &in,
               hls::stream<stream_audio> &outL,
               hls::stream<stream_audio> &outR, uint16_t &partition_size);

#endif
