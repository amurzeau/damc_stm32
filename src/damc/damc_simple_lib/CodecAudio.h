#pragma once

#include "CircularBuffer.h"
#include "CodecDamcHATInit.h"
#include "CodecSTM32F723EDiscoInit.h"
#include <stdint.h>

class CodecHardwareInterface;

class CodecAudio {
public:
	struct CodecFrame {
		int32_t headphone[2];
	};

	CodecAudio();

	void start();

	void processAudioInterleavedOutput(const CodecFrame* data_input, size_t nframes);
	void processAudioInterleavedInput(CodecFrame* data_output, size_t nframes);

	/** @brief Get DMA position in samples unit.
	 * Return the position of the hardware DMA read pointer in the buffer.
	 */
	uint32_t getDMAPos();

	/** @brief Check if the audio processing interrupt is pending.
	 * @param insertWaitStates true to insert a SAI peripheral dummy read before reading ISR
	 * @return true if the DMA ISR flag is set
	 */
	bool isAudioProcessingInterruptPending(bool insertWaitStates);

	void setMicBias(bool enable);

	static CodecAudio instance;

protected:
	void writeOutBuffer(const uint32_t* data, size_t word_size);
	void readInBuffer(uint32_t* data, size_t word_size);

private:
	struct CodecBuffers {
		CircularBuffer<CodecFrame, 2, true> out_buffer;
		CircularBuffer<CodecFrame, 2, true> in_buffer;
	};

	CodecBuffers codecBuffers;

	CodecHardwareInterface* codecHardwareInterface;
	CodecSTM32F723EDiscoInit codecSTM32F723EDiscoInit;
	CodecDamcHATInit codecDamcHATInit;

	uint32_t previousAvailableDmaIn;
	uint32_t previousAvailableDmaOut;
};
