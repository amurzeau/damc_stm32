#pragma once

#include <stdint.h>

void STACKMEASURE_init();
uint32_t STACKMEASURE_getUsedSize();
uint32_t STACKMEASURE_getTotalSize();
