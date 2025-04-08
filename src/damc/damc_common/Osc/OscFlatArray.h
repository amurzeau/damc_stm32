#pragma once

#include "OscContainer.h"

template<typename T> class OscFlatArray : protected OscContainer {
public:
	OscFlatArray(OscContainer* parent, std::string_view name, bool persistValue = true) noexcept;

	void reserve(size_t reserveSize);
	template<class U> bool updateData(const U& lambda, bool fromOsc = false);
	const std::vector<T>& getData() const;
	bool setData(const std::vector<T>& newData);
	bool setData(std::vector<T>&& newData);

	void dump() override { notifyOsc(); }

	std::string getAsString() const override;

	void execute(const std::vector<OscArgument>& arguments) override;

	void addCheckCallback(std::function<bool(const std::vector<T>&)> checkCallback);
	void addChangeCallback(std::function<void(const std::vector<T>&, const std::vector<T>&)> onChange);

	bool callCheckCallbacks(const std::vector<T>& v);

	void visit(const std::function<void(OscNode*, OscArgument*, size_t)>& nodeVisitorFunction) override;

private:
	using OscContainer::execute;

protected:
	void notifyOsc();
	bool checkData(bool fromOsc);

private:
	std::vector<T> values;
	std::vector<T> savedValues;
	std::vector<std::function<void(const std::vector<T>&, const std::vector<T>&)>> onChangeCallbacks;
	std::vector<std::function<bool(const std::vector<T>&)>> checkCallbacks;
};

EXPLICIT_INSTANCIATE_OSC_VARIABLE(extern template, OscFlatArray);

template<class T> template<class U> bool OscFlatArray<T>::updateData(const U& lambda, bool fromOsc) {
	savedValues = values;
	lambda(values);
	return checkData(fromOsc);
}
