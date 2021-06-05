#include <cow.hpp>

#include <cstdio>
#include <cstring>

void print_slice(Slice<unsigned char> memory)
{
	printf("slice: { %p, %lu }\n", memory.area(), memory.size());
}

void write_fake(Cow& clone, const char* string)
{
	strncpy(clone.area_as<char>(), string, clone.size_as<char>()-1);
}

void read_fake(const Cow& clone)
{
	printf("read_fake: %s\n", clone.area_as<char>());
}

int main()
{
	Cow real(4096);

	printf("Created real: ");
	print_slice(real);	

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
	return 0;
}
