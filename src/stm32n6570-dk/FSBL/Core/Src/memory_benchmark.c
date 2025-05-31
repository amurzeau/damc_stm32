
#include "memory_benchmark.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stm32n6xx_hal.h>

struct event_counter_desc
{
  uint16_t flag;
  const char *name;
};
/*
struct event_counter_desc counters[] = {
  {ARM_PMU_SW_INCR, "Software update to the PMU_SWINC register, architecturally executed and condition code check pass"},
  {ARM_PMU_L1I_CACHE_REFILL, "L1 I-Cache refill"},
  {ARM_PMU_L1D_CACHE_REFILL, "L1 D-Cache refill"},
  {ARM_PMU_L1D_CACHE, "L1 D-Cache access"},
  {ARM_PMU_LD_RETIRED, "Memory-reading instruction architecturally executed and condition code check pass"},
  {ARM_PMU_ST_RETIRED, "Memory-writing instruction architecturally executed and condition code check pass"},
  {ARM_PMU_INST_RETIRED, "Instruction architecturally executed"},
  {ARM_PMU_EXC_TAKEN, "Exception entry"},
  {ARM_PMU_EXC_RETURN, "Exception return instruction architecturally executed and the condition code check pass"},
  {ARM_PMU_PC_WRITE_RETIRED, "Software change to the Program Counter (PC). Instruction is architecturally executed and condition code check pass"},
  {ARM_PMU_BR_IMMED_RETIRED, "Immediate branch architecturally executed"},
  {ARM_PMU_BR_RETURN_RETIRED, "Function return instruction architecturally executed and the condition code check pass"},
  {ARM_PMU_UNALIGNED_LDST_RETIRED, "Unaligned memory memory-reading or memory-writing instruction architecturally executed and condition code check pass"},
  {ARM_PMU_BR_MIS_PRED, "Mispredicted or not predicted branch speculatively executed"},
  {ARM_PMU_CPU_CYCLES, "Cycle"},
  {ARM_PMU_BR_PRED, "Predictable branch speculatively executed"},
  {ARM_PMU_MEM_ACCESS, "Data memory access"},
  {ARM_PMU_L1I_CACHE, "Level 1 instruction cache access"},
  {ARM_PMU_L1D_CACHE_WB, "Level 1 data cache write-back"},
  {ARM_PMU_L2D_CACHE, "Level 2 data cache access"},
  {ARM_PMU_L2D_CACHE_REFILL, "Level 2 data cache refill"},
  {ARM_PMU_L2D_CACHE_WB, "Level 2 data cache write-back"},
  {ARM_PMU_BUS_ACCESS, "Bus access"},
  {ARM_PMU_MEMORY_ERROR, "Local memory error"},
  {ARM_PMU_INST_SPEC, "Instruction speculatively executed"},
  {ARM_PMU_BUS_CYCLES, "Bus cycles"},
  {ARM_PMU_L1D_CACHE_ALLOCATE, "Level 1 data cache allocation without refill"},
  {ARM_PMU_L2D_CACHE_ALLOCATE, "Level 2 data cache allocation without refill"},
  {ARM_PMU_BR_RETIRED, "Branch instruction architecturally executed"},
  {ARM_PMU_BR_MIS_PRED_RETIRED, "Mispredicted branch instruction architecturally executed"},
  {ARM_PMU_STALL_FRONTEND, "No operation issued because of the frontend"},
  {ARM_PMU_STALL_BACKEND, "No operation issued because of the backend"},
  {ARM_PMU_L2I_CACHE, "Level 2 instruction cache access"},
  {ARM_PMU_L2I_CACHE_REFILL, "Level 2 instruction cache refill"},
  {ARM_PMU_L3D_CACHE_ALLOCATE, "Level 3 data cache allocation without refill"},
  {ARM_PMU_L3D_CACHE_REFILL, "Level 3 data cache refill"},
  {ARM_PMU_L3D_CACHE, "Level 3 data cache access"},
  {ARM_PMU_L3D_CACHE_WB, "Level 3 data cache write-back"},
  {ARM_PMU_LL_CACHE_RD, "Last level data cache read"},
  {ARM_PMU_LL_CACHE_MISS_RD, "Last level data cache read miss"},
  {ARM_PMU_L1D_CACHE_MISS_RD, "Level 1 data cache read miss"},
  {ARM_PMU_OP_COMPLETE, "Operation retired"},
  {ARM_PMU_OP_SPEC, "Operation speculatively executed"},
  {ARM_PMU_STALL, "Stall cycle for instruction or operation not sent for execution"},
  {ARM_PMU_STALL_OP_BACKEND, "Stall cycle for instruction or operation not sent for execution due to pipeline backend"},
  {ARM_PMU_STALL_OP_FRONTEND, "Stall cycle for instruction or operation not sent for execution due to pipeline frontend"},
  {ARM_PMU_STALL_OP, "Instruction or operation slots not occupied each cycle"},
  {ARM_PMU_L1D_CACHE_RD, "Level 1 data cache read"},
  {ARM_PMU_LE_RETIRED, "Loop end instruction executed"},
  {ARM_PMU_LE_SPEC, "Loop end instruction speculatively executed"},
  {ARM_PMU_BF_RETIRED, "Branch future instruction architecturally executed and condition code check pass"},
  {ARM_PMU_BF_SPEC, "Branch future instruction speculatively executed and condition code check pass"},
  {ARM_PMU_LE_CANCEL, "Loop end instruction not taken"},
  {ARM_PMU_BF_CANCEL, "Branch future instruction not taken"},
  {ARM_PMU_SE_CALL_S, "Call to secure function, resulting in Security state change"},
  {ARM_PMU_SE_CALL_NS, "Call to non-secure function, resulting in Security state change"},
  {ARM_PMU_DWT_CMPMATCH0, "DWT comparator 0 match"},
  {ARM_PMU_DWT_CMPMATCH1, "DWT comparator 1 match"},
  {ARM_PMU_DWT_CMPMATCH2, "DWT comparator 2 match"},
  {ARM_PMU_DWT_CMPMATCH3, "DWT comparator 3 match"},
  {ARM_PMU_MVE_INST_RETIRED, "MVE instruction architecturally executed"},
  {ARM_PMU_MVE_INST_SPEC, "MVE instruction speculatively executed"},
  {ARM_PMU_MVE_FP_RETIRED, "MVE floating-point instruction architecturally executed"},
  {ARM_PMU_MVE_FP_SPEC, "MVE floating-point instruction speculatively executed"},
  {ARM_PMU_MVE_FP_HP_RETIRED, "MVE half-precision floating-point instruction architecturally executed"},
  {ARM_PMU_MVE_FP_HP_SPEC, "MVE half-precision floating-point instruction speculatively executed"},
  {ARM_PMU_MVE_FP_SP_RETIRED, "MVE single-precision floating-point instruction architecturally executed"},
  {ARM_PMU_MVE_FP_SP_SPEC, "MVE single-precision floating-point instruction speculatively executed"},
  {ARM_PMU_MVE_FP_MAC_RETIRED, "MVE floating-point multiply or multiply-accumulate instruction architecturally executed"},
  {ARM_PMU_MVE_FP_MAC_SPEC, "MVE floating-point multiply or multiply-accumulate instruction speculatively executed"},
  {ARM_PMU_MVE_INT_RETIRED, "MVE integer instruction architecturally executed"},
  {ARM_PMU_MVE_INT_SPEC, "MVE integer instruction speculatively executed"},
  {ARM_PMU_MVE_INT_MAC_RETIRED, "MVE multiply or multiply-accumulate instruction architecturally executed"},
  {ARM_PMU_MVE_INT_MAC_SPEC, "MVE multiply or multiply-accumulate instruction speculatively executed"},
  {ARM_PMU_MVE_LDST_RETIRED, "MVE load or store instruction architecturally executed"},
  {ARM_PMU_MVE_LDST_SPEC, "MVE load or store instruction speculatively executed"},
  {ARM_PMU_MVE_LD_RETIRED, "MVE load instruction architecturally executed"},
  {ARM_PMU_MVE_LD_SPEC, "MVE load instruction speculatively executed"},
  {ARM_PMU_MVE_ST_RETIRED, "MVE store instruction architecturally executed"},
  {ARM_PMU_MVE_ST_SPEC, "MVE store instruction speculatively executed"},
  {ARM_PMU_MVE_LDST_CONTIG_RETIRED, "MVE contiguous load or store instruction architecturally executed"},
  {ARM_PMU_MVE_LDST_CONTIG_SPEC, "MVE contiguous load or store instruction speculatively executed"},
  {ARM_PMU_MVE_LD_CONTIG_RETIRED, "MVE contiguous load instruction architecturally executed"},
  {ARM_PMU_MVE_LD_CONTIG_SPEC, "MVE contiguous load instruction speculatively executed"},
  {ARM_PMU_MVE_ST_CONTIG_RETIRED, "MVE contiguous store instruction architecturally executed"},
  {ARM_PMU_MVE_ST_CONTIG_SPEC, "MVE contiguous store instruction speculatively executed"},
  {ARM_PMU_MVE_LDST_NONCONTIG_RETIRED, "MVE non-contiguous load or store instruction architecturally executed"},
  {ARM_PMU_MVE_LDST_NONCONTIG_SPEC, "MVE non-contiguous load or store instruction speculatively executed"},
  {ARM_PMU_MVE_LD_NONCONTIG_RETIRED, "MVE non-contiguous load instruction architecturally executed"},
  {ARM_PMU_MVE_LD_NONCONTIG_SPEC, "MVE non-contiguous load instruction speculatively executed"},
  {ARM_PMU_MVE_ST_NONCONTIG_RETIRED, "MVE non-contiguous store instruction architecturally executed"},
  {ARM_PMU_MVE_ST_NONCONTIG_SPEC, "MVE non-contiguous store instruction speculatively executed"},
  {ARM_PMU_MVE_LDST_MULTI_RETIRED, "MVE memory instruction targeting multiple registers architecturally executed"},
  {ARM_PMU_MVE_LDST_MULTI_SPEC, "MVE memory instruction targeting multiple registers speculatively executed"},
  {ARM_PMU_MVE_LD_MULTI_RETIRED, "MVE memory load instruction targeting multiple registers architecturally executed"},
  {ARM_PMU_MVE_LD_MULTI_SPEC, "MVE memory load instruction targeting multiple registers speculatively executed"},
  {ARM_PMU_MVE_ST_MULTI_RETIRED, "MVE memory store instruction targeting multiple registers architecturally executed"},
  {ARM_PMU_MVE_ST_MULTI_SPEC, "MVE memory store instruction targeting multiple registers speculatively executed"},
  {ARM_PMU_MVE_LDST_UNALIGNED_RETIRED, "MVE unaligned memory load or store instruction architecturally executed"},
  {ARM_PMU_MVE_LDST_UNALIGNED_SPEC, "MVE unaligned memory load or store instruction speculatively executed"},
  {ARM_PMU_MVE_LD_UNALIGNED_RETIRED, "MVE unaligned load instruction architecturally executed"},
  {ARM_PMU_MVE_LD_UNALIGNED_SPEC, "MVE unaligned load instruction speculatively executed"},
  {ARM_PMU_MVE_ST_UNALIGNED_RETIRED, "MVE unaligned store instruction architecturally executed"},
  {ARM_PMU_MVE_ST_UNALIGNED_SPEC, "MVE unaligned store instruction speculatively executed"},
  {ARM_PMU_MVE_LDST_UNALIGNED_NONCONTIG_RETIRED, "MVE unaligned noncontiguous load or store instruction architecturally executed"},
  {ARM_PMU_MVE_LDST_UNALIGNED_NONCONTIG_SPEC, "MVE unaligned noncontiguous load or store instruction speculatively executed"},
  {ARM_PMU_MVE_VREDUCE_RETIRED, "MVE vector reduction instruction architecturally executed"},
  {ARM_PMU_MVE_VREDUCE_SPEC, "MVE vector reduction instruction speculatively executed"},
  {ARM_PMU_MVE_VREDUCE_FP_RETIRED, "MVE floating-point vector reduction instruction architecturally executed"},
  {ARM_PMU_MVE_VREDUCE_FP_SPEC, "MVE floating-point vector reduction instruction speculatively executed"},
  {ARM_PMU_MVE_VREDUCE_INT_RETIRED, "MVE integer vector reduction instruction architecturally executed"},
  {ARM_PMU_MVE_VREDUCE_INT_SPEC, "MVE integer vector reduction instruction speculatively executed"},
  {ARM_PMU_MVE_PRED, "Cycles where one or more predicated beats architecturally executed"},
  {ARM_PMU_MVE_STALL, "Stall cycles caused by an MVE instruction"},
  {ARM_PMU_MVE_STALL_RESOURCE, "Stall cycles caused by an MVE instruction because of resource conflicts"},
  {ARM_PMU_MVE_STALL_RESOURCE_MEM, "Stall cycles caused by an MVE instruction because of memory resource conflicts"},
  {ARM_PMU_MVE_STALL_RESOURCE_FP, "Stall cycles caused by an MVE instruction because of floating-point resource conflicts"},
  {ARM_PMU_MVE_STALL_RESOURCE_INT, "Stall cycles caused by an MVE instruction because of integer resource conflicts"},
  {ARM_PMU_MVE_STALL_BREAK, "Stall cycles caused by an MVE chain break"},
  {ARM_PMU_MVE_STALL_DEPENDENCY, "Stall cycles caused by MVE register dependency"},
  {ARM_PMU_ITCM_ACCESS, "Instruction TCM access"},
  {ARM_PMU_DTCM_ACCESS, "Data TCM access"},
  {ARM_PMU_TRCEXTOUT0, "ETM external output 0"},
  {ARM_PMU_TRCEXTOUT1, "ETM external output 1"},
  {ARM_PMU_TRCEXTOUT2, "ETM external output 2"},
  {ARM_PMU_TRCEXTOUT3, "ETM external output 3"},
  {ARM_PMU_CTI_TRIGOUT4, "Cross-trigger Interface output trigger 4"},
  {ARM_PMU_CTI_TRIGOUT5, "Cross-trigger Interface output trigger 5"},
  {ARM_PMU_CTI_TRIGOUT6, "Cross-trigger Interface output trigger 6"},
  {ARM_PMU_CTI_TRIGOUT7, "Cross-trigger Interface output trigger 7"},
  {0xFFFF, "Total Cycles"},
};
  */

