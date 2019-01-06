#pragma once
#include <vector>
#include <assert.h>
template <typename T>
class Ring {
private:
	std::vector<T> buffer;
	int maxsize;
	int i;
	int overflows;
public:
	Ring(int maxsize = 3) : maxsize(maxsize), i(0), overflows(0) {
		assert(maxsize > 0 && "Size must be > 0");
		buffer.resize(maxsize);
	}
	Ring(std::vector<T> & vec) : Ring(vec.size()) {
		buffer = vec;
	}
	Ring(std::initializer_list<T> & list) : Ring(list.size()) {
		buffer.insert(buffer.end(), list.begin(), list.end());
	}
	Ring& operator=(std::vector<T> & vec) {
		assert(vec.size() == maxsize && "Ringbuffer assignment size mismatch");
		buffer = vec;
		return *this;
	}
	void resize(int maxsize) {
		assert(maxsize > 0 && "Size must be > 0");
		this->maxsize = maxsize;
		buffer.resize(maxsize);
	}
	void add(const T element) noexcept {
		i = i == maxsize ? (++overflows, 0) : i;
		buffer[i++] = element;
	}
	T operator[](int i) const noexcept {
		if (i >= maxsize) i %= maxsize;
		return buffer[i];
	}
	/**
	 * Gets the amount of data actually used in the buffer
	 * @return pair in which first element indicates how many loops were taken, second indicates usage of the buffer. if first > 0 then second == maxsize
	*/
	std::pair<int, int> size() {
		if (overflows)
			return std::make_pair(overflows, maxsize);
		return std::make_pair(0, i);
	}

	/**
	 * Adds up corresponding elements of an equal sized Ringbuffer
	*/
	Ring<T>& operator+=(const Ring<T> & other) {
		assert(maxsize == other.maxsize && "Ringbuffer size must be equal!");
		for (int i = 0; i < maxsize; ++i)
			buffer[i] += other.buffer[i];
		return *this;
	}
	std::vector<T> vector() { return buffer; }
};