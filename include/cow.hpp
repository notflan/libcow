#pragma once

#include "cow.h"

#include <memory>

struct Cow {
	Cow() = delete;

	Cow(size_t size);
	Cow(Cow&& m);
	virtual ~Cow();
	
	static Cow from_raw(cow_t* owned);

	struct Fake;

	private:
	struct _inner;
	Cow(cow_t* raw);
	protected:
	const std::shared_ptr<_inner> super;
	Cow(const Cow& c);
};

struct Cow::Fake : Cow {
	Fake() = delete;

	Fake(const Fake& c);
	Fake(Fake&& m);

	~Fake();
	static Fake from_parent(const Cow& parent);
	private:
	Fake(const Cow& parent);
	cow_t* fake;
};

