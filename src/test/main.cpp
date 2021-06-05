#include <cow.hpp>

#include <cstdio>

int main()
{
	Cow real(4096);

	printf("Fast size: %lu, slow size: %lu\n", real.size_unsafe(), real.size());

	Cow::Fake clone = real;
	{
		Cow::Fake clone2 = clone;
	}
	return 0;
}
