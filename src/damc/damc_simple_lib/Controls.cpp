#include "Controls.h"
#include "usbd_audio.h"
#include <OscRoot.h>
#include <atomic>

Controls* Controls::instance;

Controls::Controls(OscRoot* oscRoot) : oscRoot(oscRoot), processUsbControlChange(false) {
	uv_async_init(uv_default_loop(), &asyncControlChanged, Controls::onControlChangedStatic);
	asyncControlChanged.data = this;
}

void Controls::init() {
	static const char* controlNodeAddresses[] = {
	    "strip/0",
	    "strip/2",
	    "strip/1",
	};

	static_assert(sizeof(controlNodeAddresses) / sizeof(controlNodeAddresses[0]) ==
	                  std::tuple_size<decltype(controlsMapping)>::value,
	              "bad controlsMapping size");

	for(size_t i = 0; i < controlsMapping.size(); i++) {
		OscNode* baseNode = oscRoot->getNode(controlNodeAddresses[i]);
		controlsMapping[i].volumeControl = (OscReadOnlyVariable<float>*) baseNode->getNode("filterChain/volume");
		controlsMapping[i].muteControl = (OscReadOnlyVariable<bool>*) baseNode->getNode("filterChain/mute");
		controlsMapping[i].volumeToSet.isChanged = false;

		controlsMapping[i].volumeControl->addChangeCallback([this, i](float) {
			if(!processUsbControlChange)
				USBD_AUDIO_NotifyUnitIdChanged(i * AUDIO_UNIT_ID_PER_ENDPOINT + AUDIO_UNIT_ID_OFFSET_FEATURE_UNIT,
				                               AUDIO_CONTROL_VOLUME);
		});
		controlsMapping[i].muteControl->addChangeCallback([this, i](bool) {
			if(!processUsbControlChange)
				USBD_AUDIO_NotifyUnitIdChanged(i * AUDIO_UNIT_ID_PER_ENDPOINT + AUDIO_UNIT_ID_OFFSET_FEATURE_UNIT,
				                               AUDIO_CONTROL_MUTE);
		});
	}

	instance = this;
}

static void USBControlWrite8(size_t& size, uint8_t* data, int8_t value) {
	data[size] = value;
	size++;
}

static void USBControlWrite16(size_t& size, uint8_t* data, int16_t value) {
	data[size] = value & 0xFF;
	size++;
	data[size] = value >> 8;
	size++;
}

static void USBControlWrite32(size_t& size, uint8_t* data, int32_t value) {
	data[size] = value & 0xFF;
	size++;
	data[size] = (value >> 8) & 0xFF;
	size++;
	data[size] = (value >> 16) & 0xFF;
	size++;
	data[size] = (value >> 24) & 0xFF;
	size++;
}

static void USBControlWriteRange8(size_t& size, uint8_t* data, int8_t min, int8_t max, int8_t res) {
	USBControlWrite16(size, data, 1);
	USBControlWrite8(size, data, min);
	USBControlWrite8(size, data, max);
	USBControlWrite8(size, data, res);
}

static void USBControlWriteRange16(size_t& size, uint8_t* data, int16_t min, int16_t max, int16_t res) {
	USBControlWrite16(size, data, 1);
	USBControlWrite16(size, data, min);
	USBControlWrite16(size, data, max);
	USBControlWrite16(size, data, res);
}

static void USBControlWriteRange32(size_t& size, uint8_t* data, int32_t min, int32_t max, int32_t res) {
	USBControlWrite16(size, data, 1);
	USBControlWrite32(size, data, min);
	USBControlWrite32(size, data, max);
	USBControlWrite32(size, data, res);
}

