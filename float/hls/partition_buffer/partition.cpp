#include "partition.h"

union fp_int
{
    float f;
    uint32_t u;
};

void partition_buffer(hls::stream<stream_i2s_fft> &in,
                      hls::stream<stream_fft_float> &out,
                      uint16_t &partition_size)
{
#pragma HLS INTERFACE axis port = in
#pragma HLS INTERFACE axis port = out
#pragma HLS INTERFACE s_axilite port = partition_size
#pragma HLS INTERFACE ap_ctrl_none port = return

    static ap_int<24> buffer[MAX_PARTITION_SIZE];
#pragma HLS BIND_STORAGE variable = buffer type = ram_1p impl = bram

    ap_uint<16> fft_size = partition_size << 1; // N = 2P

    for (ap_uint<16> i = 0; i < partition_size; i++)
    {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = 512
#pragma HLS PIPELINE II = 1
        stream_i2s_fft s = in.read();
        buffer[i] = s.data(23, 0);
    }

    for (ap_uint<16> i = 0; i < fft_size; i++)
    {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = 1024
#pragma HLS PIPELINE II = 1
        ap_int<24> sample = (i < partition_size) ? buffer[i] : ap_int<24>(0);

        fp_int conv;
        conv.f = (float)((int)sample);

        stream_fft_float o;
        o.data(31, 0) = (ap_uint<32>)conv.u;
        o.data(63, 32) = (ap_uint<32>)0;
        o.keep = -1;
        o.strb = -1;
        o.user = 0;
        o.id = 0;
        o.dest = 0;
        o.last = (i == fft_size - 1) ? ap_uint<1>(1) : ap_uint<1>(0);
        out.write(o);
    }
}
