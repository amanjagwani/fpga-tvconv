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
        stream_fft_float s_x = inX.read();
        stream_fft_float s_h = inH.read();

        fp_int xr, xi, hr, hi;
        xr.u = (uint32_t)s_x.data(31, 0);
        xi.u = (uint32_t)s_x.data(63, 32);
        hr.u = (uint32_t)s_h.data(31, 0);
        hi.u = (uint32_t)s_h.data(63, 32);

        fp_int or_out, oi_out;
        or_out.f = xr.f * hr.f - xi.f * hi.f;
        oi_out.f = xr.f * hi.f + xi.f * hr.f;

        stream_fft_float o;
        o.data(31, 0) = (ap_uint<32>)or_out.u;
        o.data(63, 32) = (ap_uint<32>)oi_out.u;
        o.last = (i == fft_size - 1) ? 1 : 0;
        o.user = s_x.user;
        o.id = s_x.id;
        o.keep = -1;
        o.strb = -1;
        out.write(o);
    }
}
