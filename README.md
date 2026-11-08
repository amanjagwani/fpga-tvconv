# fpga-tvconv

Real-time time-varying convolution of two live audio signals on a Zynq-7000. The left and right inputs of the codec are convolved with each other in the frequency domain (multi-partition overlap-add with frequency-domain delay lines, uniform partitions of P samples). All signal processing runs in the PL as Vitis HLS IPs chained with AXI4-Stream; the ARM only sets IP parameters over AXI-Lite. Target: Digilent Zybo Z7-20, Vivado/Vitis 2025.2.

```
I2S RX → i2s_2_fft ─┬→ partition_buffer → FFT → fft_mirror_replace ─┐
                    └→ partition_buffer → FFT → fft_mirror_replace ─┴→ spectral_convolve → IFFT → overlap_add → fft_2_i2s → I2S TX
```

| Build        | Datapath                                                                                   | Filter length (P = 512)          |
| ------------ | ------------------------------------------------------------------------------------------ | -------------------------------- |
| `float/`     | float FFTs and IPs                                                                         | K ≤ 8 partitions in BRAM (85 ms) |
| `fixed/`     | unscaled forward FFTs, scaled IFFT                                                         | K ≤ 8 partitions in BRAM (85 ms) |
| `float_ddr/` | float, delay lines in DDR (`spectral_convolve_ddr`, uses the `float/hls` IPs for the rest) | K ≤ 1024 partitions (10.9 s)     |

| Path                 | Contents                                                                                                |
| -------------------- | ------------------------------------------------------------------------------------------------------- |
| `i2s_2_fft/`         | I2S → two 48-bit sample streams, TLAST every P samples (HLS)                                            |
| `fft_2_i2s/`         | output stream → interleaved I2S (HLS)                                                                   |
| `<build>/hls/<ip>/`  | HLS IPs of each build (`complex_multiply` is the old single-partition version, not in any block design) |
| `<build>/vivado/`    | `build.tcl`, block design Tcl (`design_1.tcl`) and constraints                                          |
| `<build>/vitis/src/` | PS application                                                                                          |

## Build

1. HLS IPs: in every folder that has an `hls_config.cfg` (`i2s_2_fft/`, `fft_2_i2s/`, `<build>/hls/*/`), with `<top>` = its `syn.top`:
   ```
   v++ -c --mode hls --config hls_config.cfg --work_dir <top>
   vitis-run --mode hls --package --config hls_config.cfg --work_dir <top>
   ```
   `float_ddr` also needs the `float/hls` IPs.
2. Bitstream and XSA (no board files needed; the PS7 setup is in `design_1.tcl`):
   ```
   vivado -mode batch -source float/vivado/build.tcl -tclargs tvconv_float_v1.xsa
   ```
   The project goes to `<build>/vivado/vivado_proj/`, the XSA to `<build>/vivado/xsa/`, with timing and utilisation reports in `vivado_proj/`.
3. Vitis: create a standalone platform (`ps7_cortexa9_0`) from the XSA, add an empty application with the build's `vitis/src/` files, build, and run over JTAG.
4. The app sets N = 1024, P = 512, the number of partitions (`NUM_PARTITIONS`) and the output gain at start-up; edit the constants at the top of `main.cpp`.

## Board

Zybo Z7-20 (XC7Z020-1CLG400C), SSM2603 codec: line in (L and R are the two signals) → headphone/line out, 48 kHz, 24-bit. PL clock 122.88 MHz from the 125 MHz board clock. Pins are in `<build>/vivado/ZYBO_Master_Constraints.xdc`.
