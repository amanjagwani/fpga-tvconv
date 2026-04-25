#include "i2s_2_fft.h"

void i2s_2_fft(hls::stream<stream_i2s> &in,
               hls::stream<stream_fft> &outL,
               hls::stream<stream_fft> &outR, uint16_t &fft_size)
{
#pragma HLS INTERFACE axis port = in
#pragma HLS INTERFACE axis port = outL
#pragma HLS INTERFACE axis port = outR
#pragma HLS INTERFACE s_axilite port = fft_size
#pragma HLS INTERFACE ap_ctrl_none port = return

    static ap_uint<16> fft_cnt1 = 0;
    static ap_uint<16> fft_cnt2 = 0;

    while (!in.empty())
    {
#pragma HLS LOOP_TRIPCOUNT 1

        stream_i2s in_sample = in.read();
        stream_fft fft_in_sample;

        ap_uint<2> chn = in_sample.id;
        ap_int<24> real_data = in_sample.data(27, 4);

        fft_in_sample.data(23, 0) = real_data;
        fft_in_sample.data(47, 24) = ap_int<24>(0);
        fft_in_sample.user = in_sample.user;
        fft_in_sample.id = in_sample.id;
        fft_in_sample.keep = -1;
        fft_in_sample.strb = -1;

        if (chn == 0)
        {
            fft_in_sample.last = (fft_cnt1 == fft_size - 1) ? 1 : 0;
            outL.write(fft_in_sample);
            fft_cnt1++;
            if (fft_cnt1 == fft_size)
                fft_cnt1 = 0;
        }
        else if (chn == 1)
        {
            fft_in_sample.last = (fft_cnt2 == fft_size - 1) ? 1 : 0;
            outR.write(fft_in_sample);
            fft_cnt2++;
            if (fft_cnt2 == fft_size)
                fft_cnt2 = 0;
        }
    }
}
