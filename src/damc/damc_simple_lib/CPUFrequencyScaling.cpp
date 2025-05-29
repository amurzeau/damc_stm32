#include "CPUFrequencyScaling.h"
#include "AudioCApi.h"
#include "TimeMeasure.h"
#include <atomic>

static uint32_t roundToLowerPowerOf2(uint32_t target_value) {
	// Round to lower power of two
	uint32_t divider = 1;
	while(divider * 2 <= target_value)
		divider *= 2;

	return divider;
}

static uint32_t clampDivider(uint32_t divider, uint32_t min, uint32_t max, bool power_of_2) {
	if(divider > max) {
		divider = max;
	} else if(divider < min) {
		divider = min;
	}
	if(!power_of_2)
		return divider;
	else
		return roundToLowerPowerOf2(divider);
}

static uint32_t getBitPosition(uint32_t value) {
	uint32_t res = 0;

	if(!value) {
		return 32;
	}

	// Left shift 1 and check each bit
	while((value & 1) == 0) {
		value = value >> 1;
		res++;
	}

	return res;
}

#ifdef STM32F723xx
#include <stm32f7xx_hal.h>

uint32_t getHPREValueFromDivider(uint32_t divider) {
	switch(divider) {
		case 1:
			return 0;
		case 2:
			return 0b1000;
		case 4:
			return 0b1001;
		case 8:
			return 0b1010;
	}

	while(1)
		;
}

uint32_t getPPREValueFromDivider(uint32_t divider) {
	switch(divider) {
		case 1:
			return 0;
		case 2:
			return 0b100;
		case 4:
			return 0b101;
		case 8:
			return 0b110;
		case 16:
			return 0b111;
	}

	while(1)
		;
}

void setRawAHBDivider(uint32_t current_ahb_divider, uint32_t ahb_divider) {
	if(ahb_divider < current_ahb_divider) {
		// Increase flash wait states
		uint32_t flash_latency = (216 / ahb_divider - 1) / 30;
		__HAL_FLASH_SET_LATENCY(flash_latency);
		if(__HAL_FLASH_GET_LATENCY() != flash_latency) {
			// Failed to set the latency ?
			while(1)
				;
		}
	}

	// Compute new register RCC->CFGR value to apply new AHB, APB1 and APB2 dividers
	uint32_t apb1_divider = 8 / ahb_divider;
	uint32_t apb2_divider = 16 / ahb_divider;

	uint32_t rcc_cfgr = (getHPREValueFromDivider(ahb_divider) << RCC_CFGR_HPRE_Pos) |
	                    (getPPREValueFromDivider(apb1_divider) << RCC_CFGR_PPRE1_Pos) |
	                    (getPPREValueFromDivider(apb2_divider) << RCC_CFGR_PPRE2_Pos);

	// Update AHB, APB1 and APB2 dividers
	MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE_Msk | RCC_CFGR_PPRE1_Msk | RCC_CFGR_PPRE2_Msk, rcc_cfgr);

	if((READ_REG(RCC->CFGR) & (RCC_CFGR_HPRE_Msk | RCC_CFGR_PPRE1_Msk | RCC_CFGR_PPRE2_Msk)) != rcc_cfgr) {
		// Failed to set prescalers
		while(1)
			;
	}

	if(ahb_divider > current_ahb_divider) {
		// Reduce flash wait states
		uint32_t flash_latency = (216 / ahb_divider - 1) / 30;
		__HAL_FLASH_SET_LATENCY(flash_latency);
		if(__HAL_FLASH_GET_LATENCY() != flash_latency) {
			// Failed to set the latency ?
			while(1)
				;
		}
	}

	/* Update the SystemCoreClock global variable */
	SystemCoreClock = HAL_RCC_GetSysClockFreq() / ahb_divider;
	HAL_InitTick(TICK_INT_PRIORITY);
}

void CPUFrequencyScaling::setAHBDivider(uint32_t divider) {
	// Max divider: 8 for minimal APB1 frequency of 27Mhz
	if(divider > 4) {
		divider = 4;
	} else if(divider < 1) {
		divider = 1;
	}

	// Round to lower power of two
	uint32_t ahb_divider = 1;
	while(ahb_divider * 2 <= divider)
		ahb_divider *= 2;

	// Check if already at that divider
	if(ahb_divider == current_ahb_divider)
		return;

	setRawAHBDivider(current_ahb_divider, ahb_divider);

	current_ahb_divider = ahb_divider;
	recent_ahb_divider_change = true;
	uv_async_send(&asyncFrequencyChanged);

	// Reset cpu usage stats
	cpu_usage_points = 0;
	max_cpu_usage_ratio_per_thousand = 0;
}

