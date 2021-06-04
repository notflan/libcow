#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cow.h>

#define SIZE 4096

int main()
{
	cow_t* origin = cow_create(SIZE);
	
	strcpy(cow_ptr(origin), "Hello world");
	cow_t* fake = cow_clone(origin);
	printf("Fake (pre write): %s\n", cow_ptr_of(const char, fake));
	strcpy(cow_ptr(fake), "Hello fake!");

	printf("Real: %s\n", cow_ptr_of(const char, origin));
	printf("Fake: %s\n", cow_ptr_of(const char, fake));
	cow_free(fake);
	cow_free(origin);

	return 0;
}
