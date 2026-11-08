#include "spectral_convolve.h"

union fp_int
{
    float f;
    uint32_t u;
};

void spectral_convolve(
    hls::stream<stream_fft_float> &inX,
    hls::stream<stream_fft_float> &inH,
    hls::stream<stream_fft_float> &out,
    uint16_t &fft_size,
    uint16_t &num_partitions)
{
#pragma HLS INTERFACE axis port = inX
#pragma HLS INTERFACE axis port = inH
#pragma HLS INTERFACE axis port = out
#pragma HLS INTERFACE s_axilite port = fft_size
#pragma HLS INTERFACE s_axilite port = num_partitions
#pragma HLS INTERFACE ap_ctrl_none port = return

    static float fdl_x_real[MAX_NUM_PARTITIONS][MAX_FFT_SIZE];
    static float fdl_x_imag[MAX_NUM_PARTITIONS][MAX_FFT_SIZE];
    static float fdl_h_real[MAX_NUM_PARTITIONS][MAX_FFT_SIZE];
    static float fdl_h_imag[MAX_NUM_PARTITIONS][MAX_FFT_SIZE];
#pragma HLS BIND_STORAGE variable = fdl_x_real type = ram_2p impl = bram
#pragma HLS BIND_STORAGE variable = fdl_x_imag type = ram_2p impl = bram
#pragma HLS BIND_STORAGE variable = fdl_h_real type = ram_2p impl = bram
#pragma HLS BIND_STORAGE variable = fdl_h_imag type = ram_2p impl = bram

    static ap_uint<16> write_ptr = 0;
#pragma HLS RESET variable = write_ptr

    float acc_real[MAX_FFT_SIZE];
    float acc_imag[MAX_FFT_SIZE];
#pragma HLS BIND_STORAGE variable = acc_real type = ram_2p impl = bram
#pragma HLS BIND_STORAGE variable = acc_imag type = ram_2p impl = bram

    ap_uint<16> K = num_partitions;

collect:
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

        fdl_x_real[write_ptr][i] = xr.f;
        fdl_x_imag[write_ptr][i] = xi.f;
        fdl_h_real[write_ptr][i] = hr.f;
        fdl_h_imag[write_ptr][i] = hi.f;
    }

zero_acc:
    for (ap_uint<16> i = 0; i < fft_size; i++)
    {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = 1024
#pragma HLS PIPELINE II = 1
        acc_real[i] = 0.0f;
        acc_imag[i] = 0.0f;
    }

    for (ap_uint<16> p = 0; p < K; p++)
    {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = 8
        ap_uint<16> x_idx = (write_ptr + K - p) % K;

    mac_inner:
        for (ap_uint<16> i = 0; i < fft_size; i++)
        {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = 1024
#pragma HLS PIPELINE II = 2
            float xr = fdl_x_real[x_idx][i];
            float xi = fdl_x_imag[x_idx][i];
            float hr = fdl_h_real[p][i];
            float hi = fdl_h_imag[p][i];

            float prod_real = xr * hr - xi * hi;
            float prod_imag = xr * hi + xi * hr;

            acc_real[i] = acc_real[i] + prod_real;
            acc_imag[i] = acc_imag[i] + prod_imag;
        }
    }

emit:
    for (ap_uint<16> i = 0; i < fft_size; i++)
    {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = 1024
#pragma HLS PIPELINE II = 1
        fp_int or_out, oi_out;
        or_out.f = acc_real[i];
        oi_out.f = acc_imag[i];

        stream_fft_float o;
        o.data(31, 0) = (ap_uint<32>)or_out.u;
        o.data(63, 32) = (ap_uint<32>)oi_out.u;
        o.last = (i == fft_size - 1) ? 1 : 0;
        o.keep = -1;
        o.strb = -1;
        o.user = 0;
        o.id = 0;
        o.dest = 0;
        out.write(o);
    }

    write_ptr = (write_ptr + 1) % K;
}
