#include "fft_2_i2s.h"

void fft_2_i2s(hls::stream<stream_fft> &inL,
               hls::stream<stream_fft> &inR,
               hls::stream<stream_i2s> &out,
               uint16_t &fft_size)
{
#pragma HLS INTERFACE axis port = inL
#pragma HLS INTERFACE axis port = inR
#pragma HLS INTERFACE axis port = out
#pragma HLS INTERFACE s_axilite port = fft_size
#pragma HLS INTERFACE ap_ctrl_none port = return

    while (!inL.empty() && !inR.empty())
    {
#pragma HLS LOOP_TRIPCOUNT 1

        stream_fft in_sampleL = inL.read();
        stream_fft in_sampleR = inR.read();

        ap_int<24> real_dataL = in_sampleL.data(23, 0);
        ap_int<24> real_dataR = in_sampleR.data(23, 0);

        stream_i2s out_sampleL, out_sampleR;

        out_sampleL.data(27, 4) = real_dataL;
        out_sampleL.data(3, 0) = 1;
        out_sampleL.user = in_sampleL.user;
        out_sampleL.id = 0; // Left channel
        out_sampleL.keep = in_sampleL.keep;
        out_sampleL.strb = in_sampleL.strb;
        out_sampleL.last = 0;

        out_sampleR.data(27, 4) = real_dataR;
        out_sampleR.data(3, 0) = 2;
        out_sampleR.user = in_sampleR.user;
        out_sampleR.id = 1; // Right channel
        out_sampleR.keep = in_sampleR.keep;
        out_sampleR.strb = in_sampleR.strb;
        out_sampleR.last = 0;

        out.write(out_sampleL);
        out.write(out_sampleR);
    }
}
