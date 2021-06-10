#pragma once

#include <cow.hpp>
#include <utility>

template<typename T>
struct TypedCow : private Cow, public _cow_util::Span<T> {
	struct Fake;

	inline TypedCow(size_t sz) : TypedCow(sz, T()) {}
	template<typename... Args>
	inline TypedCow(size_t sz, Args&&... args) : TypedCow(sz, T(std::forward<Args>(args)...)) {}

	inline TypedCow(TypedCow<T>&& move) : Cow(std::move(move.super)) {
		unsigned char* bytes = this->as_bytes();
		for(size_t i=0;i<size();i++)
			new ((void*)(bytes + (sizeof(T)*i))) T(std::move(move[i]));
	}
	inline TypedCow(const TypedCow<T>& copy) : Cow(copy.super) {
		unsigned char* bytes = this->as_bytes();
		for(size_t i=0;i<size();i++)
			new ((void*)(bytes + (sizeof(T)*i))) T(copy[i]);
	}
	inline ~TypedCow() {
		unsigned char* bytes = this->as_bytes();
		for(size_t i=0;i<size();i++)
			reinterpret_cast<T*>(bytes + (sizeof(T)*i))->~T();
	}

	void* area() { return (*this).Cow::area(); }
	const void* area() const { return (*this).Cow::area(); }
	inline size_t size() const override { return (*this).Cow::size() / sizeof(T); }

	inline T& operator[](size_t i) { if (i>=size()) throw "idx"; else return *reinterpret_cast<T*>(this->as_bytes() + (sizeof(T) * i)); }
	inline const T& operator[](size_t i) const { if (i>=size()) throw "idx"; else return *reinterpret_cast<const T*>(this->as_bytes() + (sizeof(T) * i)); }

	private:
	inline explicit TypedCow(size_t sz, const T& copy_from) : Cow(sz * sizeof(T)) {
		unsigned char* bytes = this->as_bytes();
		for(size_t i=0;i<sz;i++)
			new ((void*)(bytes + (sizeof(T)*i))) T(copy_from);
	}
};

template<typename T>
struct TypedCow<T>::Fake : private Cow::Fake, public _cow_util::Span<T> {

};
