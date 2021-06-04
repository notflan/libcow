#pragma once

#include "cow.h"

#include <memory>

struct Cow {
	struct Fake;
	Cow() = delete;

	Cow(size_t size);
	Cow(Cow&& m);
	virtual ~Cow();

	virtual Fake clone() const;

	inline void* area() { return cow_ptr(get_raw()); }
	inline const void* area() const { return cow_ptr_of(const void, get_raw()); }
	inline size_t size() const { return cow_size(get_raw()); }
	
	inline unsigned char* as_bytes() { return (unsigned char*)area(); }
	inline const unsigned char* as_bytes() const { return (const unsigned char*)area(); }

	unsigned char& operator[](size_t index);
	const unsigned char& operator[](size_t index) const;

	static Cow from_raw(cow_t* owned);

	private:
	struct _inner;
	Cow(cow_t* raw);

	protected:
	Cow(const Cow& c);
	const std::shared_ptr<_inner> super;
	virtual cow_t* get_raw() const;

};

struct Cow::Fake : Cow {
	Fake() = delete;
	Fake(const Fake& copy);
	Fake(Fake&& move);
	~Fake();

	Fake clone() const override;

	static Fake from_real(const Cow& real);

	protected:
	cow_t* get_raw() const override;
	private:
	Fake(const Cow& real);

	cow_t* const fake;
};
