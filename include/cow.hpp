#pragma once

#include "cow.h"

#include <memory>
#include <exception>

#include "cow/slice.hpp"

struct CowException : public std::exception
{
	inline CowException(cow_err_kind k) : kind(k){}

	const char* what() const noexcept override;
	inline ~CowException(){}

	const cow_err_kind kind;
};

struct Cow : public _cow_util::Span<unsigned char> {
	struct Fake;
	Cow() = delete;

	explicit Cow(size_t size);
	Cow(Cow&& m);
	virtual ~Cow();

	virtual Fake clone() const;

	protected:
	inline void* area() override { 
		auto raw = get_raw();
		return raw ? cow_ptr(raw) : nullptr;
	}
	inline const void* area() const override { 
		auto raw = get_raw();
		return raw ? cow_ptr_of(const void, raw) : nullptr;
	}
	public:

	/// Get the size of the mapped area.
	///
	/// Note: This calls into `_inner`'s internals. To skip the call on non-LTO builds, use `size_unsafe()`.
	size_t size() const override;
	/// Get the size by assuming the ABI layout of cow_t to be correct. Potentially faster but ABI sensitive.
	/// This shouldn't be a problem if all build static assertions passed.
	///
	/// Note: This behaviour can be diabled by building with `-DCOW_NO_ASSUME_ABI`. In this case, this function calls out to the C API to determine the size.
	/// 	  There is also likely no benefit using this over `size()` in LTO enabled builds.
	///
	/// XXX: Deprecated function for now. It seems `size()` offers better codegen on LTO and non-LTO enabled builds.
	[[deprecated("size() is safer and offers better codegen.")]] inline size_t size_unsafe() const { return _cow_size_unsafe(get_raw()); }
	
	static Cow from_raw(cow_t* owned);

	virtual cow_t* raw() const;

	private:
	struct _inner;
	Cow(cow_t* raw);

	protected:
	Cow(const Cow& c);
	explicit inline Cow(std::shared_ptr<_inner>&& super) : super(std::move(super)){}

	const std::shared_ptr<_inner> super;
	virtual cow_t* get_raw() const;

};

struct Cow::Fake : public Cow {
	Fake() = delete;
	Fake(const Cow& real);
	Fake(const Fake& copy);
	Fake(Fake&& move);
	~Fake();

	Fake clone() const override;

	static Fake from_real(const Cow& real);

	inline cow_t* raw() const override { return fake; }

	protected:
	cow_t* get_raw() const override;
	private:

	cow_t* const fake;
};

