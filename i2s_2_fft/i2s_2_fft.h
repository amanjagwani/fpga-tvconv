#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <cstdint>

typedef ap_axis<32, 2, 4, 1> stream_i2s;
typedef ap_axis<48, 2, 4, 1> stream_fft;

#define PARTITION_SIZE 256
#define FFT_SIZE 512

#define NUM_CHANNELS 2;

void i2s_2_fft(hls::stream<stream_i2s> &in,
               hls::stream<stream_fft> &outL,
               hls::stream<stream_fft> &outR, uint16_t &fft_size);
