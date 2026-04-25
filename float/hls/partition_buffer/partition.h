#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <cstdint>

typedef ap_axis<48, 2, 4, 1> stream_i2s_fft;

typedef ap_axis<64, 2, 4, 1> stream_fft_float;

#define MAX_PARTITION_SIZE 512

#define DEFAULT_PARTITION_SIZE 512
#define DEFAULT_FFT_SIZE 1024

void partition_buffer(hls::stream<stream_i2s_fft> &in,
                      hls::stream<stream_fft_float> &out,
                      uint16_t &partition_size);
