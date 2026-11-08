#include "spectral_convolve.h"

void spectral_convolve(
    hls::stream<stream_fft_wide> &inX,
    hls::stream<stream_fft_wide> &inH,
    hls::stream<stream_fft_wide> &out,
    uint16_t &fft_size,
    uint16_t &num_partitions)
{
#pragma HLS INTERFACE axis port = inX
#pragma HLS INTERFACE axis port = inH
#pragma HLS INTERFACE axis port = out
#pragma HLS INTERFACE s_axilite port = fft_size
#pragma HLS INTERFACE s_axilite port = num_partitions
#pragma HLS INTERFACE ap_ctrl_none port = return

    static ap_int<FFT_OUTPUT_WIDTH> fdl_x_real[MAX_NUM_PARTITIONS][MAX_FFT_SIZE];
    static ap_int<FFT_OUTPUT_WIDTH> fdl_x_imag[MAX_NUM_PARTITIONS][MAX_FFT_SIZE];
    static ap_int<FFT_OUTPUT_WIDTH> fdl_h_real[MAX_NUM_PARTITIONS][MAX_FFT_SIZE];
    static ap_int<FFT_OUTPUT_WIDTH> fdl_h_imag[MAX_NUM_PARTITIONS][MAX_FFT_SIZE];
#pragma HLS BIND_STORAGE variable = fdl_x_real type = ram_2p impl = bram
#pragma HLS BIND_STORAGE variable = fdl_x_imag type = ram_2p impl = bram
#pragma HLS BIND_STORAGE variable = fdl_h_real type = ram_2p impl = bram
#pragma HLS BIND_STORAGE variable = fdl_h_imag type = ram_2p impl = bram

    static ap_uint<16> write_ptr = 0;
#pragma HLS RESET variable = write_ptr

    ap_int<74> acc_real[MAX_FFT_SIZE];
    ap_int<74> acc_imag[MAX_FFT_SIZE];
#pragma HLS BIND_STORAGE variable = acc_real type = ram_2p impl = bram
#pragma HLS BIND_STORAGE variable = acc_imag type = ram_2p impl = bram

    ap_uint<16> K = num_partitions;

collect:
    for (ap_uint<16> i = 0; i < fft_size; i++)
    {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = 1024
#pragma HLS PIPELINE II = 1
        stream_fft_wide s_x = inX.read();
        stream_fft_wide s_h = inH.read();

        fdl_x_real[write_ptr][i] = s_x.data(FFT_OUTPUT_WIDTH - 1, 0);
        fdl_x_imag[write_ptr][i] = s_x.data(40 + FFT_OUTPUT_WIDTH - 1, 40);
        fdl_h_real[write_ptr][i] = s_h.data(FFT_OUTPUT_WIDTH - 1, 0);
        fdl_h_imag[write_ptr][i] = s_h.data(40 + FFT_OUTPUT_WIDTH - 1, 40);
    }

zero_acc:
    for (ap_uint<16> i = 0; i < fft_size; i++)
    {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = 1024
#pragma HLS PIPELINE II = 1
        acc_real[i] = 0;
        acc_imag[i] = 0;
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
            ap_int<FFT_OUTPUT_WIDTH> xr = fdl_x_real[x_idx][i];
            ap_int<FFT_OUTPUT_WIDTH> xi = fdl_x_imag[x_idx][i];
            ap_int<FFT_OUTPUT_WIDTH> hr = fdl_h_real[p][i];   // fixed slot (Lazzarini 2020, eq. 13)
            ap_int<FFT_OUTPUT_WIDTH> hi = fdl_h_imag[p][i];

            ap_int<71> prod_real = (ap_int<71>)(xr * hr) - (ap_int<71>)(xi * hi);
            ap_int<71> prod_imag = (ap_int<71>)(xr * hi) + (ap_int<71>)(xi * hr);

            acc_real[i] = acc_real[i] + (ap_int<74>)prod_real;
            acc_imag[i] = acc_imag[i] + (ap_int<74>)prod_imag;
        }
    }

    const ap_int<74> MAX_VAL = (ap_int<74>(1) << (IFFT_INPUT_WIDTH - 1)) - 1;
    const ap_int<74> MIN_VAL = -(ap_int<74>(1) << (IFFT_INPUT_WIDTH - 1));

emit:
    for (ap_uint<16> i = 0; i < fft_size; i++)
    {
#pragma HLS LOOP_TRIPCOUNT min = 1 max = 1024
#pragma HLS PIPELINE II = 1
        ap_int<74> real_shifted = acc_real[i];
        ap_int<74> imag_shifted = acc_imag[i];

        if (SCALING_FACTOR > 0)
        {
            ap_int<74> rounding_bit = (ap_int<74>(1) << (SCALING_FACTOR - 1));
            if (real_shifted >= 0)
                real_shifted += rounding_bit;
            else
                real_shifted -= rounding_bit;
            if (imag_shifted >= 0)
                imag_shifted += rounding_bit;
            else
                imag_shifted -= rounding_bit;
        }
        real_shifted >>= SCALING_FACTOR;
        imag_shifted >>= SCALING_FACTOR;

        ap_int<IFFT_INPUT_WIDTH> out_real_sat, out_imag_sat;

        if (real_shifted > MAX_VAL)
            out_real_sat = (ap_int<IFFT_INPUT_WIDTH>)MAX_VAL;
        else if (real_shifted < MIN_VAL)
            out_real_sat = (ap_int<IFFT_INPUT_WIDTH>)MIN_VAL;
        else
            out_real_sat = (ap_int<IFFT_INPUT_WIDTH>)real_shifted;

        if (imag_shifted > MAX_VAL)
            out_imag_sat = (ap_int<IFFT_INPUT_WIDTH>)MAX_VAL;
        else if (imag_shifted < MIN_VAL)
            out_imag_sat = (ap_int<IFFT_INPUT_WIDTH>)MIN_VAL;
        else
            out_imag_sat = (ap_int<IFFT_INPUT_WIDTH>)imag_shifted;

        stream_fft_wide o;
        o.data = 0;
        o.data(IFFT_INPUT_WIDTH - 1, 0) = out_real_sat;
        o.data(40 + IFFT_INPUT_WIDTH - 1, 40) = out_imag_sat;
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
