#include "StackMeasure.h"
#include <string.h>

#define STACK_UNUSED_PATTERN 0xAAAAAAAA

static uint32_t* last_used_stack_size = nullptr;

extern "C" uint8_t _sstack;
extern "C" uint8_t _estack;

void STACKMEASURE_init() {
	uint8_t array[1];
	memset((uint32_t*) &_sstack, 0xAA, &array[0] - &_sstack);
	// Store address of array without using it directly to silence warning
	last_used_stack_size = (uint32_t*) (&_sstack + ((uint32_t) (&array[0] - &_sstack) & 0xFFFFFFE0)) + 1;
}

uint32_t STACKMEASURE_getUsedSize() {
	for(; last_used_stack_size > (uint32_t*) &_sstack; last_used_stack_size--) {
		if(*last_used_stack_size == STACK_UNUSED_PATTERN) {
			break;
		}
	}

	return (&_estack - (uint8_t*) last_used_stack_size);
}

uint32_t STACKMEASURE_getTotalSize() {
	return &_estack - &_sstack;
}
