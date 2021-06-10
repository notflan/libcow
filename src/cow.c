#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <limits.h>
#include <stddef.h>
#include <errno.h>

#include <cow.h>

#include "macros.h"

// struct cow { ... }
#include "cow_t.h"
#include "error.h"

static __attribute__((noreturn)) __attribute__((noinline)) __attribute__((cold)) void die(const char* error)
{
	fprintf(stderr, "<libcow> [FATAL]: ");
	if(!errno)
		fprintf(stderr, "%s\n (no matching errno, try compiling with `-DCOW_TRACE` and/or `-DDEBUG`)\n", error);
	else
		perror(error);

	abort();
}

static inline cow_t* box_value(cow_t v)
{
	if(UNLIKELY(v.poisoned)) {
		TRACE("WARNING: v is poisoned! not boxing { origin = %p, fd = 0x%x, size = %lu } -> 0 (%lu bytes)", v.origin, v.fd, v.size, sizeof(cow_t));
		return NULL;
	}
	cow_t* boxed = box(cow_t);
	LASSERT(boxed != NULL, "aligned_alloc() returned `NULL` for `cow_t`");

	TRACE("boxing cow_t { origin = %p, fd = 0x%x, size = %lu } -> %p (%lu bytes)", v.origin, v.fd, v.size, (const void*)boxed, sizeof(cow_t));
	*boxed = v;

	LASSERT(cow_ptr(boxed) == boxed->origin, "ptr extraction mismatch. this should never happen (check origin field offset)");
	return boxed;
}

static inline int shm_fd(size_t size)
{
#ifdef COW_NAME_SHM
	_Thread_local static char buffer[12] = {0};
	snprintf(buffer, 11, "0x%08lx", size);
	//fprintf(stderr, "shm_fd_name: '%s'\n", buffer);
	// XXX: Not entirely sure how ownership works for the string passed here... Let's not.
	int fd = memfd_create(buffer, 0);
#else
	int fd = memfd_create("cow_create:shm_fd", 0);
#endif
	SOFT_ASSERT(fd>0, COW_ERR_FDCREATE, -1);
	SOFT_ASSERT(ftruncate(fd, size) == 0, COW_ERR_SIZE, (close(fd), -1));

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

inline internal cow_t _cow_create_unboxed(size_t size)
{
	cow_t ret;

	ret.size = size;
	if( (ret.poisoned = 
		((ret.fd = shm_fd(size)) == -1))
	) {
		ret.origin = NULL;
		return ret;
	}
	ret.origin = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, ret.fd, 0);

	SOFT_ASSERT(ret.origin != MAP_FAILED, COW_ERR_MAP, (ret.poisoned = true, close(ret.fd), ret));

	TRACE("mapped new origin cow page of %lu size at %p (memfd %d)", size, ret.origin, ret.fd);
	return ret;
}
cow_t* cow_create(size_t size)
{	
	return box_value(_cow_create_unboxed(size));
}

inline internal void _cow_free_unboxed(const cow_t* cow)
{
	if(UNLIKELY(cow->poisoned)) {
		TRACE("WARNING: attempted to free poisoned object at %p", (const void*)cow);
		return;
	}
	TRACE("unmapping %s cow of %lu size from %p (fd %d, real fd %d)", cow_is_fake(cow) ? "fake" : "and closing fd of origin", cow->size, cow->origin, cow->fd, cow_real_fd(cow));
	munmap(cow->origin, cow->size);
	if(!cow_is_fake(cow))
		close(cow->fd);
}

void cow_free(cow_t* restrict cow)
{
	if(UNLIKELY(!cow)) return;

	_cow_free_unboxed(cow);
	free(cow);
}

cow_t* cow_clone(const cow_t* cow)
{
	if(UNLIKELY(cow->poisoned)) {
		_cow_set_err(COW_ERR_POISONED);
		TRACE("WARNING: attempted to clone poisoned object at %p", (const void*)cow);
	
		return NULL;
	}
	cow_t clone;

	//clone.error = COW_POISON_NONE;
	clone.poisoned=false;
	clone.origin = mmap(cow->origin, cow->size, PROT_READ|PROT_WRITE, MAP_PRIVATE, cow_real_fd(cow), 0);
	SOFT_ASSERT(clone.origin != MAP_FAILED, COW_ERR_MAP, NULL);
	
	clone.fd = (~INT_MAX) | cow->fd;
	clone.size = cow->size;

	TRACE("mapped cloned cow page of %lu size from %p (%s) at %p (clone fd %d, parent fd %d, real fd %d)", clone.size, cow->origin, cow_is_fake(cow) ? "fake" : "origin", clone.origin, clone.fd, cow->fd, cow_real_fd(&clone));
	return box_value(clone);
}
