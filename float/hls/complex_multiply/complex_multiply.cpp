#include "complex_multiply.h"

union fp_int
{
    float f;
    uint32_t u;
};

void complex_multiply(
    hls::stream<stream_fft_float> &inX,
    hls::stream<stream_fft_float> &inH,
    hls::stream<stream_fft_float> &out,
    uint16_t &fft_size)
{
#pragma HLS INTERFACE axis port = inX
#pragma HLS INTERFACE axis port = inH
#pragma HLS INTERFACE axis port = out
#pragma HLS INTERFACE s_axilite port = fft_size
#pragma HLS INTERFACE ap_ctrl_none port = return

    for (ap_uint<16> i = 0; i < fft_size; i++)
    {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = 1024
#pragma HLS PIPELINE II = 1
        stream_fft_float x = inX.read();
        stream_fft_float h = inH.read();

        // Extract float components from 64-bit TDATA
        fp_int xr, xi, hr, hi;
        xr.u = (uint32_t)x.data(31, 0);
        xi.u = (uint32_t)x.data(63, 32);
        hr.u = (uint32_t)h.data(31, 0);
        hi.u = (uint32_t)h.data(63, 32);

        fp_int or_out, oi_out;
        or_out.f = xr.f * hr.f - xi.f * hi.f;
        oi_out.f = xr.f * hi.f + xi.f * hr.f;

        // Pack into 64-bit TDATA
        stream_fft_float y_sample;
        y_sample.data(31, 0) = (ap_uint<32>)or_out.u;
        y_sample.data(63, 32) = (ap_uint<32>)oi_out.u;
        y_sample.last = (i == fft_size - 1) ? 1 : 0;
        y_sample.user = x.user;
        y_sample.id = x.id;
        y_sample.keep = -1;
        y_sample.strb = -1;
        out.write(y_sample);
    }
}