const uint32_t CPUFrequencyScaling::CPU_USAGE_MARGIN_AT_MAX_FREQUENCY = 25;

uint32_t CPUFrequencyScaling::getCpuUsageWithLowerSpeed(uint32_t value) {
	return value * 2;
}

void CPUFrequencyScaling::adjustCpuFreq(CpuFreqAdjustement adjustment) {
	switch(adjustment) {
		case CpuFreqAdjustement::ResetToMax:
			setAHBDivider(1);
			break;
		case CpuFreqAdjustement::IncreaseSpeed:
			setAHBDivider(current_ahb_divider / 2);
			break;
		case CpuFreqAdjustement::DecreaseSpeed:
			setAHBDivider(current_ahb_divider * 2);
			break;
	}
}

#elif defined(STM32N657xx)
#include <stm32n6xx_hal.h>
#include <stm32n6xx_hal_cortex.h>
#include <stm32n6xx_hal_rcc.h>

static void updateTimerPrescaler() {
	uint32_t timer_frequency = HAL_RCC_GetSysClockFreq() / (1UL << LL_RCC_GetTIMPrescaler());

	uint32_t prescaler = (timer_frequency / 1000000) - 1;
	if(prescaler == TIM2->PSC)
		return;

	__disable_irq();
	uint32_t previous_counter = TIM2->CNT;
	TIM2->PSC = prescaler;
	TIM2->EGR |= TIM_EGR_UG;
	(void) TIM2->EGR;
	TIM2->CNT = previous_counter;
	__enable_irq();
}

static uint32_t getAHBDivider() {
	return 1 << (LL_RCC_GetAHBPrescaler() >> RCC_CFGR2_HPRE_Pos);
}

// Max 800Mhz
void CPUFrequencyScaling::setRawCPUDivider(uint32_t divider) {
	divider = clampDivider(divider, 1, 256, false);

	if(divider == LL_RCC_IC1_GetDivider())
		return;

	// Set sys_cpu_ck frequency
	LL_RCC_IC1_SetDivider(divider);

	current_ahb_divider = divider;
	recent_ahb_divider_change = true;

	/* Update the SystemCoreClock global variable */
	SystemCoreClock = HAL_RCC_GetCpuClockFreq();
	HAL_SYSTICK_Config(SystemCoreClock / (1000UL / (uint32_t) uwTickFreq));

	uv_async_send(&asyncFrequencyChanged);
}

