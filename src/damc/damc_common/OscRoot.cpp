#include "OscRoot.h"
#include "tinyosc.h"
#include <float.h>
#include <math.h>
#include <spdlog/spdlog.h>
#include <string.h>
#include <string_view>

OscRoot::OscRoot(bool notifyAtInit) : OscContainer(nullptr, ""), doNotifyOscAtInit(notifyAtInit) {
	oscOutputMaxSize = 128;
	oscOutputMessage.reset(new uint8_t[oscOutputMaxSize]);
	sendSerializedMessage = [this](std::string_view nodeFullAddress, uint8_t* data, size_t size) {
		for(OscConnector* connector : connectors) {
			connector->sendOscMessage(data, size);
		}
	};
}

OscRoot::~OscRoot() {}

void OscRoot::printAllNodes() {
	SPDLOG_INFO("Nodes:\n{}", getAsString().c_str());
}

void OscRoot::serializeMessage(const std::function<void(std::string_view, uint8_t*, size_t)>& processResult,
                               const OscNode* node,
                               const OscArgument* arguments,
                               size_t number) {
	tosc_message osc;
	char format[256] = ",";
	char* formatPtr = format + 1;

	node->getFullAddress(&nodeFullAddress);

	if(number > sizeof(format) - 2) {
		SPDLOG_ERROR("Too many arguments, can't send OSC message: {}", number);
		return;
	}

	for(size_t i = 0; i < number; i++) {
		const OscArgument& argument = arguments[i];
		std::visit(
		    [&formatPtr](auto&& arg) -> void {
			    using U = std::decay_t<decltype(arg)>;
			    if constexpr(std::is_same_v<U, bool>) {
				    *formatPtr++ = arg ? 'T' : 'F';
			    } else if constexpr(std::is_same_v<U, int32_t>) {
				    *formatPtr++ = 'i';
			    } else if constexpr(std::is_same_v<U, float>) {
				    *formatPtr++ = 'f';
			    } else if constexpr(std::is_same_v<U, std::string_view>) {
				    *formatPtr++ = 's';
			    } else {
				    static_assert(always_false_v<U>, "Unhandled type");
			    }
		    },
		    argument);
	}
	*formatPtr++ = '\0';

	if(tosc_writeMessageHeader(
	       &osc, nodeFullAddress.c_str(), format, (char*) oscOutputMessage.get(), oscOutputMaxSize) != 0) {
		SPDLOG_ERROR("failed to write OSC message");
		return;
	}

	for(size_t i = 0; i < number; i++) {
		const OscArgument& argument = arguments[i];
		uint32_t result = 0;
		std::visit(
		    [&osc, &result](auto&& arg) -> void {
			    using U = std::decay_t<decltype(arg)>;
			    if constexpr(std::is_same_v<U, bool>) {
				    // No argument to write
			    } else if constexpr(std::is_same_v<U, int32_t>) {
				    result = tosc_writeNextInt32(&osc, arg);
			    } else if constexpr(std::is_same_v<U, float>) {
				    result = tosc_writeNextFloat(&osc, arg);
			    } else if constexpr(std::is_same_v<U, std::string_view>) {
				    result = tosc_writeNextStringView(&osc, arg.data(), arg.size());
			    } else {
				    static_assert(always_false_v<U>, "Unhandled type");
			    }
		    },
		    argument);

		if(result != 0) {
			SPDLOG_ERROR("failed to write OSC value {} in message", i);
			return;
		}
	}

	processResult(nodeFullAddress, oscOutputMessage.get(), tosc_getMessageLength(&osc));
}

void OscRoot::sendMessage(const OscNode* node, const OscArgument* arguments, size_t number) {
	SPDLOG_TRACE("Sending OSC message {} {}", nodeFullAddress, getArgumentVectorAsString(arguments, number));
	serializeMessage(sendSerializedMessage, node, arguments, number);
}

