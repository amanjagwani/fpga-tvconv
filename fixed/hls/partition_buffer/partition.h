#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <cstdint>

typedef ap_axis<48, 2, 4, 1> stream_fft;

#define MAX_PARTITION_SIZE 512

#define DEFAULT_PARTITION_SIZE 512
#define DEFAULT_FFT_SIZE 1024

void partition_buffer(hls::stream<stream_fft> &in,
                      hls::stream<stream_fft> &out,
                      uint16_t &partition_size);
