#include <cow.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace _cow_util;

template<typename T = unsigned char>
void print_slice(Slice<T> memory)
{
	printf("slice: { %p, %lu (%lu bytes) }\n", memory.area(), memory.size(), memory.size_bytes());

}

void write_fake(Cow& clone, const char* string)
{
	strncpy(clone.area_as<char>(), string, clone.size_as<char>()-1);
}

void read_fake(const Cow& clone)
{
	printf("read_fake: %s\n", clone.area_as<char>());
}

void moving_cow(Cow moved)
{
	auto moved_clone = moved.reinterpret<char>();
	strncpy(&moved_clone, "Ummmm....", moved_clone.size());

	read_fake(moved);
}

int main()
{
	Cow real(4096);
	memset(real, 0, real.size_bytes());

	printf("Created real: ");
	print_slice(real);
	print_slice(real.slice_wrap(-20, -10));

	write_fake(real, "Hello world");
	read_fake(real);
	
	Cow::Fake clone = real;
	printf("Fake size: %lu\n", clone.size());
	printf("Fake ptr: %p\n", clone.area());

	read_fake(clone);
	write_fake(clone, "hello fake!");
	read_fake(clone);
	read_fake(real);

	printf("First byte of: real = %x, fake = %x\n", real[0], clone[0]);
	moving_cow(std::move(real)); //moving real is fine
	// <-- real is now dropped. But `clone` still holds Rc to _inner (cow_t).
	printf("First byte of: fake = %x\n", clone[0]);
	read_fake(clone); //clone still functions because of refcount on origin.

	return 0;
}