struct event_counter_desc counters[] = {
  {ARM_PMU_SW_INCR, "Software update to the PMU_SWINC register, architecturally executed and condition code check pass"},
  {ARM_PMU_L1I_CACHE_REFILL, "L1 I-Cache refill"},
  {ARM_PMU_L1D_CACHE_REFILL, "L1 D-Cache refill"},
  {ARM_PMU_L1D_CACHE, "L1 D-Cache access"},
  {ARM_PMU_LD_RETIRED, "Memory-reading instruction architecturally executed and condition code check pass"},
  {ARM_PMU_CPU_CYCLES, "Cycle"},
  {ARM_PMU_MVE_LD_RETIRED, "MVE load instruction architecturally executed"},
};
static uint32_t counters_value[sizeof(counters) / sizeof(counters[0])];

uint32_t getCounter(uint32_t pmu_flag)
{
  for (size_t i = 0; i < sizeof(counters) / sizeof(counters[0]); i++)
  {
    if (counters[i].flag == pmu_flag)
    {
      return counters_value[i];
    }
  }
  return 0;
}

struct memory_info_t
{
  const char *name;
  uint32_t address;
  uint32_t size_bytes;
};

struct memory_info_t benchmark_memories[] = {
  {"DTCM", 0x20000000, 0x20000},
  {"FLEXMEM", 0x34000000, 0x40000},
  {"AXISRAM1", 0x34064000, 0x80000},
  {"AXISRAM2", 0x34100000, 0x100000},
  {"AXISRAM3", 0x34200000, 0x40000},
  {"AXISRAM4", 0x34270000, 0x40000},
  {"AXISRAM5", 0x342e0000, 0x40000},
  {"AXISRAM6", 0x34350000, 0x40000},
  {"PSRAM_XSPI1_800Mhz", 0x90000000, 0x100000},
  {"NOR_XSPI2", 0x70000000, 0x100000},
};