// Max 400Mhz
// Updating the AXI clock frequency requires also:
// - Updating AHB frequency so it is capped at 200Mhz
// - Updating the TIM2 prescaler which is used by TimeMeasure
//   - Updating the prescaler requires setting UG bit so the update take place immediately:
//     - Wait the counter to increment, meaning the internal prescaler counter in the timer is near 0 and we don't loose
//       to much time accuracy
//     - Write the new prescaler value
//     - Set UG bit so the new prescaler value is used immediately, but this will reset the counter value
//     - Set the counter back to its previous value after waiting UG bit write is taken into account (by doing a dummy
//       read)
//   - Updating the timer prescaler requires IRQs disabled to avoid an USB IRQ updating TimeMeasure at the wrong time
//   while the timer is reset or counting with a wrong prescaler.
void CPUFrequencyScaling::setRawAXIDivider(uint32_t divider) {
	// Only allow between 2 and 16 and power of 2 to be able to compensate the timer frequency using TIMPRE.
	divider = clampDivider(divider, 2, 16, true);
	uint32_t current_divider = LL_RCC_IC2_GetDivider();

	if(divider == current_divider)
		return;

	uint32_t hpre_value;
	uint32_t timer_frequency = HAL_RCCEx_GetPLL1CLKFreq() / divider / (1UL << LL_RCC_GetTIMPrescaler());

	// AHB max is 200Mhz
	// HPRE: if divider is 2 (AXI >= 400MHz), AHB = AXI / 2 else AHB = AXI
	if(divider == 2) {
		hpre_value = getBitPosition(2);
	} else {
		hpre_value = getBitPosition(1);
	}

	// TIM2: reset every 1s
	// Update the TIM2 prescaler instead of TIMPRE divider as TIMPRE clock frequency must be >= AHB or a multiple of
	// AHB, not lower
	uint32_t tim_prescaler_new_value = (timer_frequency / 1000000) - 1;

	// Read CFGR2 before changing AXI divider for faster update
	uint32_t cfgr2_new_value = (RCC->CFGR2 & (~RCC_CFGR2_HPRE)) | (hpre_value << RCC_CFGR2_HPRE_Pos);

	uint32_t ic2cfgr_new_value = (RCC->IC2CFGR & (~RCC_IC2CFGR_IC2INT)) | ((divider - 1UL) << RCC_IC2CFGR_IC2INT_Pos);

	// Handle the timer TIMPRE divider as close as possible to the AXI divider to avoid too much bad measurement in
	// TimeMeasure.
	if(divider > current_divider) {
		// We are increasing the AXI divider so reducing frequency

		// First wait for a new counter value of TIM2 so its internal prescaler counter is near 0
		uint32_t tim_counter;
		uint32_t previous_tim_counter = TIM2->CNT;

		__disable_irq();
		while((tim_counter = TIM2->CNT) == previous_tim_counter)
			;
		// TIM2 counter just increased, go on

		// Update AXI divider, reducting its frequency
		RCC->IC2CFGR = ic2cfgr_new_value;

		// Update the prescaler after
		TIM2->PSC = tim_prescaler_new_value;
		// Apply the new prescaler value, but this reset the counter to 0
		TIM2->EGR |= TIM_EGR_UG;
		(void) TIM2->EGR;
		// Restore back the counter value
		TIM2->CNT = tim_counter;

		__enable_irq();

		// Update HPRE divider for AHB
		RCC->CFGR2 = cfgr2_new_value;
	} else {
		// Reducing divider so increasing frequency

		// First wait for a new counter value of TIM2 so its internal prescaler counter is near 0
		uint32_t tim_counter;
		uint32_t previous_tim_counter = TIM2->CNT;

		__disable_irq();
		while((tim_counter = TIM2->CNT) == previous_tim_counter)
			;

		// First reduce AHB frequencies before increasing AXI
		RCC->CFGR2 = cfgr2_new_value;

		// Update the prescaler juste before changing AXI speed for a minimal transition time with wrong frequency
		TIM2->PSC = tim_prescaler_new_value;
		// Apply the new prescaler value, but this reset the counter to 0
		TIM2->EGR |= TIM_EGR_UG;
		(void) TIM2->EGR;
		// Restore back the counter value
		TIM2->CNT = tim_counter;

		// Update AXI divider, increasing its frequency
		RCC->IC2CFGR = ic2cfgr_new_value;

		__enable_irq();
	}

	uv_async_send(&asyncFrequencyChanged);
}

// Max 200Mhz
void CPUFrequencyScaling::setRawAHBDivider(uint32_t divider) {
	divider = clampDivider(divider, LL_RCC_IC2_GetDivider() >= 4 ? 1 : 2, 16, true);

	if(divider == getAHBDivider())
		return;

	// Set AHB frequency divider
	LL_RCC_SetAHBPrescaler(getBitPosition(divider) << RCC_CFGR2_HPRE_Pos);

	uv_async_send(&asyncFrequencyChanged);
}

// Max 1Ghz
void CPUFrequencyScaling::setRawNPUDivider(uint32_t divider) {
	divider = clampDivider(divider, 1, 256, true);

	if(divider == LL_RCC_IC6_GetDivider())
		return;

	// Set NPU frequency divider
	LL_RCC_IC6_SetDivider(divider);

	uv_async_send(&asyncFrequencyChanged);
}

// Max 900Mhz
void CPUFrequencyScaling::setRawAXISRAM3456Divider(uint32_t divider) {
	divider = clampDivider(divider, 1, 256, true);

	if(divider == LL_RCC_IC11_GetDivider())
		return;

	// Set AXISRAM3/4/5/6 frequency divider
	LL_RCC_IC11_SetDivider(divider);

	uv_async_send(&asyncFrequencyChanged);
}

static uint32_t getTimerDivider() {
	return 1 << LL_RCC_GetTIMPrescaler();
}

void CPUFrequencyScaling::setRawTimerDivider(uint32_t index, uint32_t divider) {
	divider = clampDivider(divider, 1, 8, true);

	if(index != 1)
		return;

	if(divider == getTimerDivider())
		return;

	// Set Timer frequency divider
	LL_RCC_SetTIMPrescaler(getBitPosition(divider));

	updateTimerPrescaler();

	uv_async_send(&asyncFrequencyChanged);
}

const uint32_t CPUFrequencyScaling::CPU_USAGE_MARGIN_AT_MAX_FREQUENCY = 7;

