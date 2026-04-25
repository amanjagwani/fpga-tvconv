#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <cstdint>

// Floating-point FFT streams: 32-bit float per component, TDATA = 64 bits
typedef ap_axis<64, 2, 4, 1> stream_fft_float;

#define DEFAULT_FFT_SIZE 1024

void complex_multiply(hls::stream<stream_fft_float> &inX,
                      hls::stream<stream_fft_float> &inH,
                      hls::stream<stream_fft_float> &out,
                      uint16_t &fft_size);
