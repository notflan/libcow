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

#define LIKELY(ex) __builtin_expect(!!(ex), 1)
#define UNLIKELY(ex) __builtin_expect(!!(ex), 0)

#define box(t) aligned_alloc(_Alignof(t), sizeof(t))

#if defined(DEBUG) || defined(COW_TRACE)
#define TRACE(msg, ...) (fprintf(stderr, "<libcow> [TRACE] %s->%s():%d: " msg "\n", __FILE__, __func__, __LINE__, __VA_ARGS__), (void)0)
#else
#define TRACE(msg, ...) ((void)0)
#endif

#if !defined(COW_NO_ASSERT)
#define ASSERT(expr, msg) do { if(!(expr)) die("assertion failed: `" #expr  "`: " msg);  } while(0)
#else
#define ASSERT(op, msg) ((void)0)
#endif

#define LASSERT(expr, msg) ASSERT(LIKELY(expr), "(unexpected) " msg)
#define UASSERT(expr, msg) ASSERT(UNLIKELY(expr), "(expected) " msg)

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
	if(fd<=0) die("cow_create:shm_fd:memfd_create");
	if(ftruncate(fd, size) != 0) die("cow_create:shm_fd:ftruncate");
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
	ret.error = COW_POISON_NONE;
	ret.size = size;
	ret.fd = shm_fd(size);
	ret.origin = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, ret.fd, 0);
	if(ret.origin == MAP_FAILED) die("cow_create:mmap");

	TRACE("mapped new origin cow page of %lu size at %p (memfd %d)", size, ret.origin, ret.fd);
	return ret;
}
cow_t* cow_create(size_t size)
{	
	return box_value(_cow_create_unboxed(size));
}

inline internal void _cow_free_unboxed(const cow_t* cow)
{
	TRACE("unmapping %s cow of %lu size from %p (fd %d, real fd %d)", cow_is_fake(cow) ? "fake" : "and closing fd of origin", cow->size, cow->origin, cow->fd, cow_real_fd(cow));
	munmap(cow->origin, cow->size);
	if(!cow_is_fake(cow))
		close(cow->fd);
}

void cow_free(cow_t* restrict cow)
{
	_cow_free_unboxed(cow);
	free(cow);
}

cow_t* cow_clone(const cow_t* cow)
{
	cow_t clone;

	clone.error = COW_POISON_NONE;
	clone.origin = mmap(cow->origin, cow->size, PROT_READ|PROT_WRITE, MAP_PRIVATE, cow_real_fd(cow), 0);
	if(clone.origin == MAP_FAILED) die("cow_clone:mmap");
	clone.fd = (~INT_MAX) | cow->fd;
	clone.size = cow->size;

	TRACE("mapped cloned cow page of %lu size from %p (%s) at %p (clone fd %d, parent fd %d, real fd %d)", clone.size, cow->origin, cow_is_fake(cow) ? "fake" : "origin", clone.origin, clone.fd, cow->fd, cow_real_fd(&clone));
	return box_value(clone);
}
