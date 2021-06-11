#pragma once

#include <cow.hpp>
#include <utility>

template<typename T>
struct TypedCow : private Cow, public _cow_util::Span<T> {
	struct Fake;
	friend class Fake;

	TypedCow() = delete;
	inline TypedCow(TypedCow<T>&& move) : Cow(std::move(move.super)) { /* Moves the shared_ptr. No need to move the elements. */ }
	// protected: copy
	
	inline TypedCow(size_t sz) : Cow(sz * sizeof(T)) { init_copy(T()); }
	inline TypedCow(size_t sz, const T& copy_from) : TypedCow(sz) { init_copy(copy_from); }
	template<typename... Args>
	inline TypedCow(size_t sz, Args&&... args) : TypedCow(sz) { init_copy( T(std::forward<Args>(args)...) ); }

	inline ~TypedCow() { uninit(); }

	inline void* area() { return Cow::area(); }
	inline const void* area() const { return Cow::area(); }
	inline size_t size() const override { return Cow::size() / sizeof(T); }

	inline T& operator[](size_t i) { if (i>=size()) throw "idx"; else return *reinterpret_cast<T*>(this->as_bytes() + (sizeof(T) * i)); }
	inline const T& operator[](size_t i) const { if (i>=size()) throw "idx"; else return *reinterpret_cast<const T*>(this->as_bytes() + (sizeof(T) * i)); }

	protected:
	// Should only be used for creating Fakes. Copies the refcounted pointer.
	inline TypedCow(const TypedCow<T>& copy) : Cow(copy.super) {}

	// UNSAFE: Placement-new's copys of `copy_from` into `0..size()` of this instance.
	inline void init_copy(const T& copy_from) {
		unsigned char* bytes = this->as_bytes();
		for(size_t i=0;i<size();i++)
			new ((void*)(bytes + (sizeof(T)*i))) T(copy_from);

	}
	// UNSAFE: Explicitly calls destructors of each element in this instance.
	inline void uninit() {
		unsigned char* bytes = this->as_bytes();
		for(size_t i=0;i<size();i++)
			reinterpret_cast<T*>(bytes + (sizeof(T)*i))->~T();
	}
};

template<typename T>
struct TypedCow<T>::Fake : private Cow::Fake, public _cow_util::Span<T> {

};
