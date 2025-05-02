#include "CodecHardwareInterface.h"
#include <stdlib.h>

void CodecHardwareInterface::setPeripherals(DMAStreamType* DMAStreamInstance,
                                            SAI_Block_TypeDef* SAIInstance,
                                            volatile uint32_t* DMAInstanceISR,
                                            uint32_t DMAStreamIndex) {
	this->DMAStreamInstance = DMAStreamInstance;
	this->DMAInstanceISR = DMAInstanceISR;
	this->SAIInstance = SAIInstance;
	this->DMAStreamIndex = DMAStreamIndex;
}

uint16_t CodecHardwareInterface::getDmaRemainingCount(void) {
#ifdef STM32F723xx
	return DMAStreamInstance->NDTR;
#else
	return ((DMAStreamInstance->CBR1 & DMA_CBR1_BNDT) + 3) / 4;
#endif
}

bool CodecHardwareInterface::isDMAIsrFlagSet(bool insertWaitStates) {
	if(insertWaitStates) {
		// Do a dummy read from the SAI peripheral
		(void) SAIInstance->SR;
	}
	uint32_t ISR = *DMAInstanceISR;

#ifdef STM32F723xx
	return (ISR & ((DMA_FLAG_HTIF0_4 | DMA_FLAG_TCIF0_4) << DMAStreamIndex)) != RESET;
#else
	return (ISR & (DMA_FLAG_HT | DMA_FLAG_TC)) != 0;
#endif
}
