#include <cow.hpp>

#include <utility>

#include "cow_t.h"

struct Cow::_inner {
	cow_t cow;

	inline const cow_t* ptr() const { return &cow; }
	inline cow_t* ptr() { return &cow; }

	~_inner();
	_inner(size_t sz);
	_inner(cow_t* ptr);

	_inner(const _inner& copy) = delete;
	_inner(_inner&& move) = delete;
	_inner() = delete;
};
Cow::_inner::~_inner() {
	_cow_free_unboxed(ptr());
}
Cow::_inner::_inner(size_t sz) : cow(_cow_create_unboxed(sz)){}
Cow::_inner::_inner(cow_t* ptr) : cow(*ptr)
{
	free(ptr);
}

Cow::Cow(size_t size) : super(std::make_shared<_inner>(size)){}
Cow::Cow(cow_t* raw) : super(std::make_shared<_inner>(raw)){}

Cow::Cow(Cow&& m) : super(std::move(*const_cast<std::shared_ptr<_inner>*>(&m.super))){}
Cow::Cow(const Cow& c) : super(c.super){}
Cow::~Cow(){}

Cow Cow::from_raw(cow_t* owned) { if(cow_is_fake(owned)) throw "Trying to create real from fake raw"; else return Cow(owned); }

Cow::Fake Cow::clone() const { return Fake::from_real(*this); }
cow_t* Cow::get_raw() const { return super->ptr(); }

size_t Cow::size() const { return super->cow.size; }

Cow::Fake::Fake(const Cow& copy) : Cow(copy), fake(cow_clone(copy.super->ptr())){}
Cow::Fake::Fake(const Fake& copy) : Cow(copy), fake(cow_clone(copy.fake)){}//Fake(*static_cast<const Cow*>(&copy)){}
Cow::Fake::Fake(Fake&& move) : Cow(std::move(move)), fake(move.fake)
{
	*const_cast<cow_t**>(&move.fake) = nullptr;
}
Cow::Fake::~Fake() { if(fake) cow_free(fake); }

Cow::Fake Cow::Fake::Fake::from_real(const Cow& real) { return Fake(real); }

Cow::Fake Cow::Fake::clone() const { return Fake(*static_cast<const Fake*>(this)); }
cow_t* Cow::Fake::get_raw() const { return fake; }


