#include "CodecSTM32F723EDiscoInit.h"
#include <stdlib.h>

#ifdef STM32F723xx
#include <stm32f723e_discovery.h>
#include <stm32f723e_discovery_audio.h>

void CodecSTM32F723EDiscoInit::start(void* inBuffer, void* outBuffer, size_t size_bytes) {
	init_sai();

	init_codec();

	setPeripherals(haudio_out_sai.hdmatx->Instance,
	               haudio_out_sai.Instance,
	               (volatile uint32_t*) haudio_out_sai.hdmatx->StreamBaseAddress,
	               haudio_out_sai.hdmatx->StreamIndex);

	startRxDMA(inBuffer, size_bytes);
	startTxDMA(outBuffer, size_bytes);
}

void CodecSTM32F723EDiscoInit::init_sai() {
	/* SAI2 */

	/* Initialize the haudio_out_sai Instance parameter */
	haudio_in_sai.Instance = AUDIO_IN_SAIx;
	BSP_AUDIO_IN_MspInit(&haudio_in_sai, NULL);

	haudio_out_sai.Instance = AUDIO_OUT_SAIx;
	BSP_AUDIO_OUT_MspInit(&haudio_out_sai, NULL);

	/* Disable SAI peripheral to allow access to SAI internal registers */
	__HAL_SAI_DISABLE(&haudio_out_sai);

	/* Configure SAI_Block_x
	LSBFirst: Disabled
	DataSize: 16 */
	haudio_out_sai.Init.AudioFrequency = 48000;
	haudio_out_sai.Init.AudioMode = SAI_MODEMASTER_TX;
	haudio_out_sai.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
	haudio_out_sai.Init.Protocol = SAI_FREE_PROTOCOL;
	haudio_out_sai.Init.DataSize = SAI_DATASIZE_32;
	haudio_out_sai.Init.FirstBit = SAI_FIRSTBIT_MSB;
	haudio_out_sai.Init.ClockStrobing = SAI_CLOCKSTROBING_FALLINGEDGE;
	haudio_out_sai.Init.Synchro = SAI_ASYNCHRONOUS;
	haudio_out_sai.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
	haudio_out_sai.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
	haudio_out_sai.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF;
	haudio_out_sai.Init.Mckdiv = 0;
	haudio_out_sai.Init.MonoStereoMode = SAI_STEREOMODE;
	haudio_out_sai.Init.CompandingMode = SAI_NOCOMPANDING;
	haudio_out_sai.Init.TriState = SAI_OUTPUT_NOTRELEASED;

	/* Configure SAI_Block_x Frame
	Frame Length: 64
	Frame active Length: 32
	FS Definition: Start frame + Channel Side identification
	FS Polarity: FS active Low
	FS Offset: FS asserted one bit before the first bit of slot 0 */
	haudio_out_sai.FrameInit.FrameLength = 128;
	haudio_out_sai.FrameInit.ActiveFrameLength = 64;
	haudio_out_sai.FrameInit.FSDefinition = SAI_FS_CHANNEL_IDENTIFICATION;
	haudio_out_sai.FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;
	haudio_out_sai.FrameInit.FSOffset = SAI_FS_BEFOREFIRSTBIT;
	haudio_in_sai.FrameInit = haudio_out_sai.FrameInit;

	/* Configure SAI Block_x Slot
	Slot First Bit Offset: 0
	Slot Size  : 16
	Slot Number: 4
	Slot Active: All slot actives */
	haudio_out_sai.SlotInit.FirstBitOffset = 0;
	haudio_out_sai.SlotInit.SlotSize = SAI_SLOTSIZE_DATASIZE;
	haudio_out_sai.SlotInit.SlotNumber = 4;
	haudio_out_sai.SlotInit.SlotActive = CODEC_AUDIOFRAME_SLOT_02;

	HAL_SAI_Init(&haudio_out_sai);

	/* Configure SAI Block_B */
	__HAL_SAI_DISABLE(&haudio_in_sai);

	haudio_in_sai.Init = haudio_out_sai.Init;
	haudio_in_sai.FrameInit = haudio_out_sai.FrameInit;
	haudio_in_sai.SlotInit = haudio_out_sai.SlotInit;
	haudio_in_sai.Init.AudioMode = SAI_MODESLAVE_RX;
	haudio_in_sai.Init.Synchro = SAI_SYNCHRONOUS;
	haudio_in_sai.Init.ClockStrobing = SAI_CLOCKSTROBING_RISINGEDGE;

	HAL_SAI_Init(&haudio_in_sai);

	/* Enable SAI peripheral */
	__HAL_SAI_ENABLE(&haudio_in_sai);
	__HAL_SAI_ENABLE(&haudio_out_sai);
}