uint32_t CPUFrequencyScaling::getCpuUsageWithLowerSpeed(uint32_t value) {
	return value * (current_ahb_divider + 1) / current_ahb_divider;
}

void CPUFrequencyScaling::adjustCpuFreq(CpuFreqAdjustement adjustment) {
	static const uint32_t MIN_DIVIDER = 1;
	uint32_t current_divider = LL_RCC_IC1_GetDivider();
	switch(adjustment) {
		case CpuFreqAdjustement::ResetToMax:
			current_divider = MIN_DIVIDER;
			break;
		case CpuFreqAdjustement::IncreaseSpeed:
			if(current_divider > MIN_DIVIDER)
				current_divider--;
			break;
		case CpuFreqAdjustement::DecreaseSpeed:
			if(current_divider < 256)
				current_divider++;
			break;
	}

	setRawCPUDivider(current_divider);
	setRawAXIDivider(current_divider);
	setRawNPUDivider(current_divider);
	setRawAXISRAM3456Divider(current_divider);
}

#endif

CPUFrequencyScaling::CPUFrequencyScaling(OscRoot* oscRoot)
    : OscContainer(oscRoot, "cpu"),
      oscRoot(oscRoot),
      oscManualControl(this, "manual", false),
#if defined(STM32F723xx)
      oscCpuFrequency(this, "freq", SystemCoreClock),
      oscCpuDivider(this, "divider", 1, false),
#elif defined(STM32N657xx)
      oscPllFrequency(this, "freq", HAL_RCCEx_GetPLL1CLKFreq()),
      oscCpuFrequency(this, "cpuFreq", SystemCoreClock),
      oscAXIFrequency(this, "axiFreq", HAL_RCC_GetSysClockFreq()),
      oscAHBFrequency(this, "ahbFreq", HAL_RCC_GetHCLKFreq()),
      oscTimerFrequency(this, "timerFreq", HAL_RCC_GetSysClockFreq() / getTimerDivider()),
      // Don't persist this variable as we use it also as a readonly variable when "manual" == false
      oscCpuDivider(this, "cpuDivider", LL_RCC_IC1_GetDivider(), false),
      oscAXIDivider(this, "axiDivider", LL_RCC_IC2_GetDivider(), false),
      oscAHBDivider(this, "ahbDivider", getAHBDivider(), false),
      oscTimerDivider(this, "timerDivider", getTimerDivider(), false),
#endif
      current_ahb_divider(1),
      recent_ahb_divider_change(false),
      max_cpu_usage_ratio_per_thousand(0),
      cpu_usage_points(0),
      cpu_usage_points_target(100) {
	uv_async_init(uv_default_loop(), &asyncFrequencyChanged, CPUFrequencyScaling::onFrequencyChanged);
	asyncFrequencyChanged.data = this;

#define DEFINE_CHANGE_CALLBACK(oscDivider, dividerChangeFunction, ...) \
	[this](int32_t value) { \
		if(oscManualControl) { \
			dividerChangeFunction; \
		} \
	}

	auto checkSetDividerAllowed = [this](int32_t v) { return oscManualControl.get(); };
	oscCpuDivider.addCheckCallback(checkSetDividerAllowed);

#if defined(STM32F723xx)
	oscCpuDivider.addChangeCallback(DEFINE_CHANGE_CALLBACK(oscCpuDivider, setAHBDivider(value)));
#elif defined(STM32N657xx)
	oscCpuDivider.addChangeCallback(DEFINE_CHANGE_CALLBACK(oscCpuDivider, setRawCPUDivider(value)));
	oscAXIDivider.addChangeCallback(DEFINE_CHANGE_CALLBACK(oscAXIDivider, setRawAXIDivider(value)));
	oscAHBDivider.addChangeCallback(DEFINE_CHANGE_CALLBACK(oscAHBDivider, setRawAHBDivider(value)));
	oscTimerDivider.addChangeCallback(DEFINE_CHANGE_CALLBACK(oscTimerDivider, setRawTimerDivider(1, value)));

	oscAXIDivider.addCheckCallback(checkSetDividerAllowed);
	oscAHBDivider.addCheckCallback(checkSetDividerAllowed);
	oscTimerDivider.addCheckCallback(checkSetDividerAllowed);
#endif

	resetFrequencyToMaxPerformance();
}

CPUFrequencyScaling::~CPUFrequencyScaling() {}

void CPUFrequencyScaling::init() {
	oscRoot->addValueChangedCallback([this]() { resetFrequencyToMaxPerformance(); });
}

