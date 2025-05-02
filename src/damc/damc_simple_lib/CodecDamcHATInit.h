#pragma once
#include <CodecHardwareInterface.h>
#include <stdint.h>

#ifdef STM32F723xx
#include <stm32f7xx.h>
#include <stm32f7xx_hal_dma.h>
#include <stm32f7xx_hal_i2c.h>
#include <stm32f7xx_hal_sai.h>
#include <stm32f7xx_hal_tim.h>
#elif defined(STM32N657xx)
#include <stm32n6xx.h>
#include <stm32n6xx_hal_dma.h>
#include <stm32n6xx_hal_i2c.h>
#include <stm32n6xx_hal_sai.h>
#include <stm32n6xx_hal_tim.h>
#endif

extern "C" {
void DMA2_Stream3_IRQHandler(void);
void DMA2_Stream5_IRQHandler(void);
}

class CodecDamcHATInit : public CodecHardwareInterface {
public:
	void init();
	bool isAvailable();

	void start(void* inBuffer, void* outBuffer, size_t size_bytes) override;

	void setMicBias(bool enable) override;

	void init_audio();

	void init_sai();
	void init_codec();

	void startTxDMA(void* buffer, size_t nframes);
	void startRxDMA(void* buffer, size_t nframes);

private:
	void writeI2c(uint8_t address, uint8_t value);
	uint8_t readI2c(uint8_t address);
	void setReset(bool value);
	void setTpaEn(bool value);

	friend void DMA2_Stream3_IRQHandler(void);
	friend void DMA2_Stream5_IRQHandler(void);

private:
	I2C_HandleTypeDef hi2c;
	TIM_HandleTypeDef htim;
	SAI_HandleTypeDef hsai_tx;
	SAI_HandleTypeDef hsai_rx;
	DMA_HandleTypeDef hdma_tx;
	DMA_HandleTypeDef hdma_rx;
};
