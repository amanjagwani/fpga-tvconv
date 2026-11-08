#include "fft_mirror_replace.h"

union fp_int
{
    float f;
    uint32_t u;
};

// bins 1..N/2-1 = conj of the upper half, which has less rounding noise for real input (PG109)
void fft_mirror_replace(hls::stream<stream_fft_float> &in,
                        hls::stream<stream_fft_float> &out,
                        uint16_t &fft_size)
{
#pragma HLS INTERFACE axis port = in
#pragma HLS INTERFACE axis port = out
#pragma HLS INTERFACE s_axilite port = fft_size
#pragma HLS INTERFACE ap_ctrl_none port = return

    static float buf_real[MAX_FFT_SIZE];
    static float buf_imag[MAX_FFT_SIZE];
#pragma HLS BIND_STORAGE variable = buf_real type = ram_2p impl = bram
#pragma HLS BIND_STORAGE variable = buf_imag type = ram_2p impl = bram

    ap_uint<16> N = fft_size;
    ap_uint<16> No2 = fft_size >> 1;

collect:
    for (ap_uint<16> i = 0; i < N; i++)
    {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = 1024
#pragma HLS PIPELINE II = 1
        stream_fft_float s = in.read();
        fp_int re, im;
        re.u = (uint32_t)s.data(31, 0);
        im.u = (uint32_t)s.data(63, 32);
        buf_real[i] = re.f;
        buf_imag[i] = im.f;
    }

emit:
    for (ap_uint<16> k = 0; k < N; k++)
    {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = 1024
#pragma HLS PIPELINE II = 1

        float out_re, out_im;

        if (k == 0 || k == No2)
        {
            out_re = buf_real[k];
            out_im = buf_imag[k];
        }
        else if (k < No2)
        {
            ap_uint<16> mirror = N - k;
            out_re = buf_real[mirror];
            out_im = -buf_imag[mirror];
        }
        else
        {
            out_re = buf_real[k];
            out_im = buf_imag[k];
        }

        fp_int re_out, im_out;
        re_out.f = out_re;
        im_out.f = out_im;

        stream_fft_float o;
        o.data(31, 0) = (ap_uint<32>)re_out.u;
        o.data(63, 32) = (ap_uint<32>)im_out.u;
        o.keep = -1;
        o.strb = -1;
        o.user = 0;
        o.id = 0;
        o.dest = 0;
        o.last = (k == N - 1) ? ap_uint<1>(1) : ap_uint<1>(0);
        out.write(o);
    }
}
