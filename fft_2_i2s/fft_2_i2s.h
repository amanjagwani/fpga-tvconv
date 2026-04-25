#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <cstdint>

typedef ap_axis<32, 2, 4, 1> stream_i2s;
typedef ap_axis<48, 2, 4, 1> stream_fft;

#define NUM_CHANNELS 2;

void fft_2_i2s(hls::stream<stream_fft> &inL,
               hls::stream<stream_fft> &inR,
               hls::stream<stream_i2s> &out,
               uint16_t &fft_size);
