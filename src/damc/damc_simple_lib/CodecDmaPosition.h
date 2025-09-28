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

class CodecDmaPosition {
public:
	void setPeripherals(DMAStreamType* DMAStreamInstance,
	                    SAI_Block_TypeDef* SAIInstance,
	                    volatile uint32_t* DMAInstanceISR,
	                    uint32_t DMAStreamIndex);

	uint16_t getDmaRemainingCount();
	bool isDMAIsrFlagSet(bool insertWaitStates);

private:
	DMAStreamType* DMAStreamInstance;
	SAI_Block_TypeDef* SAIInstance;
	volatile uint32_t* DMAInstanceISR;
	uint32_t DMAStreamIndex;
};
