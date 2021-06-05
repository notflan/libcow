#pragma once

template<typename T>
struct Slice {
	virtual const void* area() const =0;
	virtual void* area() = 0;
	virtual size_t size() const = 0;

	inline T* ptr() { return (T*)area(); }
	inline const T* ptr() const { return (const T*)area(); }

	inline size_t size_bytes() const { return size() * sizeof(T); }

	inline unsigned char* as_bytes() { return (unsigned char*)area(); }
	inline const unsigned char* as_bytes() const { return (const unsigned char*)area(); }

	inline T& operator[](size_t index) {
		if(index >= size()) throw "Size too large";
		return ptr()[index];
	}
	inline const T& operator[](size_t index) const {
		if(index >= size()) throw "Size too large";
		return ptr()[index];
	}

	template<typename U>
	inline U* area_as() requires(sizeof(T) % sizeof(U) == 0) { return (U*)area(); }
	template<typename U>
	inline const U* area_as() const requires(sizeof(T) % sizeof(U) == 0) { return (U*)area(); }

	template<typename U>
	size_t size_as() const requires(sizeof(T) % sizeof(U) == 0) { return size_bytes() / sizeof(U); }
};
