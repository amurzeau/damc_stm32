#pragma once

#include "AudioCApi.h"
#include PLATFORM_HEADER
#include <CircularQueue.h>
#include <TimeMeasure.h>

#define DEFINE_ThreadInISR(name, arg_type, irq_name, priority, timeMeasureItem, loopDurationUs) \
	ThreadInISR<arg_type> name{irq_name##_IRQn, priority, timeMeasureItem, loopDurationUs}; \
	extern "C" void irq_name##_IRQHandler(void) { \
		name.callFromISR(); \
	}

template<class ArgType> class ThreadInISR {
public:
	typedef void (*ThreadFunction)(ArgType arg);

	ThreadInISR(IRQn_Type irqn, int priority, TimeMeasureItem timeMeasureItem, uint32_t loopDurationUs);

	bool triggerRun(ThreadFunction threadFunction, ArgType arg, bool loopIndicator);

	void callFromISR();

private:
	struct PendingExecution {
		ThreadFunction threadFunction;
		ArgType arg;
		bool loopIndicator;  // When true, this indicate a new loop is starting (with possibly other executions to be
		                     // done with loopIndicator == false)
	};
	CircularQueue<PendingExecution, 16> pendingExecutions;

	const IRQn_Type irqn;
	const TimeMeasureItem timeMeasureItem;
	const uint32_t loopDurationUs;
};

template<class ArgType>
ThreadInISR<ArgType>::ThreadInISR(IRQn_Type irqn,
                                  int priority,
                                  TimeMeasureItem timeMeasureItem,
                                  uint32_t loopDurationUs)
    : irqn(irqn), timeMeasureItem(timeMeasureItem), loopDurationUs(loopDurationUs) {
	NVIC_SetPriority(irqn, priority);
	NVIC_ClearPendingIRQ(irqn);
	NVIC_EnableIRQ(irqn);
}

template<class ArgType>
bool ThreadInISR<ArgType>::triggerRun(ThreadFunction threadFunction, ArgType arg, bool loopIndicator) {
	if(pendingExecutions.write(
	       PendingExecution{.threadFunction = threadFunction, .arg = arg, .loopIndicator = loopIndicator}) == 0)
		return false;

	NVIC_SetPendingIRQ(irqn);

	return true;
}

template<class ArgType> void ThreadInISR<ArgType>::callFromISR() {
	PendingExecution pendingExecution;

	while(pendingExecutions.read(&pendingExecution) > 0) {
		TimeMeasure::timeMeasure[timeMeasureItem].beginMeasure();
		pendingExecution.threadFunction(pendingExecution.arg);
		TimeMeasure::timeMeasure[timeMeasureItem].endMeasure();

		if(pendingExecution.loopIndicator)
			TimeMeasure::timeMeasure[timeMeasureItem].endOfProcessingLoop(loopDurationUs);
	}
}