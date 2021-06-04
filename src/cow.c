#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <limits.h>

#define SIZE 4096

// Copy-on-write mapped memory.
typedef struct cow {
	void* origin;

	int fd; // Will be ORd with ~INT_MAX if it's a clone. Will be >0 if it's the original.
	size_t size;
} cow_t;

static __attribute__((noreturn)) __attribute__((noinline)) __attribute__((cold)) void die(const char* error)
{
	perror(error);
	exit(1);
}

static inline int shm_fd(size_t size)
{
	_Thread_local static char buffer[12] = {};
	snprintf(buffer, 11, "0x%08lx", size);
	//fprintf(stderr, "shm_fd_name: '%s'\n", buffer);
	int fd = memfd_create(buffer, 0);
	if(fd<=0) die("cow_create:shm_fd:memfd_create");
	ftruncate(fd, size);
	return fd;
}

static inline int cow_real_fd(const cow_t* cow)
{
	return cow->fd & INT_MAX;
}

int cow_is_fake(const cow_t* cow)
{
	return cow->fd<0;
}

cow_t cow_create(size_t size)
{
	cow_t ret;
	ret.size = size;
	ret.fd = shm_fd(size);
	ret.origin = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, ret.fd, 0);
	if(ret.origin == MAP_FAILED) die("cow_create:mmap");

	return ret;
}

void cow_free(cow_t cow)
{
	munmap(cow.origin, cow.size);
	if(!cow_is_fake(&cow))
		close(cow.fd);
}

cow_t cow_clone(cow_t cow)
{
	cow_t clone;

	clone.origin = mmap(cow.origin, cow.size, PROT_READ|PROT_WRITE, MAP_PRIVATE, cow_real_fd(&cow), 0);
	if(clone.origin == MAP_FAILED) die("cow_clone:mmap");
	clone.fd = (~INT_MAX) | cow.fd;
	clone.size = cow.size;

	return clone;
}

void* cow_ptr(cow_t* restrict cow)
{
	return cow->origin;
}

const void* cow_ptr_const(const cow_t* cow)
{
	return cow->origin;
}

int main()
{
	cow_t origin = cow_create(SIZE);
	
	strcpy(cow_ptr(&origin), "Hello world");
	cow_t fake = cow_clone(origin);
	printf("Fake (pre write): %s\n", (const char*)cow_ptr(&fake));
	strcpy(cow_ptr(&fake), "Hello fake!");

	printf("Real: %s\n", (const char*)cow_ptr(&origin));
	printf("Fake: %s\n", (const char*)cow_ptr(&fake));
	cow_free(fake);
	cow_free(origin);
	return 0;
}

#ifdef DEMO // This code works
void alter(void* map_ptr)
{
	strcpy(map_ptr, "Hello two");
}

int main()
{
	// This works!
	int graph = shm_fd();
	void* map_ptr = mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, graph, 0);
	if(map_ptr == MAP_FAILED) die("map_ptr");
	strcpy(map_ptr, "Hello world");
	printf("Ptr begins: %s\n", (const char*)map_ptr);
	{
		void* map_alter = mmap(map_ptr, SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE, graph, 0);
		if(map_alter == MAP_FAILED) die("map_alter");
		printf("Alter begins: %s\n", (const char*)map_alter);
		alter(map_alter);
		printf("Alter ends: %s\n", (const char*)map_alter);
		munmap(map_alter, SIZE);
	}
	printf("Ptr ends: %s\n", (const char*) map_ptr);
	munmap(map_ptr, SIZE);
	close(graph);
	return 0;
}
#endif
// Doesn't work...
#if 0
int main()
{
	void* map_ptr = mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_ANON | MAP_SHARED, 0, 0);
	if(map_ptr == MAP_FAILED) die("map_ptr");
	printf("First ptr is: %p\n  - '", map_ptr);
	strcpy(map_ptr, "Hello world");
	printf("%s'\n", (const char*)map_ptr);

#ifdef USE_FORK
	if(fork()>0) { // Eh... This is ugly.
		printf("Forked...\n");
		void* map_alter = mmap(map_ptr, SIZE, PROT_READ|PROT_WRITE, MAP_ANON | MAP_FIXED |MAP_PRIVATE, 0, 0);
#else
		//doesnt' work....
		void* map_alter = mmap(map_ptr, SIZE, PROT_READ|PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0);
#endif
		if(map_alter == MAP_FAILED) die("map_alter");
		printf("Second ptr is: %p\n  - '", map_alter);
		printf("%s' - '", (const char*)map_alter);
		strcpy(map_alter, "Hello two");
		printf("%s'\n", (const char*)map_alter);
#ifdef USE_FORK
	} else 
#endif
		printf("First is still? '%s'\n", (volatile const char*)map_ptr);
	
	return 0;
}
#endif
