#include "TimeMeasure.h"
#include "AudioCApi.h"
#include <main.h>
#include <string.h>

/**
 * CPU measures "weirdness":
 *
 * USB Interrupt cpu usage per 1ms can jump to higher values sometimes.
 * This is because as the USB clock is not synchronous to the audio clock, sometimes there will be 2 USB audio transfer
 * in one audio period when USB clock is faster. This will lead to temporary higher CPU usage in USB interrupt per audio
 * period.
 *
 * At high CPU usage near 100%, audio cpu usage per 1ms might be > 100%.
 * This is because when handling the DMA interrupt,
 * the HAL_DMA_IRQHandler might first handle the HT (half transfer complete) flag
 * then the TC flag is is while processing audio before the DMA handler continues and it will also see the TC flag set
 * within the same handler call. This means there will be 2 audio processing within the same IRQ and within the same
 * begin/end measure of AudioInterrupt.
 */

#ifdef STM32F723xx
#include <stm32f7xx.h>
#include <stm32f7xx_hal_gpio.h>

static GPIO_TypeDef* const DEBUG_GPIO_PORT[] = {
    [TMI_UsbInterrupt] = STMOD_UART4_RXD_s_GPIO_Port,
    [TMI_AudioProcessing] = STMOD_TIM2_CH1_2_ETR_GPIO_Port,
    [TMI_OtherIRQ] = STMOD_UART4_RXD_GPIO_Port,
    [TMI_MainLoop] = STMOD_UART4_TXD_GPIO_Port,
};

static const uint32_t DEBUG_GPIO_PIN[] = {
    [TMI_UsbInterrupt] = STMOD_UART4_RXD_s_Pin,
    [TMI_AudioProcessing] = STMOD_TIM2_CH1_2_ETR_Pin,
    [TMI_OtherIRQ] = STMOD_UART4_RXD_Pin,
    [TMI_MainLoop] = STMOD_UART4_TXD_Pin,
};
#elif defined(STM32N657xx)
#include <stm32n6xx.h>
#include <stm32n6xx_hal_gpio.h>

static GPIO_TypeDef* const DEBUG_GPIO_PORT[] = {
    [TMI_UsbInterrupt] = DEBUG_1_GPIO_Port,
    [TMI_AudioProcessing] = DEBUG_2_GPIO_Port,
    [TMI_OtherIRQ] = DEBUG_3_GPIO_Port,
    [TMI_MainLoop] = DEBUG_4_GPIO_Port,
    [TMI_TinyDenoiser] = DEBUG_5_GPIO_Port,
};

static const uint32_t DEBUG_GPIO_PIN[] = {
    [TMI_UsbInterrupt] = DEBUG_1_Pin,
    [TMI_AudioProcessing] = DEBUG_2_Pin,
    [TMI_OtherIRQ] = DEBUG_3_Pin,
    [TMI_MainLoop] = DEBUG_4_Pin,
    [TMI_TinyDenoiser] = DEBUG_5_Pin,
};
#endif

TimeMeasure TimeMeasure::timeMeasure[TMI_NUMBER];

static TimeMeasure* stackRunningTasks[16];
static int32_t stackRunningTasksIndex = -1;

TimeMeasure::TimeMeasure()
    : index(0),
      current_measure_sum(0),
      time_sum_between_reset(0),
      time_sum(0),
      time_sum_per_loop(0),
      time_sum_previous_loop(0),
      time_max(0),
      begin_time(0),
      isMeasuring(false) {
	memset(otherTimeMeasureState, 0, sizeof(otherTimeMeasureState));
	for(size_t i = 0; i < TMI_NUMBER; i++) {
		if(this == &timeMeasure[i]) {
			index = i;
			break;
		}
	}
}

void TimeMeasure::beginMeasure() {
	__disable_irq();
	uint32_t current_time = TIM2->CNT;
	HAL_GPIO_WritePin(DEBUG_GPIO_PORT[index], DEBUG_GPIO_PIN[index], GPIO_PIN_SET);

	int32_t previousStackIndex = stackRunningTasksIndex++;
	stackRunningTasks[previousStackIndex + 1] = this;

	// Pause running measure
	if(previousStackIndex >= 0) {
		stackRunningTasks[previousStackIndex]->updateMeasureAndStop(current_time);
	}

	begin_time = current_time;
	current_measure_sum = 0;
	__enable_irq();

	isMeasuring = true;
}

void TimeMeasure::endMeasure() {
	isMeasuring = false;

	__disable_irq();
	HAL_GPIO_WritePin(DEBUG_GPIO_PORT[index], DEBUG_GPIO_PIN[index], GPIO_PIN_RESET);

	int32_t stackIndex = --stackRunningTasksIndex;

	uint32_t current_time = TIM2->CNT;
	updateMeasureAndStop(current_time);

	// Restart paused measure
	if(stackIndex >= 0)
		stackRunningTasks[stackIndex]->begin_time = current_time;

	time_sum += current_measure_sum;
	time_sum_per_loop += current_measure_sum;
	__enable_irq();
}

bool TimeMeasure::updateMeasureAndStop(uint32_t current_time) {
	uint32_t time_measured = current_time - begin_time;
	current_measure_sum += time_measured;

	return true;
}

void TimeMeasure::endOfProcessingLoop(uint32_t loopDurationUs) {
	uint32_t time;

	__disable_irq();
	time = time_sum_per_loop;
	time_sum_per_loop = 0;
	__enable_irq();

	time = time * 1000 / loopDurationUs;

	time_sum_previous_loop = time;

	if(time > time_max)
		time_max = time;
}

uint32_t TimeMeasure::getCurrent() {
	return TIM2->CNT;
}

static uint32_t atomicReadReset(uint32_t* variable, uint32_t* current_time) {
	uint32_t value;

	__disable_irq();
	value = *variable;
	*variable = 0;
	*current_time = TIM2->CNT;
	__enable_irq();

	return value;
}

uint32_t TimeMeasure::getCumulatedTimeUsAndReset() {
	uint32_t current_time;
	uint32_t measure = atomicReadReset(&time_sum, &current_time);

	float elapsed_time = current_time - time_sum_between_reset;
	time_sum_between_reset = current_time;
	return measure * 1000000.0f / elapsed_time;
}

uint32_t TimeMeasure::getMaxUsagePerLoop1000AndReset() {
	uint32_t current_time;
	uint32_t measure = atomicReadReset(&time_max, &current_time);

	return measure;
}

uint32_t TimeMeasure::getMaxUsagePerLoop1000() {
	uint32_t value;

	__disable_irq();
	value = time_sum_previous_loop;
	time_sum_previous_loop = 0;
	__enable_irq();

	return value;
}

uint32_t TimeMeasure::getOnGoingDuration() {
	if(!isMeasuring) {
		return 0;
	}

	uint32_t result;

	__disable_irq();
	result = TIM2->CNT - begin_time + current_measure_sum;
	__enable_irq();

	return result;
}
