#include "spectral_convolve_ddr.h"

union fp_int
{
    float f;
    uint32_t u;
};

void spectral_convolve_ddr(
    hls::stream<stream_fft_float> &inX,
    hls::stream<stream_fft_float> &inH,
    hls::stream<stream_fft_float> &out,
    ap_uint<64> *fdl_x,
    ap_uint<64> *fdl_h,
    uint16_t &fft_size,
    uint16_t &num_partitions)
{
#pragma HLS INTERFACE axis port = inX
#pragma HLS INTERFACE axis port = inH
#pragma HLS INTERFACE axis port = out
#pragma HLS INTERFACE m_axi port = fdl_x offset = slave bundle = gmem0 depth = FDL_DEPTH latency = 30 max_read_burst_length = 256 max_write_burst_length = 256 num_read_outstanding = 4 num_write_outstanding = 4
#pragma HLS INTERFACE m_axi port = fdl_h offset = slave bundle = gmem1 depth = FDL_DEPTH latency = 30 max_read_burst_length = 256 max_write_burst_length = 256 num_read_outstanding = 4 num_write_outstanding = 4
#pragma HLS INTERFACE s_axilite port = fft_size
#pragma HLS INTERFACE s_axilite port = num_partitions
#ifndef COSIM_CTRL_HS
#pragma HLS INTERFACE ap_ctrl_none port = return
#endif

    static ap_uint<16> write_ptr = 0;
#pragma HLS RESET variable = write_ptr

    float acc_real[MAX_FFT_SIZE];
    float acc_imag[MAX_FFT_SIZE];
#pragma HLS BIND_STORAGE variable = acc_real type = ram_2p impl = bram
#pragma HLS BIND_STORAGE variable = acc_imag type = ram_2p impl = bram

    ap_uint<16> K = num_partitions;
    ap_uint<16> wp = write_ptr % K;
    ap_uint<32> base_w = (ap_uint<32>)wp * MAX_FFT_SIZE;

collect:
    for (ap_uint<16> i = 0; i < fft_size; i++)
    {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = 1024
#pragma HLS PIPELINE II = 1
        stream_fft_float s_x = inX.read();
        stream_fft_float s_h = inH.read();
        fdl_x[base_w + i] = s_x.data;
        fdl_h[base_w + i] = s_h.data;
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
#pragma HLS LOOP_TRIPCOUNT min = 1 max = 1024
        ap_uint<16> x_idx = (p <= wp) ? (ap_uint<16>)(wp - p) : (ap_uint<16>)(wp + K - p);
        ap_uint<32> base_x = (ap_uint<32>)x_idx * MAX_FFT_SIZE;
        ap_uint<32> base_h = (ap_uint<32>)p * MAX_FFT_SIZE;

    mac_inner:
        for (ap_uint<16> i = 0; i < fft_size; i++)
        {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = 1024
#pragma HLS PIPELINE II = 1
#pragma HLS LOOP_FLATTEN off
#pragma HLS DEPENDENCE variable = acc_real type = inter false
#pragma HLS DEPENDENCE variable = acc_imag type = inter false
            ap_uint<64> xv = fdl_x[base_x + i];
            ap_uint<64> hv = fdl_h[base_h + i];

            fp_int xr, xi, hr, hi;
            xr.u = (uint32_t)xv(31, 0);
            xi.u = (uint32_t)xv(63, 32);
            hr.u = (uint32_t)hv(31, 0);
            hi.u = (uint32_t)hv(63, 32);

            float prod_real = xr.f * hr.f - xi.f * hi.f;
            float prod_imag = xr.f * hi.f + xi.f * hr.f;

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

    write_ptr = (wp + 1 == K) ? (ap_uint<16>)0 : (ap_uint<16>)(wp + 1);
}
