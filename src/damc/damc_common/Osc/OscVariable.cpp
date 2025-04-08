#include "OscVariable.h"
#include "OscRoot.h"
#include <spdlog/spdlog.h>

template<typename T>
OscVariable<T>::OscVariable(OscContainer* parent,
                            std::string_view name,
                            readonly_type initialValue,
                            bool persistValue) noexcept
    : OscReadOnlyVariable<T>(parent, name, initialValue) {
	if(persistValue) {
		this->getRoot()->addPendingConfigNode(this);

		this->addChangeCallback([this](readonly_type) { this->getRoot()->notifyValueChanged(); });
	}

	if constexpr(std::is_same_v<T, bool>) {
		subEndpoint.emplace_back(new OscEndpoint(this, "toggle"))->setCallback([this](auto) {
			SPDLOG_INFO("{}: Toggling", this->getFullAddress());
			this->setFromOsc(!this->getToOsc());
		});
	} else if constexpr(std::is_same_v<T, std::string>) {
		// No toggle/increment/decrement
	} else {
		incrementAmount = (T) 1;
		subEndpoint.reserve(2);
		subEndpoint.emplace_back(new OscEndpoint(this, "increment"))
		    ->setCallback([this](const std::vector<OscArgument>& arguments) {
			    T amount = incrementAmount;

			    if(!arguments.empty()) {
				    OscNode::getArgumentAs<T>(arguments[0], amount);
			    }

			    SPDLOG_INFO("{}: Incrementing by {}", this->getFullAddress(), amount);
			    this->setFromOsc(this->getToOsc() + amount);
		    });
		subEndpoint.emplace_back(new OscEndpoint(this, "decrement"))
		    ->setCallback([this](const std::vector<OscArgument>& arguments) {
			    T amount = incrementAmount;

			    if(!arguments.empty()) {
				    OscNode::getArgumentAs<T>(arguments[0], amount);
			    }

			    SPDLOG_INFO("{}: Decrementing by {}", this->getFullAddress(), amount);
			    this->setFromOsc(this->getToOsc() - amount);
		    });
	}
}

template<typename T> OscVariable<T>& OscVariable<T>::operator=(const OscVariable<T>& v) {
	this->set(v.get());
	return *this;
}

template<typename T> void OscVariable<T>::execute(const std::vector<OscArgument>& arguments) {
	if(!arguments.empty()) {
		typename OscReadOnlyVariable<T>::readonly_type v;
		if(this->template getArgumentAs<typename OscReadOnlyVariable<T>::readonly_type>(arguments[0], v)) {
			this->setFromOsc(std::move(v));
		}
	}
}

template<typename T> std::string OscVariable<T>::getAsString() const {
	if(this->isDefault())
		return {};

	if constexpr(std::is_same_v<T, std::string>) {
		return "\"" + std::string(this->getToOsc()) + "\"";
	} else {
		return std::to_string(this->getToOsc());
	}
}

template<typename T>
void OscVariable<T>::visit(const std::function<void(OscNode*, OscArgument*, size_t)>& nodeVisitorFunction) {
	OscArgument valueToSend = this->getToOsc();
	nodeVisitorFunction(this, &valueToSend, 1);
}

EXPLICIT_INSTANCIATE_OSC_VARIABLE(template, OscVariable)
