#pragma once

#include "CircularBuffer.h"

template<typename T, int N> class CircularQueue : public CircularBuffer<T, N, false> {
public:
	size_t write(const T& data) { return this->writeOutBuffer(this->getReadPos(), &data, 1); }
	size_t read(T* data) { return this->readInBuffer(this->getWritePos(), data, 1); }
};
