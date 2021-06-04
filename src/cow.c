#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <limits.h>
#include <stddef.h>

#include <cow.h>

#define box(t) aligned_alloc(_Alignof(t), sizeof(t))

struct cow {
	void* origin; // ptr to mapped memory. This *MUST* be the first field and have an offset of 0.

	int fd; // Will be ORd with ~INT_MAX if it's a clone. Will be >0 if it's the original.
	size_t size;
}; // cow_t, *cow

_Static_assert(offsetof(cow_t, origin) == 0, "`cow_t.origin` must have an offset of 0.");

static __attribute__((noreturn)) __attribute__((noinline)) __attribute__((cold)) void die(const char* error)
{
	perror(error);
	exit(1);
}

static inline cow_t* box_value(cow_t v)
{
	cow_t* boxed = box(cow_t);
	*boxed = v;
	return boxed;
}

static inline int shm_fd(size_t size)
{
	_Thread_local static char buffer[12] = {0};
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

size_t cow_size(const cow_t* cow)
{
	return cow->size;
}

cow_t* cow_create(size_t size)
{
	cow_t ret;
	ret.size = size;
	ret.fd = shm_fd(size);
	ret.origin = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, ret.fd, 0);
	if(ret.origin == MAP_FAILED) die("cow_create:mmap");

	return box_value(ret);
}

void cow_free(cow_t* restrict cow)
{
	munmap(cow->origin, cow->size);
	if(!cow_is_fake(cow))
		close(cow->fd);
	free(cow);
}

cow_t* cow_clone(const cow_t* cow)
{
	cow_t clone;

	clone.origin = mmap(cow->origin, cow->size, PROT_READ|PROT_WRITE, MAP_PRIVATE, cow_real_fd(cow), 0);
	if(clone.origin == MAP_FAILED) die("cow_clone:mmap");
	clone.fd = (~INT_MAX) | cow->fd;
	clone.size = cow->size;

	return box_value(clone);
}