void OscRoot::loadNodeConfig(const std::map<std::string_view, std::vector<OscArgument>>& configValues) {
	SPDLOG_DEBUG("Traversing OscNode to assign configuration values");

	std::string nodeAddress;

	while(!nodesPendingConfig.empty()) {
		auto nextNodeIt = nodesPendingConfig.begin();
		OscNode* node = *nextNodeIt;
		nodesPendingConfig.erase(nextNodeIt);

		node->getFullAddress(&nodeAddress);

		auto it = configValues.find(nodeAddress);
		if(it != configValues.end()) {
			node->execute(it->second);
		}
	}
}

void OscRoot::saveNodeConfig(const std::function<void(OscNode*, OscArgument*, size_t)>& nodeVisitorFunction) {
	SPDLOG_DEBUG("Traversing OscNode to save configuration values");

	visit(nodeVisitorFunction);
}

std::string OscRoot::getArgumentVectorAsString(const OscArgument* arguments, size_t number) {
	using namespace std::literals;

	std::string result = "[";

	for(size_t i = 0; i < number; i++) {
		std::visit(
		    [&result](auto&& arg) -> void {
			    if constexpr(std::is_same_v<decltype(arg), const std::string_view&>) {
				    result.append(" "sv);
				    result.append(arg);
				    result.append(","sv);
			    } else {
				    result += " " + std::to_string(arg) + ",";
			    }
		    },
		    arguments[i]);
	}

	if(result.back() == ',')
		result.pop_back();

	return result + " ]";
}

void OscRoot::onOscPacketReceived(const uint8_t* data, size_t size) {
	if(tosc_isBundle((const char*) data)) {
		tosc_bundle_const bundle;
		tosc_parseBundle(&bundle, (const char*) data, size);

		tosc_message_const osc;
		while(tosc_getNextMessage(&bundle, &osc)) {
			executeMessage(&osc);
		}
	} else {
		tosc_message_const osc;
		int result = tosc_parseMessage(&osc, (const char*) data, size);
		if(result == 0) {
			executeMessage(&osc);
		}
	}
}

void OscRoot::executeMessage(tosc_message_const* osc) {
	std::vector<OscArgument> arguments;
	const char* address = tosc_getAddress(osc);

	for(int i = 0; osc->format[i] != '\0'; i++) {
		OscArgument argument;

		switch(osc->format[i]) {
			case 's':
				argument = std::string_view(tosc_getNextString(osc));
				break;
			case 'f':
				argument = tosc_getNextFloat(osc);
				break;
			case 'i':
				argument = tosc_getNextInt32(osc);
				break;
			case 'F':
				argument = false;
				break;
			case 'I':
				argument = FLT_MAX;
				break;
			case 'T':
				argument = true;
				break;

				// Unsupported formats
			case 'N':
				SPDLOG_ERROR(" Unsupported format: {}", osc->format[i]);
				break;
			case 'b': {
				const char* b = nullptr;  // will point to binary data
				int n = 0;                // takes the length of the blob
				tosc_getNextBlob(osc, &b, &n);
				SPDLOG_ERROR(" Unsupported format: {}", osc->format[i]);
				break;
			}
			case 'm':
				tosc_getNextMidi(osc);
				SPDLOG_ERROR(" Unsupported format: {}", osc->format[i]);
				break;
			case 'd':
				tosc_getNextDouble(osc);
				SPDLOG_ERROR(" Unsupported format: {}", osc->format[i]);
				break;
			case 'h':
				tosc_getNextInt64(osc);
				SPDLOG_ERROR(" Unsupported format: {}", osc->format[i]);
				break;
			case 't':
				tosc_getNextTimetag(osc);
				SPDLOG_ERROR(" Unsupported format: {}", osc->format[i]);
				break;

			default:
				SPDLOG_ERROR(" Unsupported format: {}", osc->format[i]);
				break;
		}

		arguments.push_back(std::move(argument));
	}

	if(strstr(address, "meter") == nullptr) {
		tosc_reset(osc);
		SPDLOG_DEBUG("OSC message received: {} {} {}",
		             address,
		             osc->format,
		             getArgumentVectorAsString(&arguments[0], arguments.size()));
	}
	execute(address + 1, std::move(arguments));
}

OscRoot* OscRoot::getRoot() {
	return this;
}

