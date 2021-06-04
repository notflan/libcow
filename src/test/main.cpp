#include <cow.hpp>

int main()
{
	Cow real(4096);

	Cow::Fake clone = real;
	{
		Cow::Fake clone2 = clone;
	}
	return 0;
}
