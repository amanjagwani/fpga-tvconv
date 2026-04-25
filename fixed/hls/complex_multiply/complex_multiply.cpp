#include "complex_multiply.h"

void complex_multiply(
    hls::stream<stream_fft_in> &inX,
    hls::stream<stream_fft_in> &inH,
    hls::stream<stream_fft_out> &out,
    uint16_t &fft_size,
    uint16_t &scaling_factor)
{
#pragma HLS INTERFACE axis port = inX
#pragma HLS INTERFACE axis port = inH
#pragma HLS INTERFACE axis port = out
#pragma HLS INTERFACE s_axilite port = fft_size
#pragma HLS INTERFACE s_axilite port = scaling_factor
#pragma HLS INTERFACE ap_ctrl_none port = return

    for (ap_uint<16> i = 0; i < fft_size; i++)
    {
#pragma HLS PIPELINE II = 1
        stream_fft_in x = inX.read();
        stream_fft_in h = inH.read();

        // Extract 35-bit signed components from 40-bit padded containers
        // Xilinx FFT packs: real in [39:0], imag in [79:40]
        ap_int<FFT_OUTPUT_WIDTH> x_real = x.data(FFT_OUTPUT_WIDTH - 1, 0);
        ap_int<FFT_OUTPUT_WIDTH> x_imag = x.data(40 + FFT_OUTPUT_WIDTH - 1, 40);
        ap_int<FFT_OUTPUT_WIDTH> h_real = h.data(FFT_OUTPUT_WIDTH - 1, 0);
        ap_int<FFT_OUTPUT_WIDTH> h_imag = h.data(40 + FFT_OUTPUT_WIDTH - 1, 40);

        // Full precision: 35 x 35 = 70 bits, add/sub needs 71
        ap_int<71> out_real_full = (ap_int<71>)(x_real * h_real) - (ap_int<71>)(x_imag * h_imag);
        ap_int<71> out_imag_full = (ap_int<71>)(x_real * h_imag) + (ap_int<71>)(x_imag * h_real);

        if (scaling_factor > 0)
        {
            ap_int<71> rounding_bit = (ap_int<71>(1) << (scaling_factor - 1));
            if (out_real_full >= 0)
                out_real_full += rounding_bit;
            else
                out_real_full -= rounding_bit;
            if (out_imag_full >= 0)
                out_imag_full += rounding_bit;
            else
                out_imag_full -= rounding_bit;
        }

        ap_int<71> out_real_shifted = out_real_full >> scaling_factor;
        ap_int<71> out_imag_shifted = out_imag_full >> scaling_factor;

        // Saturate to 34-bit signed range for IFFT input
        const ap_int<71> MAX_VAL = (ap_int<71>(1) << (IFFT_INPUT_WIDTH - 1)) - 1;
        const ap_int<71> MIN_VAL = -(ap_int<71>(1) << (IFFT_INPUT_WIDTH - 1));
        ap_int<IFFT_INPUT_WIDTH> out_real_sat;
        ap_int<IFFT_INPUT_WIDTH> out_imag_sat;

        if (out_real_shifted > MAX_VAL)
            out_real_sat = (ap_int<IFFT_INPUT_WIDTH>)MAX_VAL;
        else if (out_real_shifted < MIN_VAL)
            out_real_sat = (ap_int<IFFT_INPUT_WIDTH>)MIN_VAL;
        else
            out_real_sat = (ap_int<IFFT_INPUT_WIDTH>)out_real_shifted;

        if (out_imag_shifted > MAX_VAL)
            out_imag_sat = (ap_int<IFFT_INPUT_WIDTH>)MAX_VAL;
        else if (out_imag_shifted < MIN_VAL)
            out_imag_sat = (ap_int<IFFT_INPUT_WIDTH>)MIN_VAL;
        else
            out_imag_sat = (ap_int<IFFT_INPUT_WIDTH>)out_imag_shifted;

        // Pack into 80-bit TDATA: real in [39:0], imag in [79:40]
        stream_fft_out y_sample;
        y_sample.data = 0;
        y_sample.data(IFFT_INPUT_WIDTH - 1, 0) = out_real_sat;
        y_sample.data(40 + IFFT_INPUT_WIDTH - 1, 40) = out_imag_sat;
        y_sample.last = (i == fft_size - 1) ? 1 : 0;
        y_sample.user = x.user;
        y_sample.id = x.id;
        y_sample.keep = -1;
        y_sample.strb = -1;
        out.write(y_sample);
    }
}
