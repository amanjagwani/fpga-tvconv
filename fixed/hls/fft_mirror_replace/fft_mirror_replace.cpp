#include "fft_mirror_replace.h"

void fft_mirror_replace(hls::stream<stream_fft_wide> &in,
                        hls::stream<stream_fft_wide> &out,
                        uint16_t &fft_size)
{
#pragma HLS INTERFACE axis port = in
#pragma HLS INTERFACE axis port = out
#pragma HLS INTERFACE s_axilite port = fft_size
#pragma HLS INTERFACE ap_ctrl_none port = return

    static ap_int<FFT_OUTPUT_WIDTH> buf_real[MAX_FFT_SIZE];
    static ap_int<FFT_OUTPUT_WIDTH> buf_imag[MAX_FFT_SIZE];
#pragma HLS BIND_STORAGE variable = buf_real type = ram_2p impl = bram
#pragma HLS BIND_STORAGE variable = buf_imag type = ram_2p impl = bram

    ap_uint<16> N = fft_size;
    ap_uint<16> No2 = fft_size >> 1;

collect:
    for (ap_uint<16> i = 0; i < N; i++)
    {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = 1024
#pragma HLS PIPELINE II = 1
        stream_fft_wide s = in.read();
        buf_real[i] = s.data(FFT_OUTPUT_WIDTH - 1, 0);
        buf_imag[i] = s.data(40 + FFT_OUTPUT_WIDTH - 1, 40);
    }

emit:
    for (ap_uint<16> k = 0; k < N; k++)
    {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = 1024
#pragma HLS PIPELINE II = 1

        ap_int<FFT_OUTPUT_WIDTH> out_real, out_imag;

        if (k == 0 || k == No2)
        {
            out_real = buf_real[k];
            out_imag = buf_imag[k];
        }
        else if (k < No2)
        {
            ap_uint<16> mirror = N - k;
            out_real = buf_real[mirror];
            out_imag = -buf_imag[mirror];
        }
        else
        {
            out_real = buf_real[k];
            out_imag = buf_imag[k];
        }

        stream_fft_wide o;
        o.data = 0;
        o.data(FFT_OUTPUT_WIDTH - 1, 0) = out_real;
        o.data(40 + FFT_OUTPUT_WIDTH - 1, 40) = out_imag;
        o.keep = -1;
        o.strb = -1;
        o.user = 0;
        o.id = 0;
        o.dest = 0;
        o.last = (k == N - 1) ? ap_uint<1>(1) : ap_uint<1>(0);
        out.write(o);
    }
}
