#include "audio.h"
#include "xparameters.h"
#include "xi2s_2_fft.h"
#include "xspectral_convolve_ddr.h"
#include "xoverlap_add.h"
#include "xpartition_buffer.h"
#include "xfft_mirror_replace.h"
#include "xil_cache.h"
#include "xil_io.h"
#include <math.h>
#include <string.h>
#include <stdint.h>
#include "xuartps.h"

#include "xi2stx.h"
#include "xi2srx.h"
#ifndef SR
#define SR 48000
#endif
#define I2S_FS 48

constexpr uint32_t FFT_SIZE = 1024;
constexpr uint32_t PARTITION_SIZE = 512;
constexpr float OUTPUT_GAIN = 0.03125f;
constexpr uint32_t NUM_PARTITIONS = 1024;
constexpr uint32_t MAX_NUM_PARTITIONS = 1024;
constexpr uint32_t FDL_DEPTH = MAX_NUM_PARTITIONS * FFT_SIZE;

#define I2S_2_FFT_BASEADDR XPAR_I2S_2_FFT_0_BASEADDR
#define SPECTRAL_CONVOLVE_DDR_BASEADDR XPAR_SPECTRAL_CONVOLVE_DDR_0_BASEADDR
#define OVERLAP_ADD_BASEADDR XPAR_OVERLAP_ADD_0_BASEADDR
#define PARTITION_BUFFER_0_BASEADDR XPAR_PARTITION_BUFFER_0_BASEADDR
#define PARTITION_BUFFER_1_BASEADDR XPAR_PARTITION_BUFFER_1_BASEADDR
#define FFT_MIRROR_REPLACE_0_BASEADDR XPAR_FFT_MIRROR_REPLACE_0_BASEADDR
#define FFT_MIRROR_REPLACE_1_BASEADDR XPAR_FFT_MIRROR_REPLACE_1_BASEADDR

static inline u32 f2u(float v)
{
	union
	{
		float f;
		u32 u;
	} c;
	c.f = v;
	return c.u;
}

static uint64_t fdl_x[FDL_DEPTH] __attribute__((aligned(4096)));
static uint64_t fdl_h[FDL_DEPTH] __attribute__((aligned(4096)));

int main()
{
	Xil_DCacheFlushRange((UINTPTR)fdl_x, sizeof(fdl_x));
	Xil_DCacheFlushRange((UINTPTR)fdl_h, sizeof(fdl_h));

	XI2s_2_fft i2s2FftInstance;
	int status = XI2s_2_fft_Initialize(&i2s2FftInstance, I2S_2_FFT_BASEADDR);
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	XSpectral_convolve_ddr spectralConvolveInstance;
	status = XSpectral_convolve_ddr_Initialize(&spectralConvolveInstance, SPECTRAL_CONVOLVE_DDR_BASEADDR);
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	XOverlap_add overlapAddInstance;
	status = XOverlap_add_Initialize(&overlapAddInstance, OVERLAP_ADD_BASEADDR);
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	XPartition_buffer partitionBuffer0;
	status = XPartition_buffer_Initialize(&partitionBuffer0, PARTITION_BUFFER_0_BASEADDR);
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	XPartition_buffer partitionBuffer1;
	status = XPartition_buffer_Initialize(&partitionBuffer1, PARTITION_BUFFER_1_BASEADDR);
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	XFft_mirror_replace mirrorReplace0;
	status = XFft_mirror_replace_Initialize(&mirrorReplace0, FFT_MIRROR_REPLACE_0_BASEADDR);
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	XFft_mirror_replace mirrorReplace1;
	status = XFft_mirror_replace_Initialize(&mirrorReplace1, FFT_MIRROR_REPLACE_1_BASEADDR);
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	XSpectral_convolve_ddr_Set_fdl_x(&spectralConvolveInstance, (u64)(UINTPTR)fdl_x);
	XSpectral_convolve_ddr_Set_fdl_h(&spectralConvolveInstance, (u64)(UINTPTR)fdl_h);
	XSpectral_convolve_ddr_Set_fft_size(&spectralConvolveInstance, FFT_SIZE);
	XSpectral_convolve_ddr_Set_num_partitions(&spectralConvolveInstance, NUM_PARTITIONS);
	XI2s_2_fft_Set_partition_size(&i2s2FftInstance, PARTITION_SIZE);
	XOverlap_add_Set_fft_size(&overlapAddInstance, FFT_SIZE);
	XOverlap_add_Set_output_gain(&overlapAddInstance, f2u(OUTPUT_GAIN));
	XPartition_buffer_Set_partition_size(&partitionBuffer0, PARTITION_SIZE);
	XPartition_buffer_Set_partition_size(&partitionBuffer1, PARTITION_SIZE);
	XFft_mirror_replace_Set_fft_size(&mirrorReplace0, FFT_SIZE);
	XFft_mirror_replace_Set_fft_size(&mirrorReplace1, FFT_SIZE);

	XI2s_Tx I2sInstance;
	XI2s_Rx I2sRxInstance;
	XI2srx_Config *RxConfig = XI2s_Rx_LookupConfig(XPAR_I2S_RECEIVER_0_BASEADDR);
	status = XI2s_Rx_CfgInitialize(&I2sRxInstance, RxConfig, RxConfig->BaseAddress);
	XI2s_Rx_SetSclkOutDiv(&I2sRxInstance, 256 * I2S_FS, I2S_FS);
	XI2s_Rx_SetChMux(&I2sRxInstance, XI2S_RX_CHID0, XI2S_RX_CHMUX_XI2S_01);
	XI2s_Rx_Enable(&I2sRxInstance, TRUE);

	XI2stx_Config *TxConfig = XI2s_Tx_LookupConfig(XPAR_I2S_TRANSMITTER_0_BASEADDR);
	status = XI2s_Tx_CfgInitialize(&I2sInstance, TxConfig, TxConfig->BaseAddress);
	XI2s_Tx_SetChMux(&I2sInstance, XI2S_TX_CHID0, XI2S_TX_CHMUX_AXIS_01);
	XI2s_Tx_Enable(&I2sInstance, TRUE);

	IicConfig(XPAR_XIICPS_0_BASEADDR);
	AudioPllConfig();

	while (1)
	{
	}

	return 0;
}