volatile int error_control = 0;
size_t Controls::getControlFromUSB(
    uint8_t unit_id, uint8_t control_selector, uint8_t channel, uint8_t bRequest, uint8_t* data) {
	size_t size = 0;

	uint8_t endpoint_index = (unit_id - 1) / AUDIO_UNIT_ID_PER_ENDPOINT;
	uint8_t unit_id_offset = (unit_id - 1) % AUDIO_UNIT_ID_PER_ENDPOINT + 1;

	if(endpoint_index >= controlsMapping.size())
		return 0;

	switch(unit_id_offset) {
		case AUDIO_UNIT_ID_OFFSET_FEATURE_UNIT:
			switch(control_selector) {
				case AUDIO_CONTROL_VOLUME:
					switch(bRequest) {
						case AUDIO_REQ_CUR:
							USBControlWrite16(
							    size, data, (int16_t) controlsMapping[endpoint_index].volumeControl->getToOsc() * 256);
							break;
						case AUDIO_REQ_RANGE:
							USBControlWriteRange16(
							    size, data, (int16_t) (-127 * 256), (int16_t) (20 * 256), (int16_t) (1 * 256));
							break;
					}
					break;

				case AUDIO_CONTROL_MUTE:
					switch(bRequest) {
						case AUDIO_REQ_CUR:
							USBControlWrite8(size, data, controlsMapping[endpoint_index].muteControl->get() ? 1 : 0);
							break;
						case AUDIO_REQ_RANGE:
							USBControlWriteRange8(size, data, 0, 1, 1);
							break;
					}
					break;
			}
			break;
		case AUDIO_UNIT_ID_OFFSET_CLOCK_SOURCE:
			switch(control_selector) {
				case AUDIO_CONTROL_SAM_FREQ_CONTROL:
					switch(bRequest) {
						case AUDIO_REQ_CUR:
							USBControlWrite32(size, data, 48000);
							break;
						case AUDIO_REQ_RANGE:
							USBControlWriteRange32(size, data, 48000, 48000, 0);
							break;
					}
					break;
			}
			break;
	}

	return size;
}

void Controls::setControlFromUSB(
    uint8_t unit_id, uint8_t control_selector, uint8_t channel, uint8_t bRequest, uint16_t value) {
	uint8_t endpoint_index = (unit_id - 1) / AUDIO_UNIT_ID_PER_ENDPOINT;
	uint8_t unit_id_offset = (unit_id - 1) % AUDIO_UNIT_ID_PER_ENDPOINT + 1;
	if(endpoint_index >= controlsMapping.size()) {
		error_control++;
		return;
	}

	if(bRequest != AUDIO_REQ_CUR) {
		error_control++;
		return;
	}

	switch(unit_id_offset) {
		case AUDIO_UNIT_ID_OFFSET_FEATURE_UNIT:
			switch(control_selector) {
				case AUDIO_CONTROL_VOLUME:
					controlsMapping[endpoint_index].volumeToSet.value = (int16_t) value / 256.f;
					controlsMapping[endpoint_index].volumeToSet.isChanged = true;
					uv_async_send(&asyncControlChanged);
					break;

				case AUDIO_CONTROL_MUTE:
					controlsMapping[endpoint_index].muteToSet.value = value == 0 ? false : true;
					controlsMapping[endpoint_index].muteToSet.isChanged = true;
					uv_async_send(&asyncControlChanged);
					break;
			}
			break;
		case AUDIO_UNIT_ID_OFFSET_CLOCK_SOURCE:
			switch(control_selector) {
				case AUDIO_CONTROL_SAM_FREQ_CONTROL:
					// Setting sampling rate not implemented, only 48Khz supported.
					break;
			}
			break;
	}
}
void Controls::onControlChangedStatic(uv_async_t* handle) {
	Controls* thisInstance = (Controls*) handle->data;
	thisInstance->onControlChanged();
}

void Controls::onControlChanged() {
	for(size_t i = 0; i < controlsMapping.size(); i++) {
		if(controlsMapping[i].volumeToSet.isChanged) {
			controlsMapping[i].volumeToSet.isChanged = false;

			// Ensure isChanged is set to false before reading value
			std::atomic_signal_fence(std::memory_order_seq_cst);

			processUsbControlChange = true;
			controlsMapping[i].volumeControl->setFromOsc(controlsMapping[i].volumeToSet.value);
			processUsbControlChange = false;
		}
		if(controlsMapping[i].muteToSet.isChanged) {
			controlsMapping[i].muteToSet.isChanged = false;

			// Ensure isChanged is set to false before reading value
			std::atomic_signal_fence(std::memory_order_seq_cst);

			processUsbControlChange = true;
			controlsMapping[i].muteControl->setFromOsc(controlsMapping[i].muteToSet.value);
			processUsbControlChange = false;
		}
	}
}