void CPUFrequencyScaling::resetFrequencyToMaxPerformance() {
	// If manual mode, don't auto adjust frequency
	if(oscManualControl)
		return;

	max_cpu_usage_ratio_per_thousand = 0;
	cpu_usage_points = 0;
	cpu_usage_points_target = 100;

	// Go back to max performance in case an option were enabled needing the extra performance
	adjustCpuFreq(CpuFreqAdjustement::ResetToMax);
}

void CPUFrequencyScaling::updateCpuUsage() {
	// If manual mode, don't auto adjust frequency
	if(oscManualControl)
		return;

	// Skip first measure after a frequency change
	if(recent_ahb_divider_change) {
		recent_ahb_divider_change = false;
		return;
	}

	// Ensure previous code calling updateCpuUsage() is not reordered after the measure
	std::atomic_signal_fence(std::memory_order_seq_cst);

	// CPU time in realtime interrupts
	uint32_t cpu_usage_ratio_us = TimeMeasure::timeMeasure[TMI_UsbInterrupt].getMaxTimeUs() +
	                              TimeMeasure::timeMeasure[TMI_AudioProcessing].getMaxTimeUs() +
	                              TimeMeasure::timeMeasure[TMI_OtherIRQ].getMaxTimeUs();

	// Duration of the main loop task so far (if its too long, we will also increase CPU frequency to speed it up)
	uint32_t main_loop_task_duration_us = TimeMeasure::timeMeasure[TMI_MainLoop].getOnGoingDuration();

	cpu_usage_points++;
	if(cpu_usage_ratio_us > max_cpu_usage_ratio_per_thousand) {
		max_cpu_usage_ratio_per_thousand = cpu_usage_ratio_us;

		// We got a new maximum, wait for longer until we have no more maximum
		cpu_usage_points = 0;
	}

	uint32_t current_needed_cpu_usage_room = CPU_USAGE_MARGIN_AT_MAX_FREQUENCY * current_ahb_divider;

	if(main_loop_task_duration_us > 2000) {
		// If main loop current task is running so far for more than 10ms, increase CPU frequency to max
		// to speed it up.
		adjustCpuFreq(CpuFreqAdjustement::IncreaseSpeed);

		// We needed to increase the frequency for the main loop, reset wait time before decreasing again
		cpu_usage_points = 0;
	} else if(max_cpu_usage_ratio_per_thousand > 900 - current_needed_cpu_usage_room) {
		// We are above 90% + 2.5% normalized room cpu usage, increase cpu speed
		adjustCpuFreq(CpuFreqAdjustement::IncreaseSpeed);
	} else if(cpu_usage_points >= cpu_usage_points_target &&
	          (getCpuUsageWithLowerSpeed(max_cpu_usage_ratio_per_thousand) <
	           850 - getCpuUsageWithLowerSpeed(current_needed_cpu_usage_room))) {
		// We can divide cpu frequency by 2 while being under 85% cpu usage + 2.5% normalized room and only if main loop
		// is not active for more than 90% of 1ms period
		adjustCpuFreq(CpuFreqAdjustement::DecreaseSpeed);
	}

	if(cpu_usage_points >= cpu_usage_points_target) {
		// Reset stats even if we didn't changed the divider to update the needed frequency if less speed is needed
		// later
		cpu_usage_points = 0;
		max_cpu_usage_ratio_per_thousand = 0;
	}
}

void CPUFrequencyScaling::onFrequencyChanged(uv_async_t* handle) {
	CPUFrequencyScaling* thisInstance = (CPUFrequencyScaling*) handle->data;

#if defined(STM32F723xx)
	thisInstance->oscCpuDivider.set(thisInstance->current_ahb_divider);
	thisInstance->oscCpuFrequency.set(SystemCoreClock);
#elif defined(STM32N657xx)
	thisInstance->oscCpuDivider.setNoCheck(LL_RCC_IC1_GetDivider());
	thisInstance->oscAXIDivider.setNoCheck(LL_RCC_IC2_GetDivider());
	thisInstance->oscAHBDivider.setNoCheck(getAHBDivider());
	thisInstance->oscTimerDivider.setNoCheck(getTimerDivider());

	thisInstance->oscCpuFrequency.set(SystemCoreClock);
	thisInstance->oscAXIFrequency.set(HAL_RCC_GetSysClockFreq());
	thisInstance->oscAHBFrequency.set(HAL_RCC_GetHCLKFreq());
	thisInstance->oscTimerFrequency.set(HAL_RCC_GetSysClockFreq() / getTimerDivider());
#endif
}