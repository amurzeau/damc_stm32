#pragma once
#include "CodecDmaPosition.h"
#include <stddef.h>

class CodecHardwareInterface {
public:
	virtual ~CodecHardwareInterface() {}

	virtual void start(void* inBuffer, void* outBuffer, size_t size_bytes) = 0;
	virtual void setMicBias(bool enable) = 0;

	CodecDmaPosition* getDmaIn() { return &dmaIn; }
	CodecDmaPosition* getDmaOut() { return &dmaOut; }

protected:
	void setDmaIn(DMAStreamType* DMAStreamInstance,
	              SAI_Block_TypeDef* SAIInstance,
	              volatile uint32_t* DMAInstanceISR,
	              uint32_t DMAStreamIndex) {
		dmaIn.setPeripherals(DMAStreamInstance, SAIInstance, DMAInstanceISR, DMAStreamIndex);
	};
	void setDmaOut(DMAStreamType* DMAStreamInstance,
	               SAI_Block_TypeDef* SAIInstance,
	               volatile uint32_t* DMAInstanceISR,
	               uint32_t DMAStreamIndex) {
		dmaOut.setPeripherals(DMAStreamInstance, SAIInstance, DMAInstanceISR, DMAStreamIndex);
	}

private:
	CodecDmaPosition dmaIn;
	CodecDmaPosition dmaOut;
};
