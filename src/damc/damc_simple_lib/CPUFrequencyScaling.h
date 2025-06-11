#pragma once

#include <Osc/OscReadOnlyVariable.h>
#include <Osc/OscVariable.h>
#include <OscRoot.h>
#include <stdint.h>
#include <uv.h>

class CPUFrequencyScaling : public OscContainer {
public:
	CPUFrequencyScaling(OscRoot* oscRoot);
	~CPUFrequencyScaling();

	void init();
	void resetFrequencyToMaxPerformance();
	void updateCpuUsage();

protected:
	enum class CpuFreqAdjustement {
		ResetToMax,
		IncreaseSpeed,
		DecreaseSpeed,
	};
	void adjustCpuFreq(CpuFreqAdjustement adjustment);
	uint32_t getCpuUsageWithLowerSpeed(uint32_t value);
	static void onFrequencyChanged(uv_async_t* handle);

#if defined(STM32F723xx)
	void setAHBDivider(uint32_t divider);
#elif defined(STM32N657xx)
	void setRawCPUDivider(uint32_t divider);
	void setRawAXIDivider(uint32_t divider);
	void setRawAHBDivider(uint32_t divider);
	void setRawNPUDivider(uint32_t divider);
	void setRawAXISRAM3456Divider(uint32_t divider);
	void setRawTimerDivider(uint32_t index, uint32_t divider);
#endif

private:
	OscRoot* oscRoot;
	OscVariable<bool> oscManualControl;

#if defined(STM32F723xx)
	OscReadOnlyVariable<int32_t> oscCpuFrequency;
	OscVariable<int32_t> oscCpuDivider;
#elif defined(STM32N657xx)
	OscReadOnlyVariable<int32_t> oscPllFrequency;
	OscReadOnlyVariable<int32_t> oscCpuFrequency;
	OscReadOnlyVariable<int32_t> oscAXIFrequency;
	OscReadOnlyVariable<int32_t> oscAHBFrequency;
	OscReadOnlyVariable<int32_t> oscTimerFrequency;
	OscReadOnlyVariable<int32_t> oscNPUFrequency;
	OscReadOnlyVariable<int32_t> oscNPUSRAMFrequency;

	OscVariable<int32_t> oscCpuDivider;
	OscVariable<int32_t> oscAXIDivider;
	OscVariable<int32_t> oscAHBDivider;
	OscVariable<int32_t> oscTimerDivider;
	OscVariable<int32_t> oscNPUDivider;
	OscVariable<int32_t> oscNPUSRAMDivider;
#endif

	uint32_t current_ahb_divider;
	bool recent_ahb_divider_change;

	uint32_t max_cpu_usage_ratio_per_thousand;
	uint32_t cpu_usage_points;
	uint32_t cpu_usage_points_target;

	uv_async_t asyncFrequencyChanged;

	static const uint32_t CPU_USAGE_MARGIN_AT_MAX_FREQUENCY;
};
