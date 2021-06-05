#pragma once

/// A type that spans a sized region of memory
template<typename T>
struct Span {
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

	struct Slice;

	inline bool bounds_ok(size_t start, size_t len)
	{
		return (start + len) < size();
	}
	inline bool bounds_ok(size_t start)
	{
		return start < size();
	}

	inline Slice slice(size_t start, size_t len) { if(bounds_ok(start,len)) return Slice(ptr()+start, len); else throw "Out of bounds slice"; }
	inline Slice slice(size_t start) { return slice(start, size()-start); }
};

/// A slice of memory with a backing pointer and size.
template<typename T>
struct Span<T>::Slice : public Span<T> {
	inline Slice(T* ptr, size_t sz) : _area((void*)ptr), _size(sz){}
	inline Slice(const Span<T>& slice) : _area(slice.area()), _size(slice.size()){}
	inline Slice(const Slice& copy) = default;
	inline Slice(Slice&& copy) : _area(copy._area), _size(copy._size){ *const_cast<size_t*>(&copy._size) = 0; }
	Slice() = delete;

	inline const void* area() const override { return _area; }
	inline void* area() override { return _area; }
	inline size_t size() const override { return _size; }
	
	private:
	void* const _area;
	const size_t _size;
};

template<typename T>
typename Span<T>::Slice Slice;
