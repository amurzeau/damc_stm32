#include "CodecAudio.h"
#include "GlitchDetection.h"
#include <spdlog/spdlog.h>

#include <assert.h>
#include <math.h>
#include <string.h>

CodecAudio CodecAudio::instance;

CodecAudio::CodecAudio() : previousAvailableDmaIn(0), previousAvailableDmaOut(0) {}

void CodecAudio::start() {
	codecDamcHATInit.init();

	if(codecDamcHATInit.isAvailable()) {
		codecHardwareInterface = &codecDamcHATInit;
	} else {
		codecHardwareInterface = &codecSTM32F723EDiscoInit;
	}

	codecHardwareInterface->start(
	    codecBuffers.in_buffer.getBuffer(), codecBuffers.out_buffer.getBuffer(), codecBuffers.out_buffer.getSize());
}

volatile uint32_t diff_dma_out;
void CodecAudio::processAudioInterleavedOutput(const CodecFrame* data_input, size_t nframes) {
	uint16_t dma_read_offset = getDMAPos();
	uint32_t availableForDma = codecBuffers.out_buffer.getAvailableReadForDMA(dma_read_offset);
	diff_dma_out = availableForDma;

	size_t writtenSize = codecBuffers.out_buffer.writeOutBuffer(dma_read_offset, data_input, nframes);

	// Check Overrun
	if(writtenSize != nframes) {
		GLITCH_DETECTION_increment_counter(GT_CodecOutXRun);
	}

	// Check DMA Underrun
	if(previousAvailableDmaOut < nframes && availableForDma > (nframes + nframes / 2)) {
		GLITCH_DETECTION_increment_counter(GT_CodecOutDmaUnderrun);
	}
	previousAvailableDmaOut = availableForDma;
}

volatile uint32_t diff_dma_in;
void CodecAudio::processAudioInterleavedInput(CodecFrame* data_output, size_t nframes) {
	uint16_t dma_write_offset = getDMAPos();
	uint32_t availableForDma = codecBuffers.in_buffer.getAvailableWriteForDMA(dma_write_offset);
	diff_dma_in = availableForDma;
	size_t readSize = codecBuffers.in_buffer.readInBuffer(dma_write_offset, data_output, nframes);

	// Check Underrun
	if(readSize != nframes) {
		GLITCH_DETECTION_increment_counter(GT_CodecInXRun);
	}

	// Check DMA Overrun
	if(previousAvailableDmaIn < nframes && availableForDma > (nframes + nframes / 2)) {
		GLITCH_DETECTION_increment_counter(GT_CodecInDmaOverrun);
	}
	previousAvailableDmaIn = availableForDma;
}

uint32_t CodecAudio::getDMAPos() {
	uint32_t dma_pos;

	dma_pos = codecHardwareInterface->getDmaRemainingCount();

	if(dma_pos == 0) {
		return 0;
	}

	uint16_t dma_read_offset = codecBuffers.out_buffer.getCount() - ((dma_pos + 1) / 2);
	return dma_read_offset;
}

bool CodecAudio::isAudioProcessingInterruptPending(bool insertWaitStates) {
	return codecHardwareInterface->isDMAIsrFlagSet(insertWaitStates);
}

void CodecAudio::setMicBias(bool enable) {
	codecHardwareInterface->setMicBias(enable);
}