bool OscRoot::isOscValueAuthority() {
	return doNotifyOscAtInit;
}

void OscRoot::notifyValueChanged() {
	for(auto& callback : onOscValueChanged) {
		callback();
	}
}

void OscRoot::addPendingConfigNode(OscNode* node) {
	SPDLOG_DEBUG("Adding node {} as pending configuration", node->getFullAddress());
	nodesPendingConfig.insert(node);
}

void OscRoot::nodeRemoved(OscNode* node) {
	nodesPendingConfig.erase(node);
}

void OscRoot::triggerAddress(const std::string_view& address) {
	execute(std::string_view{address.data() + 1, address.size() - 1}, std::vector<OscArgument>{});
}

void OscRoot::addConnector(OscConnector* connector) {
	connectors.insert(connector);
}

void OscRoot::removeConnector(OscConnector* connector) {
	connectors.erase(connector);
}

void OscRoot::addValueChangedCallback(std::function<void()> onOscValueChanged) {
	this->onOscValueChanged.push_back(onOscValueChanged);
}

static constexpr uint8_t SLIP_END = 0xC0;
static constexpr uint8_t SLIP_ESC = 0xDB;
static constexpr uint8_t SLIP_ESC_END = 0xDC;
static constexpr uint8_t SLIP_ESC_ESC = 0xDD;

OscConnector::OscConnector(OscRoot* oscRoot, bool useSlipProtocol)
    : oscRoot(oscRoot), useSlipProtocol(useSlipProtocol), oscIsEscaping(false), discardNextMessage(false) {
	if(oscRoot)
		oscRoot->addConnector(this);

	oscInputBuffer.reserve(128);
	oscOutputBuffer.reserve(128);
}

OscConnector::~OscConnector() {
	if(oscRoot)
		oscRoot->removeConnector(this);
}

void OscConnector::sendOscMessage(const uint8_t* data, size_t size) {
	if(useSlipProtocol) {
		oscOutputBuffer.clear();
		oscOutputBuffer.reserve(size + 2 + 10);

		// Encode SLIP frame with double-END variant (one at the start, one at the end)
		oscOutputBuffer.push_back(SLIP_END);

		for(size_t i = 0; i < size; i++) {
			const uint8_t c = data[i];

			if(c == SLIP_END) {
				oscOutputBuffer.push_back(SLIP_ESC);
				oscOutputBuffer.push_back(SLIP_ESC_END);
			} else if(c == SLIP_ESC) {
				oscOutputBuffer.push_back(SLIP_ESC);
				oscOutputBuffer.push_back(SLIP_ESC_ESC);
			} else {
				oscOutputBuffer.push_back(c);
			}
		}

		oscOutputBuffer.push_back(SLIP_END);

		sendOscData(oscOutputBuffer.data(), oscOutputBuffer.size());
	} else {
		sendOscData(data, size);
	}
}

void OscConnector::onOscDataReceived(const uint8_t* data, size_t size) {
	if(oscRoot == nullptr)
		return;

	if(useSlipProtocol) {
		// Decode SLIP frame
		for(size_t i = 0; i < size; i++) {
			uint8_t c = data[i];

			if(!oscIsEscaping) {
				if(c == SLIP_ESC) {
					oscIsEscaping = true;
					continue;
				} else if(c == SLIP_END) {
					if(!oscInputBuffer.empty()) {
						if(!discardNextMessage)
							oscRoot->onOscPacketReceived(oscInputBuffer.data(), oscInputBuffer.size());
						oscInputBuffer.clear();
						discardNextMessage = false;
					}
					continue;
				}
				// else this is a regular character
			} else {
				if(c == SLIP_ESC_END) {
					c = SLIP_END;
				} else if(c == SLIP_ESC_ESC) {
					c = SLIP_ESC;
				}
				// else this is an error, escaped character doesn't need to be escaped
			}
			oscIsEscaping = false;
			if(oscInputBuffer.size() < 128)
				oscInputBuffer.push_back(c);
			else
				discardNextMessage = true;
		}
	} else {
		oscRoot->onOscPacketReceived(data, size);
	}
}
