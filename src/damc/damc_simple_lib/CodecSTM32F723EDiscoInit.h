#pragma once
#include "CodecHardwareInterface.h"
#include <stddef.h>

extern "C" {
void DMA2_Stream3_IRQHandler(void);
void DMA2_Stream5_IRQHandler(void);
}

class CodecSTM32F723EDiscoInit : public CodecHardwareInterface {
public:
	void start(void* inBuffer, void* outBuffer, size_t size_bytes) override;

	void setMicBias(bool enable) override {}

	void init_clock();
	void init_sai();
	void init_codec();

	void startTxDMA(void* buffer, size_t size_bytes);
	void startRxDMA(void* buffer, size_t size_bytes);

private:
};