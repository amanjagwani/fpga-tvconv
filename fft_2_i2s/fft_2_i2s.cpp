#include "fft_2_i2s.h"

void fft_2_i2s(hls::stream<stream_audio> &inL,
               hls::stream<stream_audio> &inR,
               hls::stream<stream_i2s> &out)
{
#pragma HLS INTERFACE axis port = inL
#pragma HLS INTERFACE axis port = inR
#pragma HLS INTERFACE axis port = out
#pragma HLS INTERFACE ap_ctrl_none port = return

    while (!inL.empty() && !inR.empty())
    {
#pragma HLS LOOP_TRIPCOUNT 1

        stream_audio s_l = inL.read();
        stream_audio s_r = inR.read();

        ap_int<24> data_l = s_l.data(23, 0);
        ap_int<24> data_r = s_r.data(23, 0);

        stream_i2s o_l, o_r;

        o_l.data(27, 4) = data_l;
        o_l.data(3, 0) = 1;
        o_l.user = s_l.user;
        o_l.id = 0;
        o_l.keep = s_l.keep;
        o_l.strb = s_l.strb;
        o_l.last = 0;

        o_r.data(27, 4) = data_r;
        o_r.data(3, 0) = 2;
        o_r.user = s_r.user;
        o_r.id = 1;
        o_r.keep = s_r.keep;
        o_r.strb = s_r.strb;
        o_r.last = 0;

        out.write(o_l);
        out.write(o_r);
    }
}
