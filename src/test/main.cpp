#include <cow.hpp>

int main()
{
	Cow real(4096);

	Cow::Fake clone = real.clone();
	{
		Cow::Fake clone2 = clone.clone();
	}
	return 0;
}
