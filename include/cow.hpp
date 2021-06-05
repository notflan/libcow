#pragma once

#include "cow.h"

#include <memory>

#include "cow/slice.hpp"

struct Cow : public _cow_util::Span<unsigned char> {
	struct Fake;
	Cow() = delete;

	Cow(size_t size);
	Cow(Cow&& m);
	virtual ~Cow();

	virtual Fake clone() const;

	inline void* area() override { return cow_ptr(get_raw()); }
	inline const void* area() const override { return cow_ptr_of(const void, get_raw()); }

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

	private:
	struct _inner;
	Cow(cow_t* raw);

	protected:
	Cow(const Cow& c);
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

	protected:
	cow_t* get_raw() const override;
	private:

	cow_t* const fake;
};
