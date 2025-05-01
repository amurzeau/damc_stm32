#include "SamplingProfiler.h"
#include "main.h"
#include <csignal>
#include PLATFORM_HEADER
#include <Utils.h>

#ifdef SAMPLING_PROFILER_ENABLE

#include <atomic>

struct SampledCallstack {
	std::atomic<uint32_t> callstack_present;
	uint32_t registers[16];
	uint32_t callstack[1024];
	uint32_t original_pc;
	uint32_t dma_timestamp;
};

__attribute__((used)) struct SampledCallstack sampled_callstack_data;
__attribute__((used)) uint32_t pc_cond;
__attribute__((used)) uint32_t sp_cond;

extern uint8_t _estack;

void SAMPLINGPROFILER_capture(const uint32_t* sp, const uint32_t* current_sp) {
	uint32_t timestamp = TIM2->CNT;
	STMOD_UART4_RXD_GPIO_Port->BSRR |= STMOD_UART4_RXD_Pin;

	uint32_t i;

	uint32_t irq_lr = current_sp[9];
	uint32_t psr = sp[7];
	const uint32_t* preempted_sp;

	if(irq_lr & (1 << 4)) {
		// No FPU context saved
		preempted_sp = sp + 8;
	} else {
		// FPU context saved
		preempted_sp = sp + 26;
	}
	if(psr & (1 << 9)) {
		// Stack was not 8 bytes aligned
		preempted_sp++;
	}

	uint32_t saved_pc = sp[6];
	uint32_t partial_instruction = 0;

	if(((SCB->ID_ISAR[2] >> 8) & 0xf) >= 2) {
		// Check for continuable instruction like push or vpush which should be assumed already executed
		if(((psr >> 25) & 3) == 0x0 && ((psr >> 12) & 0xf) != 0x0 && ((psr >> 10) & 3) == 0x0) {
			// ICI value indicate an interrupted LDM/STM/POP/PUSH/VPOP/VPUSH instruction
			// Only POP and VPOP keep the initial base register value.
			// Other instructions use the final value.
			// See ARMv7-M Architecture Reference Manual.

			bool assume_partial_instruction_done = true;

			uint16_t pc_instruction_16bits = *(uint16_t*) sp[6];

			if((pc_instruction_16bits >> 9) == 0b1011110) {
				// POP encoding T1
				assume_partial_instruction_done = false;
			} else if(pc_instruction_16bits == 0b1110100010111101) {
				// POP encoding T2
				assume_partial_instruction_done = false;
			} else if(pc_instruction_16bits == 0b1111100001011101) {
				// POP encoding T3
				assume_partial_instruction_done = false;
			} else if((pc_instruction_16bits & 0b1111111110111111) == 0b1110110010111101) {
				// VPOP encoding T1
				assume_partial_instruction_done = false;
			} else if((pc_instruction_16bits & 0b1111111110111111) == 0b1110110010111101) {
				// VPOP encoding T2
				assume_partial_instruction_done = false;
			}

			if(assume_partial_instruction_done) {
				// Assume instruction is done, so increment saved_pc to the next instruction
				// Rules for instruction size:
				// If bits [15:11] of the halfword being decoded take any of the following
				// values, the halfword is the first halfword of a 32-bit instruction:
				// • 0b11101.
				// • 0b11110.
				// • 0b11111.
				// Otherwise, the halfword is a 16-bit instruction.
				// See ARMv7-M Architecture Reference Manual.

				uint32_t instruction_masked = pc_instruction_16bits >> 11;
				if(instruction_masked == 0b11101 || instruction_masked == 0b11110 || instruction_masked == 0b11111) {
					saved_pc += 4;
				} else {
					saved_pc += 2;
				}

				partial_instruction = 1;
			}
		}
	}

	if((uint32_t) preempted_sp == sp_cond && saved_pc == pc_cond) {
		while(1)
			;
	}

	if(sampled_callstack_data.callstack_present)
		return;

	Utils::copy_n(&sampled_callstack_data.registers[0], sp, 4);
	Utils::copy_n(&sampled_callstack_data.registers[4], current_sp, 9);
	sampled_callstack_data.registers[13] = (uint32_t) preempted_sp;
	sampled_callstack_data.registers[14] = sp[5];     // lr
	sampled_callstack_data.registers[15] = saved_pc;  // pc
	if(partial_instruction)
		sampled_callstack_data.original_pc = sp[6];
	else
		sampled_callstack_data.original_pc = 0;
	sampled_callstack_data.dma_timestamp = DMA2_Stream3->NDTR;

	for(i = 0; i < sizeof(sampled_callstack_data.callstack) / sizeof(sampled_callstack_data.callstack[0]) &&
	           preempted_sp < (uint32_t*) &_estack;
	    preempted_sp++, i++) {
		sampled_callstack_data.callstack[i] = *preempted_sp;
	}

	STMOD_UART4_RXD_GPIO_Port->BSRR |= STMOD_UART4_RXD_Pin << 16;
	sampled_callstack_data.callstack_present = timestamp;
}

#endif