uint32_t cpu_divider[] = {1, 2, 4};

extern void memory_benchmark_single(uint32_t address, uint32_t size_bytes);

void memory_benchmark_performance()
{
  // Increase hardware prefetcher performance
  MEMSYSCTL->PFCR = 0b1101101101;

  /* Enable Trace */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

  /* Reset Cycle Counter and Event Counters */
  ARM_PMU_CYCCNT_Reset();

  /* Enable Cycle Counter */
  ARM_PMU_CNTR_Enable(PMU_CNTENSET_CCNTR_ENABLE_Msk);

  /* Enable the PMU */
  ARM_PMU_Enable();

  HAL_SuspendTick();

  printf("%s;%s;%s;%s;%s;%s;%s;%s\n", "CPU Freq", "Memory", "Address", "Total Cycles", "Number of VLDR executed", "MBits/s", "MBytes/s", "Active CPU cycles x1M /s");

  for (size_t c = 0; c < sizeof(cpu_divider) / sizeof(cpu_divider[0]); c++)
  {
    // Update CPU frequency
    LL_RCC_IC1_SetDivider(cpu_divider[c]);
    (void)RCC->IC1CFGR;
    SystemCoreClockUpdate();

    for (size_t m = 0; m < sizeof(benchmark_memories) / sizeof(benchmark_memories[0]); m++)
    {
      for (size_t i = 0; i < sizeof(counters) / sizeof(counters[0]); i++)
      {
        const struct event_counter_desc *const event_counter = &counters[i];
        ARM_PMU_EVCNTR_ALL_Reset();
        ARM_PMU_CYCCNT_Reset();
        if (event_counter->flag != 0xFFFF)
        {
          ARM_PMU_Set_EVTYPER(0, event_counter->flag);
          ARM_PMU_Set_EVTYPER(1, ARM_PMU_CHAIN);
        }
        SCB_CleanInvalidateDCache();
        ARM_PMU_CNTR_Enable(PMU_CNTENCLR_CCNTR_ENABLE_Msk | PMU_CNTENCLR_CNT0_ENABLE_Msk | PMU_CNTENSET_CNT1_ENABLE_Msk);

        memory_benchmark_single(benchmark_memories[m].address, benchmark_memories[m].size_bytes);

        ARM_PMU_CNTR_Disable(PMU_CNTENCLR_CCNTR_ENABLE_Msk | PMU_CNTENCLR_CNT0_ENABLE_Msk | PMU_CNTENSET_CNT1_ENABLE_Msk);
        if (event_counter->flag != 0xFFFF)
        {
          counters_value[i] = ARM_PMU_Get_EVCNTR(0) | (ARM_PMU_Get_EVCNTR(1) << 16);
        }
        else
        {
          counters_value[i] = ARM_PMU_Get_CCNTR();
        }
      }
      /*
    for (size_t i = 0; i < sizeof(counters) / sizeof(counters[0]); i++)
    {
      const struct event_counter_desc *const event_counter = &counters[i];
      printf("%s:  %s: %d\n", benchmark_memories[m].name, event_counter->name, counters_value[i]);
    }
    printf("\n");
	*/

      uint32_t cpu_frequency = SystemCoreClock / 1000000;
      uint32_t cycles = getCounter(ARM_PMU_CPU_CYCLES);
      uint32_t vldr_executed = getCounter(ARM_PMU_MVE_LD_RETIRED);
      uint32_t mbits = ((uint64_t)vldr_executed) * 128 * cpu_frequency / cycles;
      uint32_t mbytes = mbits / 8;
      uint32_t active_cpu_cycles = ((uint64_t)vldr_executed) * 2 * cpu_frequency / cycles;

      printf("%u;%s;0x%X;%u;%u;%u;%u;%u\n",
             cpu_frequency,
             benchmark_memories[m].name,
             benchmark_memories[m].address,
             cycles,
             vldr_executed,
             mbits,
             mbytes,
             active_cpu_cycles);
    }
    printf("\n");
  }

  HAL_ResumeTick();
}
