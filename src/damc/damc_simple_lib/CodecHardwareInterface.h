#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef STM32F723xx
#include <stm32f7xx.h>
#include <stm32f7xx_hal_dma.h>
#include <stm32f7xx_hal_sai.h>

typedef DMA_Stream_TypeDef DMAStreamType;
#elif defined(STM32N657xx)
#include <stm32n6xx.h>
#include <stm32n6xx_hal_dma.h>
#include <stm32n6xx_hal_sai.h>

typedef DMA_Channel_TypeDef DMAStreamType;
#endif

class CodecHardwareInterface {
public:
	virtual ~CodecHardwareInterface() {}

	virtual void start(void* inBuffer, void* outBuffer, size_t size_bytes) = 0;
	virtual void setMicBias(bool enable) = 0;

	uint16_t getDmaRemainingCount();
	bool isDMAIsrFlagSet(bool insertWaitStates);

protected:
	void setPeripherals(DMAStreamType* DMAStreamInstance,
	                    SAI_Block_TypeDef* SAIInstance,
	                    volatile uint32_t* DMAInstanceISR,
	                    uint32_t DMAStreamIndex);

private:
	DMAStreamType* DMAStreamInstance;
	SAI_Block_TypeDef* SAIInstance;
	volatile uint32_t* DMAInstanceISR;
	uint32_t DMAStreamIndex;
};
