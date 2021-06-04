#include <cow.hpp>

#include <utility>

struct Cow::_inner {
	cow_t* ptr;

	~_inner();
	_inner(size_t sz);
	_inner(cow_t* ptr);

	_inner(const _inner& copy) = delete;
	_inner(_inner&& move) = delete;
	_inner() = delete;
};
Cow::_inner::~_inner() {
	if(ptr) {
		cow_free(ptr);
		ptr = nullptr;
	}
}
Cow::_inner::_inner(size_t sz) : ptr(cow_create(sz)){}
Cow::_inner::_inner(cow_t* ptr) : ptr(ptr){}

Cow::Cow(size_t size) : super(std::make_shared<_inner>(size)){}
Cow::Cow(cow_t* raw) : super(std::make_shared<_inner>(raw)){}
Cow::~Cow(){}
Cow::Cow(Cow&& m) : super(std::move(*const_cast<std::shared_ptr<_inner>*>(&super))){}

Cow Cow::from_raw(cow_t* owned) { if(cow_is_fake(owned)) throw "Trying to create real from fake raw"; else return Cow(owned); }

Cow::Fake::Fake(const Cow& parent) : Cow(parent), fake(cow_clone(parent.super->ptr)){} 
Cow::Fake::~Fake() { if(fake) { cow_free(fake); *const_cast<cow_t**>(&fake) = nullptr; } }
Cow::Fake Cow::Fake::from_parent(const Cow& parent) { return Fake(parent); }

Cow::Fake::Fake(Fake&& move) : Cow(std::move(move)), fake(move.fake)
{
	*const_cast<cow_t**>(&fake) = nullptr;
}
