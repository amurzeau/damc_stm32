#pragma once

#include "AudioCApi.h"
#include PLATFORM_HEADER
#include <CircularQueue.h>
#include <TimeMeasure.h>

#define DEFINE_ThreadInISR(name, arg_type, irq_name, priority) \
	ThreadInISR<arg_type> name{irq_name##_IRQn, priority}; \
	extern "C" void irq_name##_IRQHandler(void) { \
		name.callFromISR(); \
	}

template<class ArgType> class ThreadInISR {
public:
	typedef void (*ThreadFunction)(ArgType arg);

	ThreadInISR(IRQn_Type irqn, int priority);

	bool triggerRun(ThreadFunction threadFunction, ArgType arg);

	void callFromISR();

private:
	struct PendingExecution {
		ThreadFunction threadFunction;
		ArgType arg;
	};
	CircularQueue<PendingExecution, 16> pendingExecutions;

	const IRQn_Type irqn;
};

template<class ArgType> ThreadInISR<ArgType>::ThreadInISR(IRQn_Type irqn, int priority) : irqn(irqn) {
	NVIC_SetPriority(irqn, priority);
	NVIC_ClearPendingIRQ(irqn);
	NVIC_EnableIRQ(irqn);
}

template<class ArgType> bool ThreadInISR<ArgType>::triggerRun(ThreadFunction threadFunction, ArgType arg) {
	if(pendingExecutions.write(PendingExecution{.threadFunction = threadFunction, .arg = arg}) == 0)
		return false;

	NVIC_SetPendingIRQ(irqn);

	return true;
}

template<class ArgType> void ThreadInISR<ArgType>::callFromISR() {
	TimeMeasure::timeMeasure[TMI_OtherIRQ].beginMeasure();

	PendingExecution pendingExecution;

	if(pendingExecutions.read(&pendingExecution) > 0) {
		pendingExecution.threadFunction(pendingExecution.arg);
	}

	TimeMeasure::timeMeasure[TMI_OtherIRQ].endMeasure();
}