#include "i2s_2_fft.h"

void i2s_2_fft(hls::stream<stream_i2s> &in,
               hls::stream<stream_audio> &outL,
               hls::stream<stream_audio> &outR, uint16_t &partition_size)
{
#pragma HLS INTERFACE axis port = in
#pragma HLS INTERFACE axis port = outL
#pragma HLS INTERFACE axis port = outR
#pragma HLS INTERFACE s_axilite port = partition_size
#pragma HLS INTERFACE ap_ctrl_none port = return

    static ap_uint<16> fft_cnt1 = 0;
#pragma HLS RESET variable = fft_cnt1
    static ap_uint<16> fft_cnt2 = 0;
#pragma HLS RESET variable = fft_cnt2

    while (!in.empty())
    {
#pragma HLS LOOP_TRIPCOUNT 1

        stream_i2s s = in.read();
        stream_audio o;

        ap_uint<2> chn = s.id;
        ap_int<24> real_data = s.data(27, 4);

        o.data(23, 0) = real_data;
        o.data(47, 24) = ap_int<24>(0);
        o.user = s.user;
        o.id = s.id;
        o.keep = -1;
        o.strb = -1;

        if (chn == 0)
        {
            o.last = (fft_cnt1 == partition_size - 1) ? 1 : 0;
            outL.write(o);
            fft_cnt1++;
            if (fft_cnt1 == partition_size)
                fft_cnt1 = 0;
        }
        else if (chn == 1)
        {
            o.last = (fft_cnt2 == partition_size - 1) ? 1 : 0;
            outR.write(o);
            fft_cnt2++;
            if (fft_cnt2 == partition_size)
                fft_cnt2 = 0;
        }
    }
}
