#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <cstdint>

typedef ap_axis<80, 2, 4, 1> stream_fft_in;

#define FFT_OUTPUT_WIDTH 35
#define MAX_FFT_SIZE 1024

#define DEFAULT_FFT_SIZE 1024

void fft_mirror_replace(hls::stream<stream_fft_in> &in,
                        hls::stream<stream_fft_in> &out,
                        uint16_t &fft_size);
