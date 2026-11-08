#ifndef FFT_2_I2S_H
#define FFT_2_I2S_H

#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <cstdint>

typedef ap_axis<32, 2, 4, 1> stream_i2s;
typedef ap_axis<48, 2, 4, 1> stream_audio;

#define NUM_CHANNELS 2;

void fft_2_i2s(hls::stream<stream_audio> &inL,
               hls::stream<stream_audio> &inR,
               hls::stream<stream_i2s> &out);

#endif
