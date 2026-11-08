#include "overlap_add.h"

void overlap_add(hls::stream<stream_fft_wide> &in,
                 hls::stream<stream_audio> &out,
                 uint16_t &fft_size,
                 uint16_t &output_gain)
{
#pragma HLS INTERFACE axis port = in
#pragma HLS INTERFACE axis port = out
#pragma HLS INTERFACE s_axilite port = fft_size
#pragma HLS INTERFACE s_axilite port = output_gain
#pragma HLS INTERFACE ap_ctrl_none port = return

    static ap_int<IFFT_OUTPUT_WIDTH> overlap_buffer[MAX_PARTITION_SIZE] = {0};
#pragma HLS BIND_STORAGE variable = overlap_buffer type = ram_2p impl = bram

    ap_int<24> out_buf[MAX_PARTITION_SIZE];
#pragma HLS BIND_STORAGE variable = out_buf type = ram_1p impl = bram

    ap_uint<16> partition_size = fft_size >> 1; // P = N/2

accumulate:
    for (ap_uint<16> i = 0; i < fft_size; i++)
    {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = 1024
#pragma HLS PIPELINE II = 1
        stream_fft_wide s = in.read();
        ap_int<IFFT_OUTPUT_WIDTH> in_real = s.data(IFFT_OUTPUT_WIDTH - 1, 0);

        if (i < partition_size)
        {
            ap_int<IFFT_OUTPUT_WIDTH + 1> sum = (ap_int<IFFT_OUTPUT_WIDTH + 1>)in_real + (ap_int<IFFT_OUTPUT_WIDTH + 1>)overlap_buffer[i];

            ap_int<64> amplified = (ap_int<64>)sum << output_gain;

            if (amplified > 8388607)
                out_buf[i] = 8388607;
            else if (amplified < -8388608)
                out_buf[i] = -8388608;
            else
                out_buf[i] = (ap_int<24>)amplified;
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
