
.syntax unified
#ifndef __clang__
.arch armv8.1-m.main
.arch_extension mve
#else
.cpu cortex-m55
#endif
.thumb

/*
// Use r2 == 2048 to not go outside minimal ram size of DTCM
DTCM	0x20000000
// Use r2 == 4096 to not go outside minimal ram size FLEXMEM
FLEXMEM	0x34000000
AXISRAM1	0x34064000
AXISRAM2	0x34100000
AXISRAM3	0x34200000
AXISRAM4	0x34270000
AXISRAM5	0x342e0000
AXISRAM6	0x34350000

// Use r2 == 256 to not go outside SRAM size
AHBSRAM1 with VLDR linear	0x38000000
AHBSRAM2 with VLDR linear	0x38004000

*/

#define USE_MVE_LOADS
#define NO_OUTER_LOOP

.global memory_benchmark_single
.section .text.memory_benchmark_single
.type memory_benchmark_single, %function
.p2align 5
memory_benchmark_single:
	// Argument 1 (r0): address of memory to test
	// Argument 1 (r1): how many bytes to test

#ifndef NO_OUTER_LOOP
	push.w	{r4, r5, r6, r7, lr}


	// Address start
	mov r5, r0

	// Number of outer loop to target 1MB total read
	mov r6, #0x100000
	udiv r6, r6, r1

	// Size in bytes
	mov r7, r1
#else
	push.w	{lr}
#endif
	
loop2:
	// How many loop to read the RAM
	// Will access r2 * 16 * 4 bytes (this must be lower than RAM size)

	// Address start
#ifndef NO_OUTER_LOOP
	mov r0, r5
	// Loop number according to bytes tested per loop
#ifdef USE_MVE_LOADS
	mov r2, r7, lsr 6
#else
	mov r2, r7, lsr 7
#endif
#else
	// Loop number according to bytes tested per loop
#ifdef USE_MVE_LOADS
	mov r2, r1, lsr 6
#else
	mov r2, r1, lsr 7
#endif
#endif


	dls	lr, r2
.p2align 2
	loop1:
#ifdef USE_MVE_LOADS
		vldrw.u32 q0, [r0], #16
		vldrw.u32 q0, [r0], #16
		vldrw.u32 q0, [r0], #16
		vldrw.u32 q0, [r0], #16
#else
		// Read just one word every 32 bytes to trigger a cache linefill every time
		// To check memory bandwidth, check L1 D-Cache refill counter and multiply by 32 bytes.
		ldr r1, [r0], #32
		ldr r2, [r0], #32
		ldr r3, [r0], #32
		ldr r4, [r0], #32
#endif
	le lr, loop1

#ifndef NO_OUTER_LOOP
	subs r6, r6, #1
	bne loop2
	pop.w	{r4, r5, r6, r7, pc}
#else
	pop.w	{pc}
#endif

  .size memory_benchmark_single, .-memory_benchmark_single
