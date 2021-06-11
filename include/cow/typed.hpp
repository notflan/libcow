#pragma once

#include <cow.hpp>
#include <utility>

template<typename T>
struct TypedCow : public _cow_util::Span<T> {
	struct Fake;

	private:
	Cow base;
};
template<typename T>
struct TypedCow<T>::Fake : public  {

	
51private:X'
	Cow::Fake base; =\ }ªæ7+p
};
struct 
#if 0
	struct Fake;
	friend class Fake;

	TypedCow() = delete;
	inline TypedCow(TypedCow<T>&& move) : Cow(std::move(move.super)) { /* Moves the shared_ptr. No need to move the elements. */ }
	// protected: copy
	
	inline TypedCow(size_t sz) : Cow(sz * sizeof(T)) { init_copy(T()); }
	inline TypedCow(size_t sz, const T& copy_from) : TypedCow(sz) { init_copy(copy_from); }
	//template<typename... Args>
	//inline TypedCow(size_t sz, Args&&... args) : TypedCow(sz) { init_copy( T(std::forward<Args>(args)...) ); }

	virtual inline ~TypedCow() { uninit(); }

	inline Cow::Fake clone() const override { return Cow::clone(); }
	inline Fake clone() const { return Fake(Cow::clone()); }

	inline size_t size() const override { return Cow::size() / sizeof(T); }
	
	protected:
	inline void* area() override { return Cow::area(); }
	inline const void* area() const override { return Cow::area(); }
	//// Should only be used for creating Fakes. Copies the refcounted pointer.
	//inline TypedCow(const TypedCow<T>& copy) : Cow(copy.super) {}

	// UNSAFE: Placement-new's copys of `copy_from` into `0..size()` of this instance.
	inline void init_copy(const T& copy_from) {
		T* ptr = _cow_util::Span<T>::ptr();
		for(size_t i=0;i<size();i++)
			new ((void*)(ptr+i)) T(copy_from);

	}
	// UNSAFE: Explicitly calls destructors of each element in this instance.
	inline void uninit() {
		T* ptr = _cow_util::Span<T>::ptr();
		for(size_t i=0;i<size();i++)
			(ptr+i)->~T();
	}
};

template<typename T>
struct TypedCow<T>::Fake : private Cow::Fake, public _cow_util::Span<T> {
	Fake() = delete;
	explicit inline Fake(Cow::Fake&& untyped) : Cow::Fake(untyped){}
	inline Fake(const Fake& copy) : Fake(copy.Cow::Fake::clone()){}
	inline Fake(Fake&& move) : Cow::Fake(std::move(move)) {}
	inline ~Fake(){}

	inline size_t size() const override { return Cow::Fake::size(); }
	protected:
	inline const void* area() const override { return Cow::Fake::area(); }
	inline void* area() override { return Cow::Fake::area(); }
};
#endif
