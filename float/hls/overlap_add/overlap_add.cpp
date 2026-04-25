#include "overlap_add.h"

union fp_int
{
    float f;
    uint32_t u;
};

void overlap_add(hls::stream<stream_fft_float> &in,
                 hls::stream<stream_audio> &out,
                 uint16_t &fft_size,
                 uint16_t &output_gain)
{
#pragma HLS INTERFACE axis port = in
#pragma HLS INTERFACE axis port = out
#pragma HLS INTERFACE s_axilite port = fft_size
#pragma HLS INTERFACE s_axilite port = output_gain
#pragma HLS INTERFACE ap_ctrl_none port = return

    static float overlap_buffer[MAX_PARTITION_SIZE] = {0};
#pragma HLS BIND_STORAGE variable = overlap_buffer type = ram_2p impl = bram

    ap_int<24> out_buf[MAX_PARTITION_SIZE];
#pragma HLS BIND_STORAGE variable = out_buf type = ram_1p impl = bram

    ap_uint<16> partition_size = fft_size >> 1; // P = N/2

    float scale = (fft_size > 0) ? (1.0f / (float)fft_size) : 0.0f;
    fp_int atten;
    atten.u = (output_gain < 127) ? ((127 - (uint32_t)output_gain) << 23) : 0;
    scale *= atten.f;

absorb:
    for (ap_uint<16> i = 0; i < fft_size; i++)
    {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = 1024
#pragma HLS PIPELINE II = 1
        stream_fft_float in_sample = in.read();

        fp_int conv;
        conv.u = (uint32_t)in_sample.data(31, 0);
        float in_real = conv.f;

        if (i < partition_size)
        {
            float sum = in_real + overlap_buffer[i];

            float scaled = sum * scale;

            int32_t sample_int;
            if (scaled > 8388607.0f)
                sample_int = 8388607;
            else if (scaled < -8388608.0f)
                sample_int = -8388608;
            else
                sample_int = (int32_t)scaled;

            out_buf[i] = (ap_int<24>)sample_int;
        }
        else
        {
            overlap_buffer[i - partition_size] = in_real;
        }
    }

emit:
    for (ap_uint<16> i = 0; i < partition_size; i++)
    {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = 512
#pragma HLS PIPELINE II = 1
        stream_audio o;
        o.data = 0;
        o.data(23, 0) = out_buf[i];
        o.data(47, 24) = 0;
        o.keep = -1;
        o.strb = -1;
        o.user = 0;
        o.id = 0;
        o.dest = 0;
        o.last = (i == partition_size - 1) ? ap_uint<1>(1) : ap_uint<1>(0);
        out.write(o);
    }
}
