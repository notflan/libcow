#pragma once

#include <cow.hpp>
#include <utility>

template<typename T>
struct TypedCow : public _cow_util::Span<T> {
	struct Fake;

	template<typename... Args>
	inline TypedCow(size_t sz, Args&&... args) : real(Cow(sz * sizeof(T))) { init_copy( T(std::forward<Args>(args)...) ); }
	inline TypedCow(size_t sz, const T& copy) : real(Cow(sz * sizeof(T))) { init_copy(copy); }
	inline TypedCow(size_t sz) : TypedCow(sz, T()){}

	inline virtual ~TypedCow() { uninit(); }

	inline virtual Fake clone() const { return Fake(*this); }

	inline virtual cow_t* raw() const { return real.raw(); }

	inline size_t size() const { return real.size() / sizeof(T); }
	protected:
	//inline explicit TypedCow(const TypedCow<T>& unsafeCopy) : real(Cow(unsafeCopy.real)){}

	inline virtual void* area() override { return reinterpret_cast<void*>( real.ptr() ); }
	inline virtual const void* area() const override { return reinterpret_cast<const void*>( real.ptr() ); }

	inline void init_copy(const T& copy_from) {
		T* ptr = this->ptr();
		for(size_t i=0;i<size();i++)
			new ((void*)(ptr+i)) T(copy_from);

	}
	// UNSAFE: Explicitly calls destructors of each element in this instance.
	inline void uninit() {
		T* ptr = this->ptr();
		if(!ptr) return;

		for(size_t i=0;i<size();i++)
			(ptr+i)->~T();
	}
	private:
	Cow real;
};
template<typename T>
struct TypedCow<T>::Fake : public TypedCow<T>  {
//XXX: How THE FUCK do we initialise base's `real` here?????
	Fake() = delete;

	inline Fake(const Fake& copy) : fake(cow_clone(copy.fake)){}
	inline Fake(Fake&& move) : fake(move.fake) { *const_cast<cow_t**>(&move.fake) = nullptr; }
	inline Fake(const TypedCow<T>& clone) : fake(cow_clone(clone.raw())){}

	inline cow_t* raw() const override { return fake; }
	inline Fake clone() const override { return Fake(*this); }

	inline ~Fake(){}

	inline size_t size() const override { return fake ? cow_size(fake) : 0; }
	protected:
	inline void* area() override { return fake ? cow_ptr(fake) : nullptr; }
	inline const void* area() const override { return fake ? cow_ptr_of(const void, fake) : nullptr; }
	
	private:
	cow_t* const fake;
};
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
