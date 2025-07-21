#pragma once

#include "AudioCApi.h"
#include <atomic>
#include PLATFORM_HEADER
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
	bool getIsPending() { return isPending; }

	void callFromISR();

private:
	ThreadFunction threadFunction = nullptr;
	ArgType arg;

	IRQn_Type irqn;
	std::atomic_bool isPending = false;
};

template<class ArgType> ThreadInISR<ArgType>::ThreadInISR(IRQn_Type irqn, int priority) : irqn(irqn) {
	NVIC_SetPriority(irqn, priority);
	NVIC_ClearPendingIRQ(irqn);
	NVIC_EnableIRQ(irqn);
}

template<class ArgType> bool ThreadInISR<ArgType>::triggerRun(ThreadFunction threadFunction, ArgType arg) {
	if(isPending)
		return false;

	this->threadFunction = threadFunction;
	this->arg = arg;

	isPending = true;
	NVIC_SetPendingIRQ(irqn);

	return true;
}

template<class ArgType> void ThreadInISR<ArgType>::callFromISR() {
	TimeMeasure::timeMeasure[TMI_OtherIRQ].beginMeasure();
	if(threadFunction != nullptr)
		threadFunction(arg);

	isPending = false;
	TimeMeasure::timeMeasure[TMI_OtherIRQ].endMeasure();
}