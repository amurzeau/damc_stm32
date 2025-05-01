#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SAMPLING_PROFILER_ENABLE

#ifdef SAMPLING_PROFILER_ENABLE
void SAMPLINGPROFILER_capture(const uint32_t* sp, const uint32_t* current_sp);
#else
#endif

#ifdef __cplusplus
}
#endif