void CodecSTM32F723EDiscoInit::init_codec() {
	uint32_t deviceid = 0x00;
	deviceid = wm8994_drv.ReadID(AUDIO_I2C_ADDRESS);

	if((deviceid) == WM8994_ID) {
		/* Reset the Codec Registers */
		wm8994_drv.Reset(AUDIO_I2C_ADDRESS);
		/* Initialize the audio driver structure */
		// 91 gives 0dB HPOUT1L_VOL
		// 80 gives 0dB AIF1ADC1L_VOL
		wm8994_drv.Init(AUDIO_I2C_ADDRESS, INPUT_DEVICE_INPUT_LINE_1 | OUTPUT_DEVICE_HEADPHONE, 91, 80, 48000);
	}
}

void CodecSTM32F723EDiscoInit::startTxDMA(void* buffer, size_t size_bytes) {
	// Unmute
	wm8994_drv.Play(AUDIO_I2C_ADDRESS, nullptr, 0);
	/* Update the Media layer and enable it for play */
	HAL_SAI_Transmit_DMA(&haudio_out_sai, (uint8_t*) buffer, size_bytes / sizeof(uint32_t));
}

void CodecSTM32F723EDiscoInit::startRxDMA(void* buffer, size_t size_bytes) {
	/* Update the Media layer and enable it for play */
	HAL_SAI_Receive_DMA(&haudio_in_sai, (uint8_t*) buffer, size_bytes / sizeof(uint32_t));
}

#elif defined(STM32N657xx)
#include <stm32n6570_discovery.h>
#include <stm32n6570_discovery_audio.h>

void CodecSTM32F723EDiscoInit::start(void* inBuffer, void* outBuffer, size_t size_bytes) {
	init_sai();

	init_codec();

	setPeripherals(haudio_out_sai.hdmatx->Instance, haudio_out_sai.Instance, &haudio_out_sai.hdmatx->Instance->CSR, 0);

	startRxDMA(inBuffer, size_bytes);
	startTxDMA(outBuffer, size_bytes);
}

void CodecSTM32F723EDiscoInit::init_sai() {
	BSP_AUDIO_Init_t inInit = {
	    .Device = AUDIO_IN_DEVICE_ANALOG_MIC,
	    .SampleRate = 48000,
	    .BitsPerSample = 32,
	    .ChannelsNbr = 2,
	    .Volume = 50,
	};

	BSP_AUDIO_Init_t outInit = {
	    .Device = AUDIO_OUT_DEVICE_HEADPHONE,
	    .SampleRate = 48000,
	    .BitsPerSample = 32,
	    .ChannelsNbr = 2,
	    .Volume = 50,
	};

	/* Initialize the haudio_out_sai Instance parameter */
	BSP_AUDIO_IN_Init(0, &inInit);

	BSP_AUDIO_OUT_Init(0, &outInit);
}

void CodecSTM32F723EDiscoInit::init_codec() {}

void CodecSTM32F723EDiscoInit::startTxDMA(void* buffer, size_t size_bytes) {
	BSP_AUDIO_OUT_Play(0, (uint8_t*) buffer, size_bytes);
}

void CodecSTM32F723EDiscoInit::startRxDMA(void* buffer, size_t size_bytes) {
	BSP_AUDIO_IN_Record(0, (uint8_t*) buffer, size_bytes);
}

#endif
