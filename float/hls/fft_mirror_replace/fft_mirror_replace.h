#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <cstdint>

// Floating-point FFT streams: 32-bit float per component, TDATA = 64 bits
typedef ap_axis<64, 2, 4, 1> stream_fft_float;

#define MAX_FFT_SIZE 1024

#define DEFAULT_FFT_SIZE 1024

void fft_mirror_replace(hls::stream<stream_fft_float> &in,
                        hls::stream<stream_fft_float> &out,
                        uint16_t &fft_size);
