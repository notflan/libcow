#pragma once

namespace _cow_util {
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
			if(index >= size()) throw "index out of range";
			return ptr()[index];
		}
		inline const T& operator[](size_t index) const {
			if(index >= size()) throw "index out of range";
			return ptr()[index];
		}

		inline T* operator&() { return &(*this)[0]; }
		inline const T* operator&() const {return &(*this)[0]; }

		inline T* operator->() { return &(*this)[0]; }
		inline const T* operator->() const {return &(*this)[0]; }

		inline T& operator*() { return (*this)[0]; }
		inline const T& operator*() const { return (*this)[0]; }

		inline operator const T*() const { return &(*this)[0]; }
		inline operator T*() { return &(*this)[0]; }

		template<typename U>
		inline U* area_as() requires(sizeof(T) % sizeof(U) == 0) { return (U*)area(); }
		template<typename U>
		inline const U* area_as() const requires(sizeof(T) % sizeof(U) == 0) { return (U*)area(); }

		template<typename U>
		size_t size_as() const requires(sizeof(T) % sizeof(U) == 0) { return size_bytes() / sizeof(U); }

		struct Slice;

		inline operator Slice() { return Slice(ptr(), size()); }
		inline operator const Slice() const { return Slice(const_cast<T*>(ptr()), size()); }

		inline bool bounds_ok(size_t start) const
		{
			return start < size();
		}
		inline bool bounds_ok(size_t start, size_t len) const
		{
			return (start + len) <= size() && bounds_ok(start);
		}

		inline ssize_t wrap_len(ssize_t len) const
		{
			if(size() ==0 ) return 0;
			return len < 0 ? wrap_len(size() + len) : ((size_t)len) % size();
		}

		/// Slice (absolute). Specify start and end.
		inline const Slice slice_abs(size_t start, size_t end) const { auto len = end -start; if(bounds_ok(start,len)) return Slice(const_cast<T*>(ptr()+start), len); else throw "Out of bounds slice"; }
		inline Slice slice_abs(size_t start, size_t end) { auto len = end -start; if(bounds_ok(start,len)) return Slice(ptr()+start, len); else throw "Out of bounds slice"; }

		/// Slice (relative). Specify start and length.
		inline const Slice slice(size_t start, size_t len) const { if(bounds_ok(start,len)) return Slice(const_cast<T*>(ptr()+start), len); else throw "Out of bounds slice"; }
		inline Slice slice(size_t start, size_t len) { if(bounds_ok(start,len)) return Slice(ptr()+start, len); else throw "Out of bounds slice"; }
		

		/// Slice from 0. Specify length.
		inline Slice slice(size_t len) { return slice(0, len); }
		inline const Slice slice(size_t len) const { return slice(0, len); }//slice(start, size()-start); }

		/// Slice total.
		inline Slice slice() { return slice(0, size()); }
		inline const Slice slice() const { return slice(0, size()); }

		/// Slice wrapping. Specify start and end that may wrap over and/or under the span's size.
		inline Slice slice_wrap(ssize_t start, ssize_t end) { return slice_abs((size_t)wrap_len(start), (size_t)wrap_len(end)); }
		inline const Slice slice_wrap(ssize_t start, ssize_t end) const { return slice_abs((size_t)wrap_len(start), (size_t)wrap_len(end)); }
		inline Slice slice_wrap(ssize_t len) { return slice_abs((size_t)wrap_len(len)); }
		inline const Slice slice_wrap(ssize_t len) const { return slice_abs((size_t)wrap_len(len)); }

		template<typename U>
		inline Span<U>::Slice reinterpret() { return typename Span<U>::Slice((U*)area(), size_bytes() / sizeof(U)); }	
		template<typename U>
		inline Span<const U>::Slice reinterpret() const { return typename Span<const U>::Slice((const U*)area(), size_bytes() / sizeof(U)); }
	};

	/// A slice of memory with a backing pointer and size.
	template<typename T>
	struct Span<T>::Slice : public Span<T> {
		inline Slice(T* ptr, size_t sz) : _area((void*)ptr), _size(sz){}
		inline Slice(const Span<T>& slice) : _area(const_cast<void*>(slice.area())), _size(slice.size()){}
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
	using Slice = Span<T>::Slice;
}
