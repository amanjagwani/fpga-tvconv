#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <cstdint>

typedef ap_axis<80, 2, 4, 1> stream_ifft_out;

typedef ap_axis<48, 2, 4, 1> stream_audio;

#define IFFT_OUTPUT_WIDTH 34

#define MAX_PARTITION_SIZE 512

#define DEFAULT_PARTITION_SIZE 512
#define DEFAULT_FFT_SIZE 1024
#define DEFAULT_OUTPUT_GAIN 0

void overlap_add(hls::stream<stream_ifft_out> &in,
                 hls::stream<stream_audio> &out,
                 uint16_t &fft_size,
                 uint16_t &output_gain);
