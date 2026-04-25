#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <cstdint>

// IFFT output: 32-bit float per component, TDATA = 64 bits
typedef ap_axis<64, 2, 4, 1> stream_fft_float;

// Output to fft_2_i2s: 24-bit fixed-point, TDATA = 48 bits
typedef ap_axis<48, 2, 4, 1> stream_audio;

#define MAX_PARTITION_SIZE 512

// Default values
#define DEFAULT_PARTITION_SIZE 512
#define DEFAULT_FFT_SIZE 1024
#define DEFAULT_OUTPUT_GAIN 0

void overlap_add(hls::stream<stream_fft_float> &in,
                 hls::stream<stream_audio> &out,
                 uint16_t &fft_size,
                 uint16_t &output_gain);